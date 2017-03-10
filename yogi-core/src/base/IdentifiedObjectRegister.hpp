#ifndef CHIRP_BASE_IDENTIFIEDOBJECTREGISTER_HPP
#define CHIRP_BASE_IDENTIFIEDOBJECTREGISTER_HPP

#include "../config.h"
#include "Id.hpp"
#include "Identifier.hpp"

#include <unordered_map>
#include <vector>
#include <stack>
#include <memory>


namespace chirp {
namespace base {

/***************************************************************************//**
 * Manages a collection of elements that each have a unique ID and a unique
 * associated identifier
 *
 * @tparam TData Type of the elements' data
 ******************************************************************************/
template <typename TData>
class IdentifiedObjectRegister final
{
public:
    typedef TData      data_type;
    typedef Id         id_type;
    typedef Identifier identifier_type;

    class element_type final : public data_type
    {
        friend class IdentifiedObjectRegister;
        friend class iterator;

        element_type(const element_type&) = delete;
        element_type& operator= (const element_type&) = delete;

    private:
        IdentifiedObjectRegister& m_register;
        const identifier_type     m_identifier;
        const id_type             m_id;
        bool                      m_hidden;

    protected:
        template <typename TIdentifier, typename TId, typename TData_>
        element_type(IdentifiedObjectRegister& reg, TIdentifier&& identifier,
            TId&& id, TData_&& data)
            : data_type   {std::forward<TData_>(data)}
            , m_register  {reg}
            , m_identifier{std::forward<TIdentifier>(identifier)}
            , m_id        {std::forward<TId>(id)}
            , m_hidden    {false}
        {
        }

        IdentifiedObjectRegister& owner() const
        {
            return m_register;
        }

        void mark_as_hidden()
        {
            m_hidden = true;
        }

    public:
        /**
         * Returns the identifier of the element
         *
         * @return The identifier of the element
         */
        const identifier_type& identifier() const
        {
            return m_identifier;
        }

        /**
         * Returns the ID of the element
         *
         * @return The ID of the element
         */
        const id_type& id() const
        {
            return m_id;
        }

        /**
         * Checks whether the element is hidden or not
         *
         * @return True if the element is hidden
         */
        bool hidden() const
        {
            return m_hidden;
        }
    };

    class const_iterator : public std::iterator<std::forward_iterator_tag,
        element_type>
    {
        friend class IdentifiedObjectRegister;

    private:
        const element_type* m_element;

    private:
        const_iterator()
            : m_element{nullptr}
        {
        }

        explicit const_iterator(const element_type& element)
            : m_element{&element}
        {
        }

    public:
        const_iterator& operator++ ()
        {
            CHIRP_ASSERT(m_element != nullptr);

            auto idn = m_element->id().number();

            auto& lut = m_element->owner().m_lut;
            while (idn < lut.size() && lut[idn] == nullptr)
                ++idn;

            if (idn < lut.size()) {
                m_element = lut[idn].get();
            }
            else {
                m_element = nullptr;
            }

            return *this;
        }

        const_iterator operator++ (int)
        {
            const_iterator tmp{*this};
            operator++ ();
            return tmp;
        }

        bool operator== (const const_iterator& rhs) const
        {
            return m_element == rhs.m_element;
        }

        bool operator!= (const const_iterator& rhs) const
        {
            return !operator== (rhs);
        }

        const element_type* operator-> ()
        {
            CHIRP_ASSERT(m_element != nullptr);
            return m_element;
        }

        const element_type& operator* ()
        {
            return *operator->();
        }
    };

    class iterator : public const_iterator
    {
        friend class IdentifiedObjectRegister;

    private:
        iterator()
        {
        }

        explicit iterator(element_type& element)
            : const_iterator{element}
        {
        }

    public:
        iterator& operator++ ()
        {
            const_iterator::operator++();
            return *this;
        }

        iterator operator++ (int)
        {
            iterator tmp{*this};
            operator++ ();
            return tmp;
        }

        element_type* operator-> ()
        {
            return const_cast<element_type*>(const_iterator::operator->());
        }

        element_type& operator* ()
        {
            return *operator->();
        }
    };

