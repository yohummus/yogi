#ifndef YOGI_CORE_MAPPEDOBJECTFSM_HPP
#define YOGI_CORE_MAPPEDOBJECTFSM_HPP

#include "../../config.h"
#include "../../base/Id.hpp"


namespace yogi {
namespace core {
namespace common {

/***************************************************************************//**
 * State machine that ensures a consistent state of objects (e.g. endpoints and
 * bindings) between leafs and nodes
 ******************************************************************************/
class MappedObjectFsm
{
    typedef base::Id id_type;

public:
    enum state_t {
        STATE_UNUSED_UNCONFIRMED,
        STATE_UNUSED_CONFIRMED,
        STATE_MAPPED_UNCONFIRMED,
        STATE_MAPPED_CONFIRMED,
        STATE_AWAIT_ACK,
        STATE_AWAIT_CONFIRMATION,
        STATE_AWAIT_REMOVALS
    };

private:
    state_t m_state           = STATE_UNUSED_CONFIRMED;
    int     m_skipRemovedAcks = 0;
    id_type m_mappedId;

public:
    state_t state() const
    {
        return m_state;
    }

    bool is_mapped() const
    {
        switch (m_state) {
        case STATE_MAPPED_UNCONFIRMED:
        case STATE_MAPPED_CONFIRMED:
            return true;

        default:
            return false;
        }
    }

    bool destroyed() const
    {
        switch (m_state) {
        case STATE_UNUSED_UNCONFIRMED:
        case STATE_UNUSED_CONFIRMED:
        case STATE_MAPPED_UNCONFIRMED:
        case STATE_MAPPED_CONFIRMED:
            return false;

        case STATE_AWAIT_ACK:
        case STATE_AWAIT_CONFIRMATION:
        case STATE_AWAIT_REMOVALS:
            return true;
        }

        YOGI_NEVER_REACHED;
        return false;
    }

    id_type mapped_id() const
    {
        return m_mappedId;
    }

    template <typename TSendDescFn>
    void reset(bool hidden, TSendDescFn sendDescFn)
    {
        // reset the state
        if (hidden) {
            m_state = STATE_UNUSED_CONFIRMED;
        }
        else {
            sendDescFn();
            m_state = STATE_UNUSED_UNCONFIRMED;
        }

        m_mappedId        = id_type{};
        m_skipRemovedAcks = 0;
    }

    template <typename TSendRemFn>
    bool process_object_destroyed(TSendRemFn sendRemFn)
    {
        switch (m_state) {
        case STATE_MAPPED_CONFIRMED:
            sendRemFn(m_mappedId);
            m_state = STATE_AWAIT_ACK;
            break;

        case STATE_UNUSED_CONFIRMED:
            if (m_skipRemovedAcks > 0)
                m_state = STATE_AWAIT_REMOVALS;
            else
                return false;
            break;

        case STATE_MAPPED_UNCONFIRMED:
        case STATE_UNUSED_UNCONFIRMED:
            m_state = STATE_AWAIT_CONFIRMATION;
            break;

        default:
            YOGI_NEVER_REACHED;
        }

        return true;
    }

    template <typename TSendMappFn, typename TSendRemAckFn,
        typename TMappStateChangedFn>
    void process_rcvd_description(id_type remoteId, TSendMappFn sendMappFn,
        TSendRemAckFn sendRemAckFn, TMappStateChangedFn mappStChFn)
    {
        switch (m_state) {
        case STATE_UNUSED_UNCONFIRMED:
            sendMappFn();
            m_mappedId = remoteId;
            m_state    = STATE_MAPPED_UNCONFIRMED;
            mappStChFn(true);
            break;

        case STATE_UNUSED_CONFIRMED:
            sendMappFn();
            m_mappedId = remoteId;
            m_state    = STATE_MAPPED_CONFIRMED;
            mappStChFn(true);
            break;

        case STATE_MAPPED_UNCONFIRMED:
        case STATE_MAPPED_CONFIRMED:
            sendRemAckFn(m_mappedId);
            sendMappFn();
            m_mappedId = remoteId;
            m_skipRemovedAcks++;
            mappStChFn(false);
            mappStChFn(true);
            break;

        default:
            YOGI_NEVER_REACHED;
        }
    }

