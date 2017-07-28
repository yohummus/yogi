#include "DnsService.hh"
#include "../helpers/to_byte_array.hh"


namespace session_services {

DnsService::DnsService(yogi_network::YogiSession& session)
: m_session(session)
{
}

DnsService::request_handlers_map DnsService::make_request_handlers()
{
    return {{
        REQ_CLIENT_ADDRESS, [this](auto& request) {
            return this->handle_client_address_request(request);
        }}, {
        REQ_START_DNS_LOOKUP, [this](auto& request) {
            return this->handle_start_dns_lookup_request(request);
        }}
    };
}

DnsService::response_pair DnsService::handle_client_address_request(const QByteArray& request)
{
    auto addr = m_session.socket().peerAddress().toString();
    return {RES_OK, helpers::to_byte_array(addr)};
}

DnsService::response_pair DnsService::handle_start_dns_lookup_request(const QByteArray& request)
{
    if (request.size() < 2 || request.at(request.size() - 1) != '\0') {
		return {RES_INVALID_REQUEST, {}};
	}

	int id = QHostInfo::lookupHost(request.mid(1), this, SLOT(on_dns_lookup_finished(QHostInfo)));
    return {RES_OK, helpers::to_byte_array(id)};
}

void DnsService::on_dns_lookup_finished(QHostInfo info)
{
	QByteArray json;
	json += "{";

	json += "\"error\":";
	if (info.error() == QHostInfo::NoError) {
		json += "null";
	}
	else {
		json += "\"" + info.errorString() + "\"";
	}

	json += ",";

	json += "\"addresses\":[";
	if (!info.addresses().empty()) {
		for (auto& address : info.addresses()) {
			json += "\"" + address.toString() + "\",";
		}

		json.chop(1);
	}
	json += "]";

	json += ",";

	json += "\"hostname\":\"" + info.hostName() + "\"";

	json += "}";

	auto data = helpers::to_byte_array(info.lookupId()) + json;

    m_session.notify_client(ASY_DNS_LOOKUP, data);
}

} // namespace session_services
