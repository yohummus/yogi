#ifndef YOGI_INTERFACES_ISCHEDULER_HPP
#define YOGI_INTERFACES_ISCHEDULER_HPP

#include "../config.h"
#include "IPublicObject.hpp"

#include <boost/asio/io_service.hpp>


namespace yogi {
namespace interfaces {

/***************************************************************************//**
 * Interface for schedulers
 *
 * Schedulers decide when and how to execute asynchronous operations. There are
 * no constraints in which order those operations have to be executed.
 ******************************************************************************/
struct IScheduler : virtual public IPublicObject
{
    virtual boost::asio::io_service& io_service() =0;

    template <typename TFn>
    void dispatch(TFn fn)
    {
        io_service().dispatch(fn);
    }

    template <typename TFn>
    void post(TFn fn)
    {
        io_service().post(fn);
    }
};

typedef std::shared_ptr<IScheduler> scheduler_ptr;

} // namespace interfaces
} // namespace yogi

#endif // YOGI_INTERFACES_ISCHEDULER_HPP
