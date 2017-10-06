#ifndef YOGI_INTERFACES_ITERMINAL_HPP
#define YOGI_INTERFACES_ITERMINAL_HPP

#include "../config.h"
#include "../base/Id.hpp"
#include "../base/Identifier.hpp"
#include "IPublicObject.hpp"


namespace yogi {
namespace interfaces {

struct ILeaf;

/***************************************************************************//**
 * Interface for terminals
 *
 * Terminals are communication endpoints assigned to leafs. There are different
 * types of terminals, such as scatter-gather or publish-subscribe based ones.
 ******************************************************************************/
struct ITerminal : virtual public IPublicObject
{
    virtual ILeaf& leaf() =0;
    virtual const base::Identifier& identifier() const =0;
    virtual base::Id id() const =0;
};

typedef std::shared_ptr<ITerminal> terminal_ptr;

} // namespace interfaces
} // namespace yogi

#endif // YOGI_INTERFACES_ITERMINAL_HPP
