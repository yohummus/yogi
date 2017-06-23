#include "KnownTerminalsMonitor.hh"
#include "../helpers/to_byte_array.hh"

#include <yogi_core.h>

#include <QStringList>

#include <cctype>
#include <cassert>


namespace yogi_network {

KnownTerminalsMonitor::KnownTerminalsMonitor(yogi::Node& node, QObject* parent)
: QObject(parent)
, m_node(node)
, m_logger("Known Terminals Monitor")
{
    assert (ms_instance == nullptr);
    ms_instance = this;

    connect(this, &KnownTerminalsMonitor::known_terminals_changed,
        this, &KnownTerminalsMonitor::on_known_terminals_changed);

    start_await_known_terminals_change();
}

KnownTerminalsMonitor::~KnownTerminalsMonitor()
{
    ms_instance = nullptr;
    m_node.cancel_await_known_terminals_change();
}

KnownTerminalsMonitor& KnownTerminalsMonitor::instance()
{
    assert (ms_instance != nullptr);
    return *ms_instance;
}

QByteArray KnownTerminalsMonitor::get_all_terminals() const
{
    QMutexLocker lock(&m_mutex);

    QByteArray data;
    for (const auto& terminal : m_terminals) {
        data += terminal;
    }

    return data;
}

QByteArray KnownTerminalsMonitor::get_terminals_subtree(QString path, bool absolute)
{
    QMutexLocker lock(&m_mutex);

    QByteArray data;

    TreeNode* root = absolute ? &m_absoluteTerminalsTree : &m_relativeTerminalsTree;
    auto tn = find_subtree(root, path);
    if (!tn) {
        return data;
    }

    for (auto childIt = tn->children.begin(); childIt != tn->children.end(); ++childIt) {
        data += childIt.key() + '\0';
        for (auto info : childIt->terminals) {
            data += info.type;
            data += helpers::to_byte_array(info.signature);
        }

        data += static_cast<char>(-1);
    }

    return data;
}

QByteArray KnownTerminalsMonitor::get_terminals_containing(QString nameSubstr, bool caseSensitive)
{
	QByteArray nameSubstrAsUtf8 = nameSubstr.toUtf8();
	auto stringContainsFn = caseSensitive
		? &KnownTerminalsMonitor::string_contains<true>
		: &KnownTerminalsMonitor::string_contains<false>;

	QMutexLocker lock(&m_mutex);

	QByteArray data;
	for (const auto& terminal : m_terminals) {
		if (stringContainsFn(terminal.constData() + 1 + 4, nameSubstrAsUtf8.constData())) {
			data += terminal;
		}
	}

	return data;
}

bool KnownTerminalsMonitor::TerminalInfo::operator== (const TerminalInfo& rhs) const
{
    return type == rhs.type && signature == rhs.signature;
}

KnownTerminalsMonitor* KnownTerminalsMonitor::ms_instance = nullptr;

void KnownTerminalsMonitor::start_await_known_terminals_change()
{
    m_node.async_await_known_terminals_change([=](auto& res, auto&& info, auto change) {
        if (!res) {
            return;
        }

        auto changeStr = change == yogi::change_type::ADDED ? "added" : "removed";
        YOGI_LOG_DEBUG(this->m_logger, info.type << " '" << info.name << "' [" << info.signature << "] " << changeStr);
        this->handle_known_terminals_changed(res, std::move(info), change);
    });
}

void KnownTerminalsMonitor::handle_known_terminals_changed(const yogi::Result& res,
    yogi::terminal_info&& info, yogi::change_type change)
{
    if (!res) {
        return;
    }

    QByteArray blob;
    blob += helpers::to_byte_array(change);
    blob += helpers::to_byte_array(info.type);
    blob += helpers::to_byte_array(info.signature);
    blob += helpers::to_byte_array(info.name);
    emit(known_terminals_changed(blob));

    start_await_known_terminals_change();
}

KnownTerminalsMonitor::TreeNode* KnownTerminalsMonitor::find_subtree(TreeNode* root, QString path)
{
    if (path.isEmpty() || path == "/") {
        return root;
    }

    if (path.startsWith('/')) {
        path = path.mid(1);
    }

    auto parts = path.split('/');

    TreeNode* node = root;
    for (auto part : parts) {
        auto it = node->children.find(part);
        if (it == node->children.end()) {
            return nullptr;
        }

        node = &it.value();
    }

    return node;
}

KnownTerminalsMonitor::TreeNode& KnownTerminalsMonitor::create_subtree(QString path)
{
    auto parts = path.split('/');
    if (!parts.isEmpty() && parts.first().isEmpty()) {
        parts.removeFirst();
    }

    TreeNode* node = path.startsWith('/') ? &m_absoluteTerminalsTree : &m_relativeTerminalsTree;
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

template <bool CaseSensitive>
bool KnownTerminalsMonitor::string_contains(const char* str, const char* substr)
{
	auto charsAreEqual = [](char a, char b) {
		return CaseSensitive ? (a == b) : (std::tolower(a) == std::tolower(b));
	};

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

void KnownTerminalsMonitor::on_known_terminals_changed(QByteArray blob)
{
    TerminalInfo info;
    info.type      = static_cast<yogi::terminal_type>(blob.at(1));
    info.signature = yogi::Signature(*reinterpret_cast<unsigned*>(blob.data() + 2));

    auto       change   = static_cast<yogi::change_type>(blob.at(0));
    QByteArray terminal = blob.mid(1);
    QString    name     = blob.mid(1 + 1 + 4);

    QMutexLocker lock(&m_mutex);
    if (change == yogi::change_type::ADDED) {
        assert (!m_terminals.contains(terminal));
        m_terminals.insert(terminal);

        TreeNode& node = create_subtree(name);
        node.terminals.append(info);
    }
    else {
        assert (m_terminals.contains(terminal));
        m_terminals.remove(terminal);

        TreeNode* root = name.startsWith('/') ? &m_absoluteTerminalsTree : &m_relativeTerminalsTree;
        TreeNode* node = find_subtree(root, name);
        assert (node != nullptr);
        assert (node->terminals.indexOf(info) != -1);
        node->terminals.remove(node->terminals.indexOf(info));

        while (node->terminals.empty() && node->children.empty() && node->parent) {
            auto parent = node->parent;
            parent->children.remove(QString(node->name));
            node = parent;
        }
    }
}

} // namespace yogi_network
