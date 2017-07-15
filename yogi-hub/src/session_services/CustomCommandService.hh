#ifndef SESSION_SERVICES_CUSTOMCOMMANDSERVICE_HH
#define SESSION_SERVICES_CUSTOMCOMMANDSERVICE_HH

#include "Service.hh"
#include "../yogi_network/YogiSession.hh"
#include "../helpers/LookupTable.hh"

#include <QStringList>
#include <QMap>
#include <QProcess>


namespace session_services {

class CustomCommandService : public Service
{
    Q_OBJECT

public:
    static void extract_command_details_from_config();

    CustomCommandService(yogi_network::YogiSession& session);

    virtual request_handlers_map make_request_handlers() override;

private:
    struct CommandDetails {
        QString                   name;
        QString                   executable;
        QStringList               arguments;
        QString                   workingDirectory;
        std::chrono::milliseconds maxRuntime;
    };

    class ActiveCommand;

    static std::unique_ptr<yogi::Logger> ms_logger;
    static QMap<QString, CommandDetails> ms_commands;

    yogi_network::YogiSession&           m_session;
    helpers::LookupTable<ActiveCommand>  m_activeCommands;

    static void log_and_throw(const std::string& msg);
    static void check_executable_exists(const QString& executable);
    static void check_directory_exists(const QString& dir);

    std::shared_ptr<ActiveCommand> start_command(const QString& cmd, const QStringList& args);

    response_pair handle_start_command_request(QByteArray* request);
    response_pair handle_terminate_command_request(QByteArray* request);
    response_pair handle_write_command_request(QByteArray* request);

    void on_process_updated(unsigned commandId, QProcess::ProcessState state, QByteArray out,
        QByteArray err, int exitCode, QProcess::ProcessError error);
};

class CustomCommandService::ActiveCommand
{
public:
    ActiveCommand(CustomCommandService& service, const CommandDetails& details, const QStringList& args);
    ~ActiveCommand();

    void set_id(unsigned id);
    unsigned id() const;
    void write_stdin(const QByteArray& data);

private:
    CustomCommandService&                       m_service;
    const CustomCommandService::CommandDetails& m_details;
    unsigned                                    m_id = 0;
    QProcess                                    m_process;

    QStringList make_translated_arguments(const QStringList& args);
    void connect_signals();
    void start(const QStringList& translatedArgs);
    void update_state();
};

} // namespace session_services

#endif // SESSION_SERVICES_CUSTOMCOMMANDSERVICE_HH
