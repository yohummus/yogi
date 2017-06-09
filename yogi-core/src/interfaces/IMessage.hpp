#ifndef YOGI_INTERFACES_IMESSAGE_HPP
#define YOGI_INTERFACES_IMESSAGE_HPP

#include "../config.h"
#include "../base/Id.hpp"

#include <memory>
#include <string>
#include <ostream>
#include <vector>


namespace yogi {
namespace interfaces {

struct IMessage;
typedef std::unique_ptr<IMessage> message_ptr;

/***************************************************************************//**
 * Interface for messages
 *
 * Messages are send and received by nodes and leafs in order to communicate
 * with one another.
 ******************************************************************************/
struct IMessage
{
    typedef std::vector<char> buffer_type;

    typedef base::Id id_type;

    virtual ~IMessage() =default;

    virtual id_type type_id() const =0;
    virtual const char* name() const =0;
    virtual std::string to_string() const =0;
    virtual message_ptr clone() const =0;
    virtual void serialize(buffer_type& buffer) const =0;
    virtual void deserialize(const buffer_type& buffer,
        buffer_type::const_iterator start) =0;
};

} // namespace interfaces
} // namespace yogi

inline std::ostream& operator<< (std::ostream& os,
    const yogi::interfaces::IMessage& msg)
{
    os << msg.to_string();
    return os;
}

#endif // YOGI_INTERFACES_IMESSAGE_HPP
