#ifndef COMMANDS_CUSTOMCOMMANDSERVICE_HPP
#define COMMANDS_CUSTOMCOMMANDSERVICE_HPP

#include <yogi.hpp>

#include <QByteArray>
#include <QStringList>
#include <QMap>
#include <QProcess>

#include <chrono>
#include <memory>


namespace commands {

class CustomCommandService : public QObject
{
    Q_OBJECT
    struct CommandInfo;

public:
    class Command
    {
    public:
        Command(CustomCommandService& service, const CommandInfo& info, const QStringList& args);
        ~Command();

        void write_stdin(const QByteArray& data);

    private:
        CustomCommandService& m_service;
        QProcess              m_process;
        QString               m_name;

        void update_state();
    };

    static CustomCommandService& instance();

    CustomCommandService(QObject* parent = Q_NULLPTR);

    std::unique_ptr<Command> start_command(const QString& cmd, const QStringList& args);

Q_SIGNALS:
    void process_update(Command*, QProcess::ProcessState running, QByteArray out, QByteArray err, int exitCode, QProcess::ProcessError);

private:
    struct CommandInfo {
        QString                   name;
        bool                      enabled;
        QString                   executable;
        QStringList               arguments;
        QString                   workingDirectory;
        std::chrono::milliseconds maxRuntime;
    };

    static CustomCommandService* ms_instance;

    yogi::Logger                 m_logger;
    QMap<QString, CommandInfo>   m_commands;

    void log_and_throw(const std::string& msg);
    void check_executable_exists(const QString& executable);
    void check_directory_exists(const QString& dir);
};

} // namespace commands

#endif // COMMANDS_CUSTOMCOMMANDSERVICE_HPP
