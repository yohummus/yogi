#ifndef YOGICPP_BINDING_HPP
#define YOGICPP_BINDING_HPP

#include "object.hpp"
#include "binder.hpp"


namespace yogi {

class PrimitiveTerminal;

class Binding : public Object, public Binder
{
private:
    PrimitiveTerminal& m_terminal;
    const std::string  m_targets;

public:
    Binding(PrimitiveTerminal& terminal, std::string targets);
    virtual ~Binding();

    virtual const std::string& class_name() const override;

    const PrimitiveTerminal& terminal() const
    {
        return m_terminal;
    }

    PrimitiveTerminal& terminal()
    {
        return m_terminal;
    }

    const std::string& targets() const
    {
        return m_targets;
    }
};

} // namespace yogi

#endif // YOGICPP_BINDING_HPP