    template <typename TSendRemFn, typename TSendRemAckFn,
        typename TMappStateChangedFn>
    void process_rcvd_mapping(id_type remoteId, TSendRemFn sendRemFn,
        TSendRemAckFn sendRemAckFn, TMappStateChangedFn mappStChFn)
    {
        switch (m_state) {
        case STATE_UNUSED_UNCONFIRMED:
            m_mappedId = remoteId;
            m_state    = STATE_MAPPED_CONFIRMED;
            mappStChFn(true);
            break;

        case STATE_MAPPED_UNCONFIRMED:
            YOGI_ASSERT(remoteId == m_mappedId);
            m_state = STATE_MAPPED_CONFIRMED;
            break;

        case STATE_AWAIT_CONFIRMATION:
            if (!m_mappedId || m_mappedId == remoteId) {
                sendRemFn();
                m_mappedId = remoteId;
                m_state    = STATE_AWAIT_ACK;
            }
            else {
                sendRemAckFn(m_mappedId);
                sendRemFn();
                m_mappedId = remoteId;
                m_state    = STATE_AWAIT_ACK;
                m_skipRemovedAcks++;
            }
            break;

        default:
            YOGI_NEVER_REACHED;
        }
    }

    template <typename TSendRemAckFn, typename TMappStateChangedFn>
    bool process_rcvd_noticed(TSendRemAckFn sendRemAckFn,
        TMappStateChangedFn mappStChFn)
    {
        switch (m_state) {
        case STATE_UNUSED_UNCONFIRMED:
            m_state = STATE_UNUSED_CONFIRMED;
            break;

        case STATE_MAPPED_UNCONFIRMED:
            sendRemAckFn(m_mappedId);
            m_mappedId = id_type{};
            m_state    = STATE_UNUSED_CONFIRMED;
            m_skipRemovedAcks++;
            mappStChFn(false);
            break;

        case STATE_AWAIT_CONFIRMATION:
            if (m_mappedId) {
                sendRemAckFn(m_mappedId);
                m_mappedId = id_type{};
                m_state    = STATE_AWAIT_REMOVALS;
                m_skipRemovedAcks++;
            }
            else {
                return false;
            }
            break;

        default:
            YOGI_NEVER_REACHED;
        }

        return true;
    }

    template <typename TSendRemAckFn, typename TMappStateChangedFn>
    bool process_rcvd_removed(TSendRemAckFn sendRemAckFn,
        TMappStateChangedFn mappStChFn)
    {
        switch (m_state) {
        case STATE_UNUSED_CONFIRMED:
            YOGI_ASSERT(m_skipRemovedAcks > 0);
            m_skipRemovedAcks--;
            break;

        case STATE_MAPPED_CONFIRMED:
            if (m_skipRemovedAcks > 0) {
                m_skipRemovedAcks--;
            }
            else {
                sendRemAckFn(m_mappedId);
                m_mappedId = id_type{};
                m_state    = STATE_UNUSED_CONFIRMED;
                mappStChFn(false);
            }
            break;

        case STATE_AWAIT_ACK:
            if (m_skipRemovedAcks > 0) {
                YOGI_ASSERT(m_mappedId);
                m_skipRemovedAcks--;
            }
            else {
                YOGI_ASSERT(m_mappedId);
                sendRemAckFn(m_mappedId);
                m_mappedId = id_type{};
            }
            break;

        case STATE_AWAIT_REMOVALS:
            if (m_skipRemovedAcks > 1) {
                m_skipRemovedAcks--;
            }
            else {
                YOGI_ASSERT(m_skipRemovedAcks == 1);
                return false;
            }
            break;

        default:
            YOGI_NEVER_REACHED;
        }

        return true;
    }

    bool process_rcvd_removed_ack()
    {
        switch (m_state) {
        case STATE_AWAIT_ACK:
            YOGI_ASSERT(m_skipRemovedAcks == 0);
            return false;

        default:
            YOGI_NEVER_REACHED;
        }

        return true;
    }
};

} // namespace common
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_MAPPEDOBJECTFSM_HPP