    typedef std::unordered_map<identifier_type, element_type*> map_type;
    typedef std::vector<std::unique_ptr<element_type>>         lut_type;
    typedef std::stack<id_type>                                id_stack_type;

private:
    map_type      m_map;
    lut_type      m_lut;
    id_stack_type m_freeIds;

private:
    bool is_valid(iterator element) const
    {
        return element->id().valid() && element->id().number() <= m_lut.size()
            && m_lut[element->id().number() - 1].get() == &*element;
    }

public:
    /**
     * Inserts a new element
     *
     * @tparam TIdentifier Type of \p identifier
     * @tparam TData       Type of \p data
     * @param  identifier  Identifier of the element
     * @param  data        Argument to be passed to the data type's constructor
     * @return A pair consisting of a bool indicating whether insertion took
     *         place or not and an iterator of the inserted element or the
     *         already existing element if no insertion happened
     */
    template <typename TIdentifier, typename TData_ = data_type>
    std::pair<iterator, bool> insert(TIdentifier&& identifier, TData_&& data
        = data_type{})
    {
        // if there are no unused IDs, we create a new one
        if (m_freeIds.empty()) {
            m_freeIds.push(id_type{m_lut.size() + 1});
            m_lut.push_back(nullptr);
        }

        // try to insert new entry in the map
        auto res = m_map.emplace(identifier, nullptr);

        // if element was inserted, we construct a new element and update both 
        // the map and the lookup table entries
        if (res.second) {
            element_type* element;
            try {
                element = new element_type{*this, identifier, m_freeIds.top(),
                    std::forward<TData_>(data)};
            }
            catch (...) {
                m_map.erase(res.first);
                throw;
            }

            m_lut[m_freeIds.top().number() - 1].reset(element);
            res.first->second = element;
            m_freeIds.pop();
        }

        // return iterator of new element and a bool indicating whether
        // insertion did happen or not
        return std::make_pair(iterator{*res.first->second}, res.second);
    }

    /**
     * Removes an element from the register
     *
     * Erasing an element only invalidates the iterator of that specific
     * element.
     *
     * @param element The element to remove
     * @return Iterator to the next element in the register or an iterator of
     *         the end of the register if no next element exists
     */
    iterator erase(iterator element)
    {
        CHIRP_ASSERT(is_valid(element));

        iterator tmp = element;
        ++tmp;

        hide(element);
        m_freeIds.push(element->id());
        m_lut[element->id().number() - 1].reset();

        return tmp;
    }

    /**
     * Hides an element such that it can no longer be found using its identifier
     *
     * If the element is already hidden, this function will do nothing.
     *
     * @param element The element to hide
     */
    void hide(iterator element)
    {
        CHIRP_ASSERT(is_valid(element));
        CHIRP_ASSERT(element->hidden() || m_map.count(element->identifier()));

        if (!element->hidden()) {
            m_map.erase(element->identifier());
            element->mark_as_hidden();
        }
    }

    /**
     * Searches for an element using its ID
     *
     * @param id ID of the element
     * @return Iterator of the element with the matching ID or an iterator to
     *         the end of the register if no such element exists
     */
    iterator find(const id_type& id)
    {
        auto idn = id.number();
        if (0 < idn && idn <= m_lut.size() && m_lut[idn - 1]) {
            return iterator{*m_lut[idn - 1]};
        }

        return end();
    }

    /**
     * Searches for an element using its identifier
     *
     * @param identifier Identifier of the element
     * @return Iterator of the element with the matching identifier or an 
     *         iterator of the end of the register if no such element exists
     */
    iterator find(const identifier_type& identifier)
    {
        auto it = m_map.find(identifier);
        if (it != m_map.end()) {
            return iterator{*it->second};
        }

        return end();
    }

    /**
     * Finds an element using its ID
     *
     * An element with a matching ID must exist when calling this function.
     *
     * @param id ID of the element
     */
    element_type& operator[] (id_type id)
    {
        CHIRP_ASSERT(find(id) != end());
        return *m_lut[id.number() - 1];
    }

    /**
     * Finds an element using its identifier
     *
     * An element with a matching identifier must exist when calling this 
     * function.
     *
     * @param identifier Identifier of the element
     */
    element_type& operator[] (identifier_type identifier)
    {
        CHIRP_ASSERT(find(identifier) != end());
        return *m_map[identifier];
    }

    /**
     * Returns an iterator of the first element
     *
     * @return Iterator of the first element or an iterator of the end of the
     *         register if the register is empty
     */
    iterator begin()
    {
        for (const auto& element : m_lut) {
            if (element) {
                return iterator{*element};
            }
        }

        return end();
    }

    /**
     * Returns an iterator of the end of the register
     *
     * @return An iterator of the end of the register
     */
    iterator end()
    {
        return iterator{};
    }
};

} // namespace base
} // namespace chirp

#endif // CHIRP_BASE_IDENTIFIEDOBJECTREGISTER_HPP
