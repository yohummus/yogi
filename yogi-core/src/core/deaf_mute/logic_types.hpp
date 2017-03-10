#ifndef CHIRP_CORE_DEAF_MUTE_LOGIC_TYPES_HPP
#define CHIRP_CORE_DEAF_MUTE_LOGIC_TYPES_HPP

#include "../../config.h"
#include "../../messaging/messages/DeafMute.hpp"
#include "../common/logic_types.hpp"


namespace chirp {
namespace core {
namespace deaf_mute {

template <typename TTypes>
class LeafLogic;

template <typename TTypes>
class NodeLogic;

template <typename TTypes>
class Terminal;

template <typename TMessages = messaging::messages::DeafMute>
struct logic_types
	: public common::logic_types
	, public TMessages
{
	typedef LeafLogic<logic_types> leaf_logic_type;
	typedef NodeLogic<logic_types> node_logic_type;
	typedef Terminal<logic_types> terminal_type;
};

} // namespace deaf_mute
} // namespace core
} // namespace chirp

#endif // CHIRP_CORE_DEAF_MUTE_LOGIC_TYPES_HPP
