#include "Service.hh"


namespace http_services {

Service::~Service()
{
}

} // namespace http_services

std::ostream& operator<< (std::ostream& os, http_services::request_type requestType)
{
    switch (requestType) {
    case http_services::HTTP_GET:
        os << "GET";
        break;

    case http_services::HTTP_POST:
        os << "POST";
        break;

    case http_services::HTTP_PUT:
        os << "PUT";
        break;

    case http_services::HTTP_OPTIONS:
        os << "OPTIONS";
        break;

    case http_services::HTTP_DELETE:
        os << "DELETE";
        break;

    case http_services::HTTP_CONNECT:
        os << "CONNECT";
        break;
    }

    return os;
}
