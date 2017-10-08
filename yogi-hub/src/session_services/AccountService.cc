#include "AccountService.hh"
#include "../helpers/to_byte_array.hh"
#include "../helpers/read_from_stream.hh"

using namespace std::string_literals;


namespace session_services {

void AccountService::init()
{
    qRegisterMetaType<std::string>("std::string");

    ms_accountManagerLocation = yogi::ProcessInterface::config().get<std::string>(
        "account-service.manager-location");

    ms_createSessionTerminal = std::make_unique<decltype(ms_createSessionTerminal)::element_type>(
        ms_accountManagerLocation / "Create Web Session");

    ms_dependency = std::make_unique<decltype(ms_dependency)::element_type>(
        ms_accountManagerLocation, *ms_createSessionTerminal);
}

void AccountService::destroy()
{
    ms_dependency.reset();
    ms_createSessionTerminal.reset();
}

AccountService::AccountService(yogi_network::YogiSession& session)
: m_session(session)
, m_logger("Account Service")
, m_loggingIn(false)
, m_loggedIn(false)
{
}

AccountService::request_handlers_map AccountService::make_request_handlers()
{
    return {{
        REQ_START_LOGIN_TASK, [this](auto& request) {
            return this->handle_start_login_task_request(request);
        }}, {
        REQ_START_STORE_DATA_TASK, [this](auto& request) {
            return this->handle_start_store_data_task_request(request);
        }}, {
        REQ_START_READ_DATA_TASK, [this](auto& request) {
            return this->handle_start_read_data_task_request(request);
        }}
    };
}

yogi::Path                                  AccountService::ms_accountManagerLocation;
AccountService::create_session_terminal_ptr AccountService::ms_createSessionTerminal;
std::unique_ptr<yogi::ProcessDependency>    AccountService::ms_dependency;

AccountService::response_pair AccountService::handle_start_login_task_request(const QByteArray& request)
{
    if (request.size() < 7 || count_strings(request, 5) != 2) {
		return {RES_INVALID_REQUEST, {}};
    }

    auto taskId = get_task_id(request);
    auto username = std::string(request.data() + 5);
    auto password = std::string(request.data() + 5 + username.size() + 1);

    if (m_loggedIn) {
        YOGI_LOG_DEBUG(m_logger, "[" << taskId << ":" << username << "] Login rejected since user"
            << " is alreay logged in as " << m_username);
        return {RES_ALREADY_LOGGED_IN, {}};
    }

    if (m_loggingIn) {
        YOGI_LOG_WARNING(m_logger, "[" << taskId << ":" << username << "] Login failed since a"
            << " login request is already running");
        return {RES_NOT_READY, {}};
    }

    YOGI_LOG_DEBUG(m_logger, "[" << taskId << ":" << username << "] Login request received with "
        << password.size() << " byte password");

    m_loginTaskId = taskId;
    m_username = username;

    if (!send_create_session_request(password)) {
        return {RES_NOT_READY, {}};
    }

    m_loggingIn = true;

    return {RES_OK, {}};
}

bool AccountService::send_create_session_request(const std::string& password)
{
    auto msg = ms_createSessionTerminal->make_request_message();
    decltype(msg)::Pair pair;
    msg.set_allocated_value(&pair);
    pair.set_first(m_username);
    pair.set_second(password);
    auto op = ms_createSessionTerminal->try_async_request(msg, [=](auto& res, auto&& response) {
        return this->on_login_request_answer_received(res, std::move(response));
    });
    msg.release_value();

    if (!op) {
        YOGI_LOG_WARNING(m_logger, "[" << m_loginTaskId << ":" << m_username << "] Login failed"
            << " since " << ms_createSessionTerminal->name() << " is not ready");
    }

    return !!op;
}

yogi::control_flow AccountService::on_login_request_answer_received(const yogi::Result& res,
    create_session_terminal::Response&& response)
{
    bool success = false;
    std::string webSessionName;

    if (res) {
        success = response.message().value().first();
        webSessionName = response.message().value().second();
    }

    QMetaObject::invokeMethod(this, "handle_login_request_answer_in_qt_thread",
        Qt::QueuedConnection,
        Q_ARG(int, res.value()),
        Q_ARG(bool, success),
        Q_ARG(std::string, webSessionName)
    );

    return yogi::STOP;
}

void AccountService::handle_login_request_answer_in_qt_thread(int res_, bool success,
    const std::string& webSessionName)
{
    auto res = yogi::Result(res_);

    if (res && success) {
        m_webSessionName = webSessionName;

        setup_service_leaf();
        setup_account_terminals();
        setup_session_terminals();
    }
    else {
        if (!res) {
            YOGI_LOG_ERROR(m_logger, "[" << m_loginTaskId << ":" << m_username
                << "] Failed to handle login request: " << res);
        }
        else {
            YOGI_LOG_WARNING(m_logger, "[" << m_loginTaskId << ":" << m_username
                << "] Login failed (probably bad credentials)");
        }

        m_loggingIn = false;

        auto data = helpers::to_byte_array(m_loginTaskId);
        m_session.notify_client(ASY_TASK_COMPLETED, data);
    }
}

void AccountService::try_completing_login_request_in_qt_thread()
{
    if (m_loggedIn) {
        return;
    }

    if (!m_firstName || !m_lastName || !m_groups) {
        return;
    }

    yogi::Subscribable* tms[] = {
        &*m_storeAccountDataTerminal,
        &*m_readAccountDataTerminal,
        &*m_storeSessionDataTerminal,
        &*m_readSessionDataTerminal,
        &*m_terminateTerminal
    };

    for (auto tm : tms) {
        if (tm->get_subscription_state() == yogi::UNSUBSCRIBED) {
            return;
        }
    }

    m_loggingIn = false;
    m_loggedIn = true;

    m_firstNameObserver.reset();
    m_firstNameTerminal.reset();
    m_lastNameObserver .reset();
    m_lastNameTerminal .reset();
    m_groupsObserver   .reset();
    m_groupsTerminal   .reset();

    m_storeAccountDataSubObserver.reset();
    m_readAccountDataSubObserver .reset();
    m_storeSessionDataSubObserver.reset();
    m_readSessionDataSubObserver .reset();
    m_terminateSubObserver       .reset();

    YOGI_LOG_INFO(m_logger, "[" << m_loginTaskId << ":" << m_username
        << "] Login successful; session is " << m_webSessionName);

    auto data = helpers::to_byte_array(m_loginTaskId)
              + helpers::to_byte_array(m_webSessionName);

    for (auto& group : *m_groups) {
        data += helpers::to_byte_array(group);
    }

    m_session.notify_client(ASY_TASK_COMPLETED, data);
}

AccountService::response_pair AccountService::handle_start_store_data_task_request(
    const QByteArray& request)
{
    if (request.size() < 7 || count_strings(request, 6) < 1) {
        return {RES_INVALID_REQUEST, {}};
    }

    if (!m_loggedIn) {
        return {RES_NOT_LOGGED_IN, {}};
    }

    auto taskId        = get_task_id(request);
    bool isAccountData = request[5] != 0;
    auto variable      = std::string(request.data() + 6);
    auto data          = request.mid(6 + variable.size() + 1);

    if (variable.empty()) {
        return {RES_INVALID_REQUEST, {}};
    }

    YOGI_LOG_DEBUG(m_logger, "[" << taskId << ":" << m_username << "] Storing " << data.size()
        << " bytes in " << (isAccountData ? "account" : "session") << " variable '" << variable
        << "'...");

    if (!send_store_data_request(taskId, isAccountData, variable, data)) {
        return {RES_NOT_READY, {}};
    }

    return {RES_OK, {}};
}

bool AccountService::send_store_data_request(unsigned taskId, bool isAccountData,
    const std::string& variable, const QByteArray& data)
{
    auto tm = isAccountData ? &*m_storeAccountDataTerminal : &*m_storeSessionDataTerminal;
    auto msg = tm->make_request_message();
    decltype(msg)::Pair pair;
    msg.set_allocated_value(&pair);
    pair.set_first(variable);
    pair.set_second(data.toStdString());
    auto op = tm->try_async_request(msg, [=](auto& res, auto&& response) {
        return this->on_store_data_request_answer_received(res, std::move(response), taskId);
    });
    msg.release_value();

    if (!op) {
        YOGI_LOG_WARNING(m_logger, "[" << taskId << ":" << m_username << "] Storing data failed"
            << " since " << tm->name() << " is not ready");
    }

    return !!op;
}

yogi::control_flow AccountService::on_store_data_request_answer_received(const yogi::Result& res,
    store_data_terminal::Response&& response, unsigned taskId)
{
    bool success = false;

    if (res) {
        success = response.message().value();
    }

    QMetaObject::invokeMethod(this, "handle_store_data_request_answer_in_qt_thread",
        Qt::QueuedConnection,
        Q_ARG(int, res.value()),
        Q_ARG(bool, success),
        Q_ARG(unsigned, taskId)
    );

    return yogi::STOP;
}

void AccountService::handle_store_data_request_answer_in_qt_thread(int res_, bool success,
    unsigned taskId)
{
    auto res = yogi::Result(res_);

    if (!res) {
        YOGI_LOG_ERROR(m_logger, "[" << taskId << ":" << m_username << "] Failed to handle store"
            << " data request: " << res);
    }
    else if (!success) {
        YOGI_LOG_ERROR(m_logger, "[" << taskId << ":" << m_username << "] Failed to store data;"
            << " check log files for yogi-account-manager");
    }
    else {
        YOGI_LOG_DEBUG(m_logger, "[" << taskId << ":" << m_username << "] Successfully stored data");
    }

    auto data = helpers::to_byte_array(taskId)
              + helpers::to_byte_array(res && success);
    m_session.notify_client(ASY_TASK_COMPLETED, data);
}

AccountService::response_pair AccountService::handle_start_read_data_task_request(
    const QByteArray& request)
{
    if (request.size() < 7 || count_strings(request, 6) < 1) {
        return {RES_INVALID_REQUEST, {}};
    }

    if (!m_loggedIn) {
        return {RES_NOT_LOGGED_IN, {}};
    }

    auto taskId        = get_task_id(request);
    bool isAccountData = request[5] != 0;
    auto variable      = std::string(request.data() + 6);

    if (variable.empty()) {
        return {RES_INVALID_REQUEST, {}};
    }

    YOGI_LOG_DEBUG(m_logger, "[" << taskId << ":" << m_username << "] Reading "
        << (isAccountData ? "account" : "session") << " variable '" << variable << "'...");

    if (!send_read_data_request(taskId, isAccountData, variable)) {
        return {RES_NOT_READY, {}};
    }

    return {RES_OK, {}};
}

bool AccountService::send_read_data_request(unsigned taskId, bool isAccountData,
    const std::string& variable)
{
    auto tm = isAccountData ? &*m_readAccountDataTerminal : &*m_readSessionDataTerminal;
    auto msg = tm->make_request_message();
    msg.set_value(variable);
    auto op = tm->try_async_request(msg, [=](auto& res, auto&& response) {
        return this->on_read_data_request_answer_received(res, std::move(response), taskId);
    });

    if (!op) {
        YOGI_LOG_WARNING(m_logger, "[" << taskId << ":" << m_username << "] Reading data failed"
            << " since " << tm->name() << " is not ready");
    }

    return !!op;
}

yogi::control_flow AccountService::on_read_data_request_answer_received(const yogi::Result& res,
    read_data_terminal::Response&& response, unsigned taskId)
{
    bool success = false;
    QByteArray data;

    if (res) {
        success = response.message().value().first();
        data = QByteArray::fromStdString(response.message().value().second());
    }

    QMetaObject::invokeMethod(this, "handle_read_data_request_answer_in_qt_thread",
        Qt::QueuedConnection,
        Q_ARG(int, res.value()),
        Q_ARG(bool, success),
        Q_ARG(QByteArray, data),
        Q_ARG(unsigned, taskId)
    );

    return yogi::STOP;
}

Q_SLOT void AccountService::handle_read_data_request_answer_in_qt_thread(int res_, bool success,
    const QByteArray& readData, unsigned taskId)
{
    auto res = yogi::Result(res_);

    if (!res) {
        YOGI_LOG_ERROR(m_logger, "[" << taskId << ":" << m_username << "] Failed to handle read"
            << " data request: " << res);
    }
    else if (!success) {
        YOGI_LOG_ERROR(m_logger, "[" << taskId << ":" << m_username << "] Failed to read data;"
            << " check log files for yogi-account-manager");
    }
    else {
        YOGI_LOG_DEBUG(m_logger, "[" << taskId << ":" << m_username << "] Successfully read "
            << readData.size() << " bytes of data");
    }

    auto data = helpers::to_byte_array(taskId)
              + helpers::to_byte_array(res && success)
              + readData;
    m_session.notify_client(ASY_TASK_COMPLETED, data);
}

unsigned AccountService::get_task_id(const QByteArray& request)
{
    QDataStream stream(request);
    stream.skipRawData(1);
    auto taskId = helpers::read_from_stream<unsigned>(&stream);
    return taskId;
}

int AccountService::count_strings(const QByteArray& request, int offset)
{
    int n = 0;
    int pos = request.indexOf('\0', offset);
    while (pos != -1 && pos < request.size()) {
        ++n;
        pos = request.indexOf('\0', pos + 1);
    }

    return n;
}

void AccountService::setup_service_leaf()
{
    m_serviceLeaf = std::make_unique<yogi::Leaf>(m_session.leaf().scheduler());
    m_serviceConnection = std::make_unique<yogi::LocalConnection>(m_session.node(), *m_serviceLeaf);
}

void AccountService::setup_account_terminals()
{
    setup_consumer(&m_firstNameTerminal, &m_firstNameObserver, "First Name", [=](auto& msg) {
        this->m_firstName = msg.value();
    });

    setup_consumer(&m_lastNameTerminal, &m_lastNameObserver, "Last Name", [=](auto& msg) {
        this->m_lastName = msg.value();
    });

    setup_consumer(&m_groupsTerminal, &m_groupsObserver, "Groups", [=](auto& msg) {
        this->m_groups = std::vector<std::string>();
        for (int i = 0; i < msg.value_size(); ++i) {
            this->m_groups->push_back(msg.value(i));
        }
    });

    setup_client(&m_storeAccountDataTerminal, &m_storeAccountDataSubObserver, "Store Data", false);
    setup_client(&m_readAccountDataTerminal, &m_readAccountDataSubObserver, "Read Data", false);
}

void AccountService::setup_session_terminals()
{
    setup_client(&m_storeSessionDataTerminal, &m_storeSessionDataSubObserver, "Store Data", true);
    setup_client(&m_readSessionDataTerminal, &m_readSessionDataSubObserver, "Read Data", true);
    setup_client(&m_terminateTerminal, &m_terminateSubObserver, "Terminate", true);
}

template <typename Terminal, typename Fn>
void AccountService::setup_consumer(std::unique_ptr<Terminal>* tm,
    std::unique_ptr<yogi::MessageObserver<Terminal>>* ob, const char* tmName, Fn fn)
{
    *tm = std::make_unique<Terminal>(*m_serviceLeaf, ms_accountManagerLocation / "Accounts"
        / m_username / tmName);

    auto wrappedFn = [=, rawTm=tm->get()](auto& msg) {
        YOGI_LOG_TRACE(this->m_logger, "[" << m_loginTaskId << ":" << m_username
            << "] Received value" << " from " << rawTm->name());

        fn(msg);

        QMetaObject::invokeMethod(this, "try_completing_login_request_in_qt_thread",
            Qt::QueuedConnection
        );
    };

    *ob = std::make_unique<yogi::MessageObserver<Terminal>>(**tm);
    (*ob)->add(wrappedFn);
    (*ob)->start();

    try {
        auto msg = (*tm)->get_cached_message();
        wrappedFn(msg);
    }
    catch (const yogi::Failure&) {
    }
}

template <typename Terminal>
void AccountService::setup_client(std::unique_ptr<Terminal>* tm,
    std::unique_ptr<yogi::SubscriptionObserver>* ob, const char* tmName, bool sessionSpecific)
{
    auto path = sessionSpecific
              ? ms_accountManagerLocation / "Sessions" / m_webSessionName / tmName
              : ms_accountManagerLocation / "Accounts" / m_username / tmName;

    *tm = std::make_unique<Terminal>(*m_serviceLeaf, path);
    *ob = std::make_unique<yogi::SubscriptionObserver>(**tm);
    (*ob)->add([=, rawTm=tm->get()](bool subscribed) {
        YOGI_LOG_TRACE(this->m_logger, "[" << m_loginTaskId << ":" << m_username << "] "
            << rawTm->name() << " now " << (subscribed ? "available" : "unavailable"));

        QMetaObject::invokeMethod(this, "try_completing_login_request_in_qt_thread",
            Qt::QueuedConnection
        );
    });
    (*ob)->start();
}

} // namespace session_services
