#ifndef YOGI_API_PUBLICOBJECTREGISTER_HPP
#define YOGI_API_PUBLICOBJECTREGISTER_HPP

#include "../config.h"
#include "../interfaces/IPublicObject.hpp"
#include "../yogi_core.h"
#include "ExceptionT.hpp"

#include <mutex>
#include <unordered_map>
#include <algorithm>


namespace yogi {
namespace api {

/***************************************************************************//**
 * Singleton for managing objects that are created, used and destroyed through
 * the shared library interface and referred to via handles
 ******************************************************************************/
class PublicObjectRegister final
{
    typedef void* handle_type;
    typedef std::unordered_map<handle_type, interfaces::public_object_ptr>
        objects_map;

private:
    static std::mutex  ms_mutex;
    static objects_map ms_objects;

public:
    template <typename TO = interfaces::IPublicObject>
    static inline TO& get(handle_type handle)
    {
        return *static_cast<TO*>(handle);
    }

    template <typename TO = interfaces::IPublicObject>
    static TO& get_s(handle_type handle)
    {
        std::lock_guard<std::mutex> lock(ms_mutex);
        auto obj = ms_objects.find(handle);

        if (obj == ms_objects.end()) {
            throw ExceptionT<YOGI_ERR_INVALID_HANDLE>{};
        }

        TO* p = dynamic_cast<TO*>(obj->second.get());
        if (!p) {
            throw ExceptionT<YOGI_ERR_WRONG_OBJECT_TYPE>{};
        }

        return *p;
    }

    template <typename TO, typename... TArgs>
    static handle_type create(TArgs&&... args)
    {
        auto obj = std::make_shared<TO>(std::forward<TArgs>(args)...);

        std::lock_guard<std::mutex> lock(ms_mutex);

        handle_type handle = static_cast<handle_type>(obj.get());
        ms_objects[handle] = obj;

        return handle;
    }

    template <typename TO>
    static handle_type add(std::shared_ptr<TO> obj)
    {
        std::lock_guard<std::mutex> lock(ms_mutex);

        handle_type handle = static_cast<handle_type>(obj.get());
        YOGI_ASSERT(!ms_objects.count(handle));
        ms_objects[handle] = obj;

        return handle;
    }

    static void destroy(handle_type handle);
    static void clear();
};

} // namespace api
} // namespace yogi

#endif // YOGI_API_PUBLICOBJECTREGISTER_HPP
