#ifndef YOGI_BASE_OBJECTREGISTER_HPP
#define YOGI_BASE_OBJECTREGISTER_HPP

#include "../config.h"
#include "Id.hpp"

#include <vector>
#include <stack>
#include <memory>


namespace yogi {
namespace base {

/***************************************************************************//**
 * Manages a collection of elements that each have a unique ID
 *
 * @tparam TData Type of the elements' data
 ******************************************************************************/
template <typename TData>
class ObjectRegister final
{
public:
    typedef TData                  data_type;
    typedef Id                     id_type;
    typedef std::vector<data_type> lut_type;
    typedef std::stack<id_type>    id_stack_type;

private:
    lut_type      m_lut;
    id_stack_type m_freeIds;

private:
    bool is_valid(id_type id) const
    {
        if (id.number() > m_lut.size()) {
            return false;
        }

        id_stack_type freeIds = m_freeIds;
        while (!freeIds.empty()) {
            if (id == freeIds.top()) {
                return false;
            }

            freeIds.pop();
        }

        return true;
    }

public:
    template <typename TData_ = data_type>
    id_type insert(TData_&& data = data_type{})
    {
        if (m_freeIds.empty()) {
            id_type id{m_lut.size() + 1};
            m_lut.emplace_back(std::forward<TData_>(data));
            return id;
        }
        else {
            id_type id = m_freeIds.top();
            m_lut[id.number() - 1] = data_type(std::forward<TData_>(data));
            m_freeIds.pop();
            return id;
        }
    }

    void erase(id_type id)
    {
        YOGI_ASSERT(is_valid(id));

        m_lut[id.number() - 1] = data_type{};
        m_freeIds.push(id);
    }

#ifndef NDEBUG
    // This is not very efficient; use only for debugging
    std::size_t count(id_type id) const
    {
        return is_valid(id) ? 1 : 0;
    }
#endif

    const data_type& operator[] (id_type id) const
    {
        YOGI_ASSERT(is_valid(id));

        return m_lut[id.number() - 1];
    }

    data_type& operator[] (id_type id)
    {
        YOGI_ASSERT(is_valid(id));

        return m_lut[id.number() - 1];
    }
};

} // namespace base
} // namespace yogi

#endif // YOGI_BASE_OBJECTREGISTER_HPP
