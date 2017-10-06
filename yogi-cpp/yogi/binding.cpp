#include "binding.hpp"

#include "terminals.hpp"


namespace yogi {

Binding::Binding(PrimitiveTerminal& terminal, std::string targets)
: Object(YOGI_CreateBinding, terminal.handle(), targets.c_str())
, Binder(this)
, m_terminal(terminal)
, m_targets(targets)
{
}

Binding::~Binding()
{
    this->_destroy();
}

const std::string& Binding::class_name() const
{
    static std::string s = "Binding";
    return s;
}

} // namespace yogi
