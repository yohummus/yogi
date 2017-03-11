#ifndef YOGI_INTERFACES_IPUBLICOBJECT_HPP
#define YOGI_INTERFACES_IPUBLICOBJECT_HPP

#include "../config.h"

#include <memory>


namespace yogi {
namespace interfaces {

/***************************************************************************//**
 * Interface class for objects that are created, used and destroyed through the
 * shared library interface and referred to via handles
 ******************************************************************************/
class IPublicObject : public std::enable_shared_from_this<IPublicObject>
{
    // noncopyable
    IPublicObject(const IPublicObject&) = delete;
    void operator= (const IPublicObject&) = delete;

public:
    IPublicObject() = default;
    virtual ~IPublicObject() = default;

    template <typename T>
    std::shared_ptr<T> make_ptr()
    {
        return std::dynamic_pointer_cast<T>(shared_from_this());
    }
};

typedef std::shared_ptr<IPublicObject> public_object_ptr;

} // namespace interfaces
} // namespace yogi

#endif // YOGI_INTERFACES_IPUBLICOBJECT_HPP
