#include "object.hpp"
#include "logging.hpp"

#include <yogi_core.h>

#include <cassert>
#include <sstream>


namespace yogi {
namespace internal {

void throw_on_object_ctor_failure(int res, const Object* obj)
{
    if (res != YOGI_OK) {
        YOGI_LOG_ERROR(Logger::yogi_logger(), "Could not create " << obj->class_name() << ": " << YOGI_GetErrorString(res));
        throw Failure(res);
    }
}

} // namespace internal

std::atomic<int> Object::ms_instances(0);

void Object::_initialise_library()
{
    if (++ms_instances == 1) {
        YOGI_Initialise();
    }
}

Object::Object(void* handle)
: m_handle(handle)
{
    _initialise_library();
}

Object::Object(std::nullptr_t)
: m_handle(nullptr)
{
    _initialise_library();
}

void Object::_destroy()
{
    int res = YOGI_Destroy(m_handle);
    if (res != YOGI_OK) {
        YOGI_LOG_ERROR(Logger::yogi_logger(), "Could not destroy " << class_name() << ": " << YOGI_GetErrorString(res));
    }

    m_handle = nullptr;
}

Object::~Object()
{
    assert (m_handle == nullptr);

    if (--ms_instances == 0) {
        YOGI_Shutdown();
    }
}

const std::string& Object::class_name() const
{
    static std::string s = "Object";
    return s;
}

std::string Object::to_string() const
{
    std::stringstream ss;
    ss << class_name() << " [0x" << std::hex << handle() << "]";
    return ss.str();
}

} //namespace yogi

std::ostream& operator<< (std::ostream& os, const yogi::Object& obj)
{
    return os << obj.to_string();
}
