#include "CustomCommandService.hh"
#include "../helpers/to_byte_array.hh"
#include "../helpers/read_from_stream.hh"
#include "../helpers/ostream.hh"
#include "../helpers/time.hh"

#include <QDir>
#include <QFile>
#include <QRegExp>
#include <QDataStream>

using namespace std::string_literals;


namespace session_services {

void CustomCommandService::extract_command_details_from_config()
{
    ms_logger = std::make_unique<yogi::Logger>("Custom Command Service");

    for (auto child : yogi::ProcessInterface::config().get_child("custom-commands")) {
        CommandDetails cmd;
        cmd.name = QString::fromStdString(child.first);

        if (!child.second.get<bool>("enabled")) {
            YOGI_LOG_DEBUG(*ms_logger, "Custom command '" << cmd.name << "' disabled");
            continue;
        }

        cmd.executable = QString::fromStdString(child.second.get<std::string>("executable"));
        for (auto argChild : child.second.get_child("arguments")) {
            cmd.arguments.push_back(QString::fromStdString(argChild.second.get_value<std::string>()));
        }
        cmd.workingDirectory = QString::fromStdString(child.second.get<std::string>("working-directory"));
        cmd.maxRuntime = helpers::float_to_timeout(child.second.get<float>("max-runtime", 0));
        ms_commands.insert(cmd.name, cmd);

        check_executable_exists(cmd.executable);
        check_directory_exists(cmd.workingDirectory);
        YOGI_LOG_INFO(*ms_logger, "Custom command '" << cmd.name << "' enabled");
    }
}

CustomCommandService::CustomCommandService(yogi_network::YogiSession& session)
: m_session(session)
{
}

CustomCommandService::request_handlers_map CustomCommandService::make_request_handlers()
{
    return {{
        REQ_START_CUSTOM_COMMAND, [this](auto& request) {
            return this->handle_start_command_request(request);
        }}, {
        REQ_TERMINATE_CUSTOM_COMMAND, [this](auto& request) {
            return this->handle_terminate_command_request(request);
        }}, {
        REQ_WRITE_CUSTOM_COMMAND_STDIN, [this](auto& request) {
            return this->handle_write_command_request(request);
        }}
    };
}

std::unique_ptr<yogi::Logger> CustomCommandService::ms_logger;
QMap<QString, CustomCommandService::CommandDetails> CustomCommandService::ms_commands;

void CustomCommandService::log_and_throw(const std::string& msg)
{
    YOGI_LOG_ERROR(*ms_logger, msg);
    throw std::runtime_error(msg);
}

void CustomCommandService::check_executable_exists(const QString& executable)
{
    if (!QFile::exists(executable)) {
        log_and_throw("Cannot find executable '"s + executable.toStdString() + "'");
    }
}

void CustomCommandService::check_directory_exists(const QString& dir)
{
    if (!QDir(dir).exists()) {
        log_and_throw("Cannot find directory '"s + dir.toStdString() + "'");
    }
}

std::shared_ptr<CustomCommandService::ActiveCommand> CustomCommandService::start_command(
    const QString& cmd, const QStringList& args)
{
    auto cmdIt = ms_commands.find(cmd);
    if (cmdIt == ms_commands.end()) {
        YOGI_LOG_ERROR(*ms_logger, "Cannot run unknown custom command '" << cmd << "'");
        return {};
    }

    return std::make_shared<ActiveCommand>(*this, *cmdIt, args);
}

CustomCommandService::response_pair CustomCommandService::handle_start_command_request(const QByteArray& request)
{
    if (request.size() < 2 || request.at(request.size() - 1) != '\0') {
        return {RES_INVALID_REQUEST, {}};
    }

    auto parts = request.mid(1, request.length() - 2).split('\0');
    QString cmd = parts[0];
    QStringList args;
    for (int i = 1; i < parts.size(); ++i) {
        args.push_back(parts[i]);
    }

    auto command = start_command(cmd, args);
    if (!command) {
        return {RES_INVALID_REQUEST, {}};
    }

    auto id = m_activeCommands.add(command);
    command->set_id(id);

    return {RES_OK, helpers::to_byte_array(id)};
}

CustomCommandService::response_pair CustomCommandService::handle_terminate_command_request(const QByteArray& request)
{
    QDataStream stream(request);
    stream.skipRawData(1);

    auto id = helpers::read_from_stream<unsigned>(&stream);
    auto command = m_activeCommands.take(id);

    if (command) {
        return {RES_OK, {}};
    }
    else {
        return {RES_INVALID_COMMAND_ID, {}};
    }
}

CustomCommandService::response_pair CustomCommandService::handle_write_command_request(const QByteArray& request)
{
    QDataStream stream(request);
    stream.skipRawData(1);

    auto id = helpers::read_from_stream<unsigned>(&stream);
    auto command = m_activeCommands.get(id);

    if (!command) {
        return {RES_INVALID_COMMAND_ID, {}};
    }

    command->write_stdin(request.mid(5));
    return {RES_OK, {}};
}

void CustomCommandService::on_process_updated(unsigned commandId, QProcess::ProcessState state, QByteArray out,
    QByteArray err, int exitCode, QProcess::ProcessError error)
{
    QByteArray data;
    data += helpers::to_byte_array(commandId);
    data += helpers::to_byte_array(state == QProcess::ProcessState::Running);
    data += helpers::to_byte_array(exitCode);
    data += helpers::to_byte_array(out.size());
    data += out;
    data += helpers::to_byte_array(err.size());
    data += err;

    switch (error) {
    case QProcess::ProcessError::FailedToStart: data += helpers::to_byte_array("Failed to start"s); break;
    case QProcess::ProcessError::Crashed:       data += helpers::to_byte_array("Crashed"s);         break;
    case QProcess::ProcessError::Timedout:      data += helpers::to_byte_array("Timed out"s);       break;
    case QProcess::ProcessError::ReadError:     data += helpers::to_byte_array("Read error"s);      break;
    case QProcess::ProcessError::WriteError:    data += helpers::to_byte_array("Write error"s);     break;
    default:                                    data += helpers::to_byte_array(""s);                break;
    }

    m_session.notify_client(ASY_CUSTOM_COMMAND_STATE, data);
}

CustomCommandService::ActiveCommand::ActiveCommand(CustomCommandService& service,
    const CommandDetails& details, const QStringList& args)
: m_service(service)
, m_details(details)
{
    auto translatedArgs = make_translated_arguments(args);
    connect_signals();
    start(translatedArgs);
}

CustomCommandService::ActiveCommand::~ActiveCommand()
{
    m_process.close();
}

void CustomCommandService::ActiveCommand::set_id(unsigned id)
{
    m_id = id;
}

unsigned CustomCommandService::ActiveCommand::id() const
{
    return m_id;
}

void CustomCommandService::ActiveCommand::write_stdin(const QByteArray& data)
{
    m_process.write(data);
}

QStringList CustomCommandService::ActiveCommand::make_translated_arguments(const QStringList& args)
{
    QRegExp rx("\\$\\{(\\*)|([<>]?\\d+)\\}");

    QStringList translatedArgs;
    for (auto& arg : m_details.arguments) {
        if (rx.indexIn(arg) == -1) {
            translatedArgs.push_back(arg);
        }
        else if (rx.cap(1) == "*") {
            translatedArgs.append(args);
        }
        else {
            bool less = rx.cap(2).startsWith('<');
            bool greater = rx.cap(2).startsWith('>');
            int idx = rx.cap(2).mid((less || greater) ? 1 : 0).toInt();

            if (less) {
                for (int i = 0; i < args.size(); ++i) {
                    if (i < idx) {
                        translatedArgs.append(args[i]);
                    }
                }
            }
            else if (greater) {
                for (int i = 0; i < args.size(); ++i) {
                    if (i > idx) {
                        translatedArgs.append(args[i]);
                    }
                }
            }
            else {
                if (idx < args.size()) {
                    translatedArgs.append(args[idx]);
                }
                else {
                    auto msg = "Not enough arguments for command '"s + m_details.name.toStdString() + "'";
                    YOGI_LOG_ERROR(*m_service.ms_logger, msg);
                    throw std::runtime_error(msg);
                }
            }
        }
    }

    return translatedArgs;
}

void CustomCommandService::ActiveCommand::connect_signals()
{
    QObject::connect(&m_process, &QProcess::started, [=] {
        YOGI_LOG_DEBUG(*this->m_service.ms_logger, "Command '" << this->m_details.name
            << "' started successfully");
        this->update_state();
    });

    QObject::connect<void (QProcess::*)(int, QProcess::ExitStatus)>(&m_process, &QProcess::finished, [=](int exitCode, auto exitStatus) {
        YOGI_LOG_DEBUG(*this->m_service.ms_logger, "Command '" << this->m_details.name
            << "' terminated with exit code " << exitCode);
        this->update_state();
    });

    QObject::connect(&m_process, &QProcess::readyReadStandardOutput, [=] {
        this->update_state();
    });

    QObject::connect(&m_process, &QProcess::readyReadStandardError, [=] {
        this->update_state();
    });
}

void CustomCommandService::ActiveCommand::start(const QStringList& translatedArgs)
{
    m_process.setWorkingDirectory(m_details.workingDirectory);
    m_process.start(m_details.executable, translatedArgs);

    YOGI_LOG_INFO(*m_service.ms_logger, "Running command '" << m_details.name << "' with arguments '"
        << translatedArgs.join(' ') << "'...");
}

void CustomCommandService::ActiveCommand::update_state()
{
    m_service.on_process_updated(
        m_id,
        m_process.state(),
        m_process.readAllStandardOutput(),
        m_process.readAllStandardError(),
        m_process.exitCode(),
        m_process.error()
    );
}

} // namespace session_services
