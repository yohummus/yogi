#include "TimeService.hh"
#include "../helpers/to_byte_array.hh"

#include <chrono>


namespace session_services {

TimeService::TimeService(yogi_network::YogiSession& session)
{
}

TimeService::request_handlers_map TimeService::make_request_handlers()
{
    return {{
        REQ_CURRENT_TIME, [this](auto& request) {
            return this->handle_current_time_request(request);
        }}
    };
}

TimeService::response_pair TimeService::handle_current_time_request(const QByteArray& request)
{
    return {RES_OK, helpers::to_byte_array(std::chrono::system_clock::now())};
}

} // namespace session_services
