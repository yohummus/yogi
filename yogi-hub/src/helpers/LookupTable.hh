#ifndef HELPERS_LOOKUPTABLE_HH
#define HELPERS_LOOKUPTABLE_HH

#include <unordered_map>
#include <memory>
#include <mutex>


namespace helpers {

template <typename T>
class LookupTable
{
public:
    typedef unsigned                              id_type;
    typedef std::shared_ptr<T>                    item_ptr;
    typedef std::unordered_map<id_type, item_ptr> item_map;
    typedef std::recursive_mutex                  mutex_type;
    typedef std::unique_lock<mutex_type>          lock_type;

    lock_type make_lock() const
    {
        return lock_type(m_mutex);
    }

    id_type add(item_ptr val)
    {
        auto lock = make_lock();

        auto id = ++m_lastId;
        m_lut[id] = val;

        return id;
    }

    item_ptr take(id_type id)
    {
        auto lock = make_lock();

        auto it = m_lut.find(id);
        if (it == m_lut.end()) {
            return {};
        }

        auto item = it->second;
        m_lut.erase(it);

        return item;
    }

    item_ptr get(id_type id)
    {
        auto lock = make_lock();

        auto it = m_lut.find(id);
        if (it == m_lut.end()) {
            return {};
        }
        else {
            return it->second;
        }
    }

private:
    id_type            m_lastId = 0;
    item_map           m_lut;
    mutable mutex_type m_mutex;
};

} // namespace helpers

#endif // HELPERS_LOOKUPTABLE_HH
