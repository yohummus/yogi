#ifndef CHIRP_CORE_SCATTER_GATHER_GATHER_FLAGS_HPP
#define CHIRP_CORE_SCATTER_GATHER_GATHER_FLAGS_HPP

#include "../../config.h"

#include <cstdint>
#include <ostream>
#include <vector>

namespace chirp {
namespace core {
namespace scatter_gather {

enum gather_flags
{
    GATHER_NO_FLAGS         = 0,
    GATHER_FINISHED         = (1<<0),
    GATHER_IGNORED          = (1<<1),
    GATHER_DEAF             = (1<<2),
    GATHER_BINDINGDESTROYED = (1<<3),
    GATHER_CONNECTIONLOST   = (1<<4),
};

inline gather_flags operator& (gather_flags a, gather_flags b)
{
    return static_cast<gather_flags>(static_cast<int>(a) & static_cast<int>(b));
}

inline gather_flags operator| (gather_flags a, gather_flags b)
{
    return static_cast<gather_flags>(static_cast<int>(a) | static_cast<int>(b));
}

inline gather_flags operator~ (gather_flags a)
{
    return static_cast<gather_flags>(~static_cast<int>(a));
}

inline gather_flags& operator|= (gather_flags& a, gather_flags b)
{
    return a = a | b;
}

inline gather_flags& operator&= (gather_flags& a, gather_flags b)
{
    return a = a & b;
}

} // namespace scatter_gather
} // namespace core
} // namespace chirp

namespace std {

inline std::ostream& operator<< (std::ostream& os,
	chirp::core::scatter_gather::gather_flags flags)
{
	using namespace chirp::core::scatter_gather;

	std::vector<const char*> v;
	if (flags == GATHER_NO_FLAGS) {
		v.push_back("NO_FLAGS");
	}
	else {
		if (flags & GATHER_FINISHED) {
			v.push_back("FINISHED");
		}
		if (flags & GATHER_IGNORED) {
			v.push_back("IGNORED");
		}
		if (flags & GATHER_DEAF) {
			v.push_back("DEAF");
		}
		if (flags & GATHER_BINDINGDESTROYED) {
			v.push_back("BINDINGDESTROYED");
		}
		if (flags & GATHER_CONNECTIONLOST) {
			v.push_back("CONNECTIONLOST");
		}
	}

	for (std::size_t i = 0; i < v.size(); ++i) {
		os << v[i];
		if (i < v.size() - 1) {
			os << " | ";
		}
	}

	return os;
}

} // namespace std

#endif // CHIRP_CORE_SCATTER_GATHER_GATHER_FLAGS_HPP
