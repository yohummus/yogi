#include "PublicObjectRegister.hpp"


namespace chirp {
namespace api {

std::mutex                        PublicObjectRegister::ms_mutex;
PublicObjectRegister::objects_map PublicObjectRegister::ms_objects;

void PublicObjectRegister::destroy(handle_type handle)
{
    interfaces::public_object_ptr obj; // destroy object after lock released

    std::unique_lock<std::mutex> lock(ms_mutex);
    auto it = ms_objects.find(handle);

    if (it == ms_objects.end()) {
        throw ExceptionT<CHIRP_ERR_INVALID_HANDLE>{};
    }

    if (!it->second.unique()) {
        throw ExceptionT<CHIRP_ERR_OBJECT_STILL_USED>{};
    }

    obj = it->second;
    ms_objects.erase(it);
}

void PublicObjectRegister::clear()
{
    objects_map objects;

    {{
        std::lock_guard<std::mutex> lock(ms_mutex);
        std::swap(ms_objects, objects);
    }}
        
    auto oldSize = std::numeric_limits<std::size_t>::max();
    while (objects.size() < oldSize) {
        oldSize = objects.size();

        for (auto it = objects.begin(); it != objects.end(); ) {
            if (it->second.unique()) {
                it = objects.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    CHIRP_ASSERT(objects.empty());
}

} // namespace api
} // namespace chirp
