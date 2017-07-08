#include "VersionService.hh"
#include "../helpers/to_byte_array.hh"

#include <yogi.hpp>


namespace session_services {

VersionService::VersionService(yogi_network::YogiSession& session)
{
}

VersionService::request_handlers_map VersionService::make_request_handlers()
{
    return {{
        REQ_VERSION, [this](auto request) {
            return this->handle_version_request(request);
        }}
    };
}

VersionService::response_pair VersionService::handle_version_request(QByteArray* request)
{
    return {RES_OK, QByteArray::fromStdString(yogi::get_version())};
}

} // namespace session_services
