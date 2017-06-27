#include "QueryService.hh"
#include "../helpers/config.hh"

#include <yogi.hpp>


namespace http_services {

QueryService::QueryService(const yogi::ConfigurationChild& config)
: m_logger("Query Service")
, m_queries(helpers::extract_map_from_config(config, "queries"))
{
}

void QueryService::async_handle_request(request_type type, const QString& path,
	const QMap<QString, QString>& header, const QByteArray& content, completion_handler completionHandler)
{
	if (type != HTTP_GET) {
		completionHandler(HTTP_404, {}, {}, false);
        return;
	}

    auto queryPath = path.mid(1);
    for (auto path : m_queries.keys()) {
        if (queryPath == path) {
            completionHandler(HTTP_201, m_queries[path].toUtf8(), "text/plain", false);
            return;
        }
    }

    completionHandler(HTTP_404, {}, {}, false);
}

} // namespace http_services
