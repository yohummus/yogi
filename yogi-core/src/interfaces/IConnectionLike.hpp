#ifndef YOGI_INTERFACES_ICONNECTIONLIKE_HPP
#define YOGI_INTERFACES_ICONNECTIONLIKE_HPP

#include "../config.h"
#include "IPublicObject.hpp"

#include <vector>


namespace yogi {
namespace interfaces {

/***************************************************************************//**
 * Interface for connection-like objects
 ******************************************************************************/
struct IConnectionLike : virtual public IPublicObject
{
    virtual const std::string& description() const =0;
	virtual const std::string& remote_version() const =0;
	virtual const std::vector<char>& remote_identification() const =0;
};

typedef std::shared_ptr<IConnectionLike> connection_like_ptr;

} // namespace interfaces
} // namespace yogi

#endif // YOGI_INTERFACES_ICONNECTIONLIKE_HPP
