#ifndef CHIRP_MESSAGING_FIELDS_FIELDS_HPP
#define CHIRP_MESSAGING_FIELDS_FIELDS_HPP

#include "../../config.h"
#include "../../base/Id.hpp"
#include "../../base/Identifier.hpp"
#include "../../base/Buffer.hpp"
#include "../../core/scatter_gather/gather_flags.hpp"


namespace chirp {
namespace messaging {
namespace fields {

static struct Identifier {
	typedef base::Identifier type;
	static inline const char* name() { return "identifier"; };
} identifier;

static struct Id {
	typedef base::Id type;
	static inline const char* name() { return "id"; };
} id;

static struct TerminalId {
	typedef base::Id type;
	static inline const char* name() { return "terminalId"; };
} terminalId;

static struct BindingId {
	typedef base::Id type;
	static inline const char* name() { return "bindingId"; };
} bindingId;

static struct SubscriptionId {
	typedef base::Id type;
	static inline const char* name() { return "subscriptionId"; };
} subscriptionId;

static struct OperationId {
	typedef base::Id type;
	static inline const char* name() { return "operationId"; };
} operationId;

static struct MappedId {
	typedef base::Id type;
	static inline const char* name() { return "mappedId"; };
} mappedId;

static struct Data {
	typedef base::Buffer type;
	static inline const char* name() { return "data"; };
} data;

static struct GatherFlags {
	typedef core::scatter_gather::gather_flags type;
	static inline const char* name() { return "gatherFlags"; };
} gatherFlags;

} // namespace fields
} // namespace messaging
} // namespace chirp

#endif // CHIRP_MESSAGING_FIELDS_FIELDS_HPP
