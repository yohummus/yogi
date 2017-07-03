#include "CustomCommandService.hh"
#include "../helpers/ostream.hh"
#include "../helpers/time.hh"

#include <QDir>
#include <QFile>
#include <QRegExp>

#include <exception>
#include <cassert>
#include <string>
using namespace std::string_literals;


namespace commands {

CustomCommandService::Command::Command(CustomCommandService& service, const CommandInfo& info, const QStringList& args)
: m_service(service)
, m_name(info.name)
{
    QRegExp rx("\\$\\{(\\*)|([<>]?\\d+)\\}");

    QStringList arguments;
    for (auto& arg : info.arguments) {
        if (rx.indexIn(arg) == -1) {
            arguments.push_back(arg);
        }
        else if (rx.cap(1) == "*") {
            arguments.append(args);
        }
        else {
            bool less = rx.cap(2).startsWith('<');
            bool greater = rx.cap(2).startsWith('>');
            int idx = rx.cap(2).mid((less || greater) ? 1 : 0).toInt();

            if (less) {
                for (int i = 0; i < args.size(); ++i) {
                    if (i < idx) {
                        arguments.append(args[i]);
                    }
                }
            }
            else if (greater) {
                for (int i = 0; i < args.size(); ++i) {
                    if (i > idx) {
                        arguments.append(args[i]);
                    }
                }
            }
            else {
                if (idx < args.size()) {
                    arguments.append(args[idx]);
                }
                else {
                    auto msg = "Not enough arguments for command '"s + info.name.toStdString() + "'";
                    YOGI_LOG_ERROR(m_service.m_logger, msg);
                    throw std::runtime_error(msg);
                }
            }
        }
    }

    QObject::connect(&m_process, &QProcess::started, [=] {
        YOGI_LOG_DEBUG(this->m_service.m_logger, "Command '" << this->m_name << "' started successfully");
        this->update_state();
    });

    QObject::connect<void (QProcess::*)(int, QProcess::ExitStatus)>(&m_process, &QProcess::finished, [=](int exitCode, auto exitStatus) {
        YOGI_LOG_DEBUG(this->m_service.m_logger, "Command '" << this->m_name << "' terminated with exit code " << exitCode);
        this->update_state();
    });

    QObject::connect(&m_process, &QProcess::readyReadStandardOutput, [=] {
        this->update_state();
    });

    QObject::connect(&m_process, &QProcess::readyReadStandardError, [=] {
        this->update_state();
    });

    m_process.setWorkingDirectory(info.workingDirectory);
    m_process.start(info.executable, arguments);

    YOGI_LOG_INFO(m_service.m_logger, "Running command '" << info.name << "' with arguments '" << arguments.join(' ') << "'...");
}

CustomCommandService::Command::~Command()
{
    m_process.close();
}

void CustomCommandService::Command::write_stdin(const QByteArray& data)
{
    m_process.write(data);
}

void CustomCommandService::Command::update_state()
{
    emit(m_service.process_update(this,
        m_process.state(),
        m_process.readAllStandardOutput(),
        m_process.readAllStandardError(),
        m_process.exitCode(),
        m_process.error()
    ));
}

CustomCommandService& CustomCommandService::instance()
{
    assert (ms_instance != nullptr);
    return *ms_instance;
}

CustomCommandService::CustomCommandService(QObject* parent)
: QObject(parent)
, m_logger("Custom Command Service")
{
    for (auto child : yogi::ProcessInterface::config().get_child("custom-commands")) {
        CommandInfo cmd;
        cmd.name = QString::fromStdString(child.first);
        cmd.enabled = child.second.get<bool>("enabled");

        cmd.executable = QString::fromStdString(child.second.get<std::string>("executable"));
        for (auto argChild : child.second.get_child("arguments")) {
            cmd.arguments.push_back(QString::fromStdString(argChild.second.get_value<std::string>()));
        }
        cmd.workingDirectory = QString::fromStdString(child.second.get<std::string>("working-directory"));
        cmd.maxRuntime = helpers::float_to_timeout(child.second.get<float>("max-runtime", 0));
        m_commands.insert(cmd.name, cmd);

        if (cmd.enabled) {
            check_executable_exists(cmd.executable);
            check_directory_exists(cmd.workingDirectory);
            YOGI_LOG_INFO(m_logger, "Custom command '" << cmd.name << "' enabled");
        } else {
            YOGI_LOG_DEBUG(m_logger, "Custom command '" << cmd.name << "' disabled");
            continue;
        }
    }

    assert (ms_instance == nullptr);
    ms_instance = this;
}

std::unique_ptr<CustomCommandService::Command> CustomCommandService::start_command(const QString& cmd, const QStringList& args)
{
    auto cmdIt = m_commands.find(cmd);
    if (cmdIt == m_commands.end()) {
        YOGI_LOG_ERROR(m_logger, "Cannot run unknown custom command '" << cmd << "'");
        return {};
    }

    if (!cmdIt->enabled) {
        YOGI_LOG_INFO(m_logger, "Denied request to run disabled custom command '" << cmd << "'");
        return {};
    }

    return std::make_unique<Command>(*this, *cmdIt, args);
}

CustomCommandService* CustomCommandService::ms_instance = nullptr;

void CustomCommandService::log_and_throw(const std::string& msg)
{
    YOGI_LOG_ERROR(m_logger, msg);
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

} // namespace commands
