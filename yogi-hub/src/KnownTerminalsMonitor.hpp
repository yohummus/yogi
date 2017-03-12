#ifndef KNOWNTERMINALSMONITOR_HPP
#define KNOWNTERMINALSMONITOR_HPP

#include <yogi_cpp.hpp>

#include <QObject>
#include <QByteArray>
#include <QSet>
#include <QMap>
#include <QMutex>
#include <QVector>


class KnownTerminalsMonitor : public QObject
{
    Q_OBJECT

    struct TerminalInfo {
        yogi::terminal_type type;
        yogi::Signature     signature;

        bool operator== (const TerminalInfo& rhs) const
        {
            return type == rhs.type && signature == rhs.signature;
        }
    };

    struct TreeNode {
        TreeNode*               parent = nullptr;
        QString                 name;
        QMap<QString, TreeNode> children;
        QVector<TerminalInfo>   terminals;
    };

private:
    static KnownTerminalsMonitor* ms_instance;

    yogi::Node&  m_node;
    yogi::Logger m_logger;
    QSet<QByteArray> m_terminals;
    TreeNode         m_absoluteTerminalsTree;
    TreeNode         m_relativeTerminalsTree;
    mutable QMutex   m_mutex;

Q_SIGNALS:
    void known_terminals_changed(QByteArray);

private Q_SLOTS:
    void on_known_terminals_changed(QByteArray blob);

private:
    void start_await_known_terminals_change();
    void handle_known_terminals_changed(const yogi::Result& res, yogi::terminal_info&& info, yogi::change_type change);
    TreeNode* find_subtree(TreeNode* root, QString path);
    TreeNode& create_subtree(QString path);

	template <bool CaseSensitive>
	static bool string_contains(const char* str, const char* substr);

public:
    KnownTerminalsMonitor(yogi::Node& node, QObject* parent = Q_NULLPTR);
    ~KnownTerminalsMonitor();
    static KnownTerminalsMonitor& instance();

    QByteArray get_all_terminals() const;
    QByteArray get_terminals_subtree(QString path, bool absolute);
	QByteArray get_terminals_containing(QString nameSubstr, bool caseSensitive);
};

#endif // KNOWNTERMINALSMONITOR_HPP
