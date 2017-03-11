#ifndef YOGI_CORE_PUBLISH_SUBSCRIBE_TERMINAL_HPP
#define YOGI_CORE_PUBLISH_SUBSCRIBE_TERMINAL_HPP

#include "../../config.h"
#include "../../base/Buffer.hpp"
#include "../../base/AsyncOperation.hpp"
#include "../../api/ExceptionT.hpp"
#include "../common/PublishSubscribeTerminalBaseT.hpp"
#include "logic_types.hpp"

#include <boost/asio/buffer.hpp>

#include <atomic>


namespace yogi {
namespace core {

class Leaf;

namespace publish_subscribe {

/***************************************************************************//**
 * Implements a publish-subscribe terminal
 *
 * Publish-subscribe terminals implement the publish-subscribe pattern, i.e.
 * terminals can subscribe to other terminals (via bindings) in order to receive
 * data message published by those terminals.
 ******************************************************************************/
template <typename TTypes=logic_types<>>
class Terminal : public common::PublishSubscribeTerminalBaseT<TTypes, false>
{
    typedef common::PublishSubscribeTerminalBaseT<TTypes, false> super;

public:
    Terminal(Leaf& leaf, base::Identifier identifier)
        : super(leaf, identifier)
    {
    }
};

} // namespace publish_subscribe
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_PUBLISH_SUBSCRIBE_TERMINAL_HPP
