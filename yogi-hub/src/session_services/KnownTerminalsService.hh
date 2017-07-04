#ifndef SESSION_SERVICES_KNOWNTERMINALSERVICE_HH
#define SESSION_SERVICES_KNOWNTERMINALSERVICE_HH

#include "Service.hh"
#include "../yogi_network/YogiSession.hh"

#include <QByteArray>
#include <QMap>
#include <QMutex>
#include <QVector>
#include <QSet>

#include <yogi.hpp>


namespace session_services {

class KnownTerminalsService : public Service
{
    Q_OBJECT

public:
    KnownTerminalsService(yogi::Node& node);
    KnownTerminalsService(yogi_network::YogiSession& session);
    virtual ~KnownTerminalsService();

    virtual request_handlers_map make_request_handlers() override;

Q_SIGNALS:
    void known_terminals_changed(QByteArray);

private:
    struct TerminalInfo {
        yogi::terminal_type type;
        yogi::Signature     signature;

        bool operator== (const TerminalInfo& rhs) const;
    };

    struct TreeNode {
        TreeNode*               parent = nullptr;
        QString                 name;
        QMap<QString, TreeNode> children;
        QVector<TerminalInfo>   terminals;
    };

    static std::vector<KnownTerminalsService*> ms_instances;
    static yogi::Node*                         ms_node;
    static std::unique_ptr<yogi::Logger>       ms_logger;
    static QSet<QByteArray>                    ms_terminals;
    static TreeNode                            ms_absoluteTerminalsTree;
    static TreeNode                            ms_relativeTerminalsTree;
    static QMutex                              ms_mutex;

    yogi_network::YogiSession* m_session;
    std::atomic<bool>          m_monitorTerminals;

	static bool string_contains(const char* str, const char* substr, bool caseSensitive);
    static void start_await_known_terminals_change();
    static void on_known_terminals_changed(const yogi::terminal_info& info, yogi::change_type change);
    static void update_known_terminals_state_and_notify_sessions(const yogi::terminal_info& info, yogi::change_type change);
    static void add_terminal(const QString& name, const TerminalInfo& ti, const QByteArray& blob);
    static void remove_terminal(const QString& name, const TerminalInfo& ti, const QByteArray& blob);
    static TreeNode* find_subtree(TreeNode* root, const QString& path);
    static TreeNode& create_subtree(const QString& path);

    response_pair handle_known_terminals_request(QByteArray* request);
    response_pair handle_known_terminals_subtree_request(QByteArray* request);
    response_pair handle_find_known_terminals_request(QByteArray* request);
    response_pair handle_monitor_known_terminals_request(QByteArray* request);
};

} // namespace session_services

#endif // SESSION_SERVICES_KNOWNTERMINALSERVICE_HH
