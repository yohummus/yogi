#include "KnownTerminalsService.hh"
#include "../helpers/to_byte_array.hh"

#include <yogi_core.h>

#include <cctype>
#include <algorithm>

using namespace std::placeholders;


namespace session_services {

KnownTerminalsService::KnownTerminalsService(yogi::Node& node)
: m_session(nullptr)
, m_monitorTerminals(false)
{
    QMutexLocker lock(&ms_mutex);
    if (ms_instances.empty()) {
        ms_node = &node;
        ms_logger = std::make_unique<yogi::Logger>("Known Terminals Service");
        start_await_known_terminals_change();
    }

    ms_instances.push_back(this);
}

KnownTerminalsService::KnownTerminalsService(yogi_network::YogiSession& session)
: KnownTerminalsService(session.node())
{
    m_session = &session;
}

KnownTerminalsService::~KnownTerminalsService()
{
    QMutexLocker lock(&ms_mutex);
    ms_instances.erase(std::remove(ms_instances.begin(), ms_instances.end(), this), ms_instances.end());
    if (ms_instances.empty()) {
        ms_node->cancel_await_known_terminals_change();
        ms_logger.reset();
        ms_terminals.clear();
        ms_absoluteTerminalsTree.children.clear();
        ms_absoluteTerminalsTree.terminals.clear();
        ms_relativeTerminalsTree.children.clear();
        ms_relativeTerminalsTree.terminals.clear();
    }
}

KnownTerminalsService::request_handlers_map KnownTerminalsService::make_request_handlers()
{
    return {{
        REQ_KNOWN_TERMINALS, [this](auto& request) {
            return this->handle_known_terminals_request(request);
        }}, {
        REQ_KNOWN_TERMINALS_SUBTREE, [this](auto& request) {
            return this->handle_known_terminals_subtree_request(request);
        }}, {
        REQ_FIND_KNOWN_TERMINALS, [this](auto& request) {
            return this->handle_find_known_terminals_request(request);
        }}, {
        REQ_MONITOR_KNOWN_TERMINALS, [this](auto& request) {
            return this->handle_monitor_known_terminals_request(request);
        }}
    };
}

bool KnownTerminalsService::TerminalInfo::operator== (const TerminalInfo& rhs) const
{
    return type == rhs.type && signature == rhs.signature;
}

std::vector<KnownTerminalsService*> KnownTerminalsService::ms_instances;
std::unique_ptr<yogi::Logger>       KnownTerminalsService::ms_logger;
yogi::Node*                         KnownTerminalsService::ms_node;
QSet<QByteArray>                    KnownTerminalsService::ms_terminals;
KnownTerminalsService::TreeNode     KnownTerminalsService::ms_absoluteTerminalsTree;
KnownTerminalsService::TreeNode     KnownTerminalsService::ms_relativeTerminalsTree;
QMutex                              KnownTerminalsService::ms_mutex;

bool KnownTerminalsService::string_contains(const char* str, const char* substr, bool caseSensitive)
{
	auto charsAreEqual = caseSensitive
                       ? [](char a, char b) { return a == b; }
                       : [](char a, char b) { return std::tolower(a) == std::tolower(b); };

	do {
		while (charsAreEqual(*str, *substr) && *substr) {
			++str;
			++substr;
		}

		if (*substr == '\0') {
			return true;
		}
	} while (*str++);

	return false;
}

void KnownTerminalsService::start_await_known_terminals_change()
{
    ms_node->async_await_known_terminals_change([=](auto& res, auto&& info, auto change) {
        if (res) {
            on_known_terminals_changed(info, change);
        }
    });
}

void KnownTerminalsService::on_known_terminals_changed(const yogi::terminal_info& info, yogi::change_type change)
{
    auto changeStr = change == yogi::change_type::ADDED ? "added" : "removed";
    YOGI_LOG_DEBUG(*ms_logger, info.type << " '" << info.name << "' [" << info.signature << "] " << changeStr);

    update_known_terminals_state_and_notify_sessions(info, change);

    start_await_known_terminals_change();
}

void KnownTerminalsService::update_known_terminals_state_and_notify_sessions(const yogi::terminal_info& info,
    yogi::change_type change)
{
    auto name = QString::fromStdString(info.name);

    TerminalInfo ti;
    ti.type      = info.type;
    ti.signature = info.signature;

    QByteArray blob;
    blob += helpers::to_byte_array(info.type);
    blob += helpers::to_byte_array(info.signature);
    blob += helpers::to_byte_array(info.name);

    QMutexLocker lock(&ms_mutex);
    if (change == yogi::change_type::ADDED) {
        add_terminal(name, ti, blob);
    }
    else {
        remove_terminal(name, ti, blob);
    }

    auto changeBlob = helpers::to_byte_array(change) + blob;
    for (auto kts : ms_instances) {
        if (kts->m_monitorTerminals) {
            kts->m_session->notify_client(MON_KNOWN_TERMINALS_CHANGED, changeBlob);
        }
    }
}

void KnownTerminalsService::add_terminal(const QString& name, const TerminalInfo& ti, const QByteArray& blob)
{
    assert (!ms_terminals.contains(blob));
    ms_terminals.insert(blob);

    TreeNode& node = create_subtree(name);
    node.terminals.append(ti);
}

void KnownTerminalsService::remove_terminal(const QString& name, const TerminalInfo& ti, const QByteArray& blob)
{
    assert (ms_terminals.contains(blob));
    ms_terminals.remove(blob);

    auto root = name.startsWith('/') ? &ms_absoluteTerminalsTree : &ms_relativeTerminalsTree;
    auto node = find_subtree(root, name);
    assert (node != nullptr);
    assert (node->terminals.indexOf(ti) != -1);
    node->terminals.remove(node->terminals.indexOf(ti));

    while (node->terminals.empty() && node->children.empty() && node->parent) {
        auto parent = node->parent;
        assert (parent->children.contains(node->name));
        parent->children.remove(QString(node->name)); // copy required since node will be destroyed in remove()
        node = parent;
    }
}

KnownTerminalsService::TreeNode* KnownTerminalsService::find_subtree(TreeNode* root, const QString& path)
{
    if (path.isEmpty() || path == "/") {
        return root;
    }

    auto parts = path.startsWith('/')
               ? path.mid(1).split('/')
               : path.split('/');

    auto node = root;
    for (auto part : parts) {
        auto it = node->children.find(part);
        if (it == node->children.end()) {
            return nullptr;
        }

        node = &it.value();
    }

    return node;
}

KnownTerminalsService::TreeNode& KnownTerminalsService::create_subtree(const QString& path)
{
    auto parts = path.split('/');
    if (!parts.isEmpty() && parts.first().isEmpty()) {
        parts.removeFirst();
    }

    auto node = path.startsWith('/') ? &ms_absoluteTerminalsTree : &ms_relativeTerminalsTree;
    for (auto part : parts) {
        auto it = node->children.find(part);
        if (it == node->children.end()) {
            TreeNode newChild;
            newChild.parent = node;
            newChild.name = part;
            node = &node->children.insert(part, newChild).value();
        } else {
            node = &it.value();
        }
    }

    return *node;
}

Service::response_pair KnownTerminalsService::handle_known_terminals_request(const QByteArray& request)
{
    QByteArray response;

    {{
        QMutexLocker lock(&ms_mutex);
        for (const auto& terminal : ms_terminals) {
            response += terminal;
        }
    }}

    return {RES_OK, response};
}

Service::response_pair KnownTerminalsService::handle_known_terminals_subtree_request(const QByteArray& request)
{
    bool absolute = request.at(1) ? true : false;
    QString path = request.mid(2);

    QByteArray response;

    {{
        QMutexLocker lock(&ms_mutex);

        auto root = absolute ? &ms_absoluteTerminalsTree : &ms_relativeTerminalsTree;
        auto tn = find_subtree(root, path);
        if (tn) {
            for (auto childIt = tn->children.begin(); childIt != tn->children.end(); ++childIt) {
                response += childIt.key() + '\0';
                for (auto info : childIt->terminals) {
                    response += info.type;
                    response += helpers::to_byte_array(info.signature);
                }

                response += static_cast<char>(-1);
            }
        }
    }}

    return {RES_OK, response};
}

Service::response_pair KnownTerminalsService::handle_find_known_terminals_request(const QByteArray& request)
{
	bool caseSensitive = request.at(1) ? true : false;
    QByteArray nameSubstr = request.mid(2);

	QByteArray response;

    {{
        QMutexLocker lock(&ms_mutex);

        for (const auto& terminal : ms_terminals) {
            if (string_contains(terminal.constData() + 1 + 4, nameSubstr.constData(), caseSensitive)) {
                response += terminal;
            }
        }
    }}

    return {RES_OK, response};
}

Service::response_pair KnownTerminalsService::handle_monitor_known_terminals_request(const QByteArray& request)
{
    if (m_monitorTerminals) {
        return {RES_ALREADY_MONITORING, {}};
    }
    else {
        m_monitorTerminals = true;
        return handle_known_terminals_request(request);
    }
}

} // namespace session_services
