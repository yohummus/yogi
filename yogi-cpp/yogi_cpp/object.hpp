#ifndef YOGICPP_OBJECT_HPP
#define YOGICPP_OBJECT_HPP

#include "result.hpp"
#include "internal/utility.hpp"

#include <string>
#include <ostream>
#include <atomic>


namespace yogi {

class Object;

namespace internal {

void throw_on_object_ctor_failure(int res, const Object* obj);

} // namespace internal

class Object
{
private:
    static std::atomic<int> ms_instances;

private:
    void* m_handle;

private:
    void _initialise_library();

protected:
    explicit Object(void* handle);
    explicit Object(std::nullptr_t);

    template <typename ApiFn, typename... Args>
    Object(ApiFn apiFn, Args... args)
    {
        _initialise_library();
        int res = apiFn(&m_handle, args...);
        internal::throw_on_object_ctor_failure(res, this);
    }

    void _destroy();

public:
    Object(const Object& rhs) = delete;
    Object& operator= (const Object& rhs) = delete;

    virtual ~Object();

    virtual const std::string& class_name() const;
    std::string to_string() const;
    void* handle() const
    {
        return m_handle;
    }
};

} // namespace yogi

std::ostream& operator<< (std::ostream& os, const yogi::Object& obj);

#endif // YOGICPP_OBJECT_HPP
