/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2019 Johannes Bergmann.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "common.h"
#include "../src/utils/crypto.h"
#include "../src/utils/system.h"
#include "../src/api/constants.h"
#include "../src/network/messages.h"

#include <boost/uuid/uuid_io.hpp>
#include <boost/asio.hpp>
#include <sstream>
using namespace std::string_literals;

namespace fs = boost::filesystem;

TestFixture::TestFixture() {
  // Logging for all tests
  // SetupLogging(YOGI_VB_TRACE);
}

TestFixture::~TestFixture() {
  EXPECT_EQ(YOGI_DestroyAll(), YOGI_OK);

  YOGI_ConfigureConsoleLogging(YOGI_VB_NONE, 0, 0, nullptr, nullptr);
  YOGI_ConfigureHookLogging(YOGI_VB_NONE, nullptr, nullptr);
  YOGI_ConfigureFileLogging(YOGI_VB_NONE, nullptr, nullptr, 0, nullptr,
                            nullptr);
}

BranchEventRecorder::BranchEventRecorder(void* context, void* branch)
    : context_(context), branch_(branch), json_str_(10000) {
  StartAwaitEvent();
}

nlohmann::json BranchEventRecorder::RunContextUntil(
    int event, const boost::uuids::uuid& uuid, int ev_res) {
  while (true) {
    for (auto& entry : events_) {
      if (entry.uuid == uuid && entry.event == event &&
          entry.ev_res == ev_res) {
        return entry.json;
      }
    }

    auto n = events_.size();
    while (n == events_.size()) {
      int res = YOGI_ContextRunOne(context_, nullptr, -1);
      EXPECT_OK(res);
    }
  }
}

nlohmann::json BranchEventRecorder::RunContextUntil(int event, void* branch,
                                                    int ev_res) {
  return RunContextUntil(event, GetBranchUuid(branch), ev_res);
}

void RunContextUntilBranchesAreConnected(
    void* context, std::initializer_list<void*> branches) {
  std::map<void*, std::set<boost::uuids::uuid>> uuids;
  for (auto branch : branches) {
    uuids[branch] = {};
  }
  for (auto& entry : uuids) {
    for (auto branch : branches) {
      if (branch != entry.first) {
        entry.second.insert(GetBranchUuid(branch));
      }
    }
  }

  auto start = std::chrono::steady_clock::now();

  while (!uuids.empty()) {
    auto res = YOGI_ContextPoll(context, nullptr);
    EXPECT_OK(res);

    auto entry = uuids.begin();
    auto infos = GetConnectedBranches(entry->first);
    for (auto& info : infos) {
      auto uuid = info.first;
      entry->second.erase(uuid);
    }

    if (entry->second.empty()) {
      uuids.erase(entry);
    }

    if (std::chrono::steady_clock::now() - start > 3s) {
      throw std::runtime_error("Branches did not connect");
    }
  }
}

void BranchEventRecorder::StartAwaitEvent() {
  int res = YOGI_BranchAwaitEventAsync(
      branch_, YOGI_BEV_ALL, &uuid_, json_str_.data(),
      static_cast<int>(json_str_.size()), &BranchEventRecorder::Callback, this);
  EXPECT_OK(res);
}

void BranchEventRecorder::Callback(int res, int event, int ev_res,
                                   void* userarg) {
  if (res == YOGI_ERR_CANCELED) return;

  auto self = static_cast<BranchEventRecorder*>(userarg);
  self->events_.push_back({self->uuid_,
                           nlohmann::json::parse(self->json_str_.data()), event,
                           ev_res});

  self->StartAwaitEvent();
}

MulticastSocket::MulticastSocket(
    const boost::asio::ip::udp::endpoint& multicast_ep)
    : mc_ep_(multicast_ep), socket_(ioc_, mc_ep_.protocol()) {
  using namespace boost::asio::ip;

  socket_.set_option(udp::socket::reuse_address(true));
  socket_.bind(udp::endpoint(mc_ep_.protocol(), mc_ep_.port()));

  auto ifs =
      utils::GetFilteredNetworkInterfaces({"localhost"}, mc_ep_.protocol());
  auto addr = ifs[0].addresses[0];

  if (addr.is_v6()) {
    socket_.set_option(multicast::join_group(mc_ep_.address().to_v6(),
                                             addr.to_v6().scope_id()));
    socket_.set_option(boost::asio::ip::multicast::outbound_interface(
        static_cast<unsigned int>(addr.to_v6().scope_id())));
  } else {
    socket_.set_option(
        multicast::join_group(mc_ep_.address().to_v4(), addr.to_v4()));
  }
}

void MulticastSocket::Send(const utils::ByteVector& msg) {
  socket_.send_to(boost::asio::buffer(msg), mc_ep_);
}

std::pair<boost::asio::ip::address, utils::ByteVector> MulticastSocket::Receive(
    const std::chrono::milliseconds& timeout) {
  utils::ByteVector msg(1000);
  boost::asio::ip::udp::endpoint sender_ep;
  socket_.async_receive_from(boost::asio::buffer(msg), sender_ep,
                             [&](auto ec, auto size) {
                               EXPECT_FALSE(ec) << ec.message();
                               msg.resize(size);
                             });

  ioc_.reset();
  if (!ioc_.run_one_for(timeout)) {
    throw std::runtime_error(
        "No multicast message received within the specified time.");
  }

  return {sender_ep.address(), msg};
}

FakeBranch::FakeBranch()
    : acceptor_(ioc_),
      tcp_socket_(ioc_),
      adv_ep_(boost::asio::ip::make_address(kAdvAddress), kAdvPort),
      mc_socket_(adv_ep_) {
  acceptor_.open(kTcpProtocol);
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(boost::asio::ip::tcp::endpoint(kTcpProtocol, 0));
  acceptor_.listen();

  auto adv_ifs =
      utils::GetFilteredNetworkInterfaces({"localhost"}, adv_ep_.protocol());

  nlohmann::json cfg = {
      {"name", "Fake Branch"},
      {"advertising_address", kAdvAddress},
      {"advertising_port", kAdvPort},
  };

  info_ = std::make_shared<objects::branch::detail::LocalBranchInfo>(
      cfg, adv_ifs, acceptor_.local_endpoint().port());
}

void FakeBranch::Connect(void* branch,
                         std::function<void(utils::ByteVector*)> msg_changer) {
  auto addr = mc_socket_.Receive().first;
  tcp_socket_.connect(
      boost::asio::ip::tcp::endpoint(addr, GetBranchTcpServerPort(branch)));
  Authenticate(msg_changer);
}

void FakeBranch::Accept(std::function<void(utils::ByteVector*)> msg_changer) {
  tcp_socket_ = acceptor_.accept();
  Authenticate(msg_changer);
}

void FakeBranch::Disconnect() {
  tcp_socket_.shutdown(tcp_socket_.shutdown_both);
  tcp_socket_.close();
}

void FakeBranch::Advertise(
    std::function<void(utils::ByteVector*)> msg_changer) {
  auto msg = *info_->MakeAdvertisingMessage();
  if (msg_changer) msg_changer(&msg);
  mc_socket_.Send(msg);
}

bool FakeBranch::IsConnectedTo(void* branch) const {
  struct Data {
    boost::uuids::uuid my_uuid;
    boost::uuids::uuid uuid;
    bool connected = false;
  } data;

  data.my_uuid = info_->GetUuid();

  int res = YOGI_BranchGetConnectedBranches(branch, &data.uuid, nullptr, 0,
                                            [](int, void* userarg) {
                                              auto data =
                                                  static_cast<Data*>(userarg);
                                              if (data->uuid == data->my_uuid) {
                                                data->connected = true;
                                              }
                                            },
                                            &data);
  EXPECT_OK(res);

  return data.connected;
}

void FakeBranch::Authenticate(
    std::function<void(utils::ByteVector*)> msg_changer) {
  // Send branch info
  auto info_msg = *info_->MakeInfoMessage();
  if (msg_changer) msg_changer(&info_msg);
  boost::asio::write(tcp_socket_, boost::asio::buffer(info_msg));

  // Receive branch info
  auto buffer = utils::ByteVector(
      objects::branch::detail::BranchInfo::kInfoMessageHeaderSize);
  boost::asio::read(tcp_socket_, boost::asio::buffer(buffer));
  std::size_t body_size;
  objects::branch::detail::RemoteBranchInfo::DeserializeInfoMessageBodySize(
      &body_size, buffer);
  buffer.resize(body_size);
  boost::asio::read(tcp_socket_, boost::asio::buffer(buffer));

  // ACK
  ExchangeAck();

  // Send challenge
  auto my_challenge = utils::GenerateRandomBytes(8);
  boost::asio::write(tcp_socket_, boost::asio::buffer(my_challenge));

  // Receive challenge
  auto remote_challenge = utils::ByteVector(8);
  boost::asio::read(tcp_socket_, boost::asio::buffer(remote_challenge));

  // Send solution
  auto password_hash = utils::MakeSha256(utils::ByteVector{});
  buffer = remote_challenge;
  buffer.insert(buffer.end(), password_hash.begin(), password_hash.end());
  auto remote_solution = utils::MakeSha256(buffer);
  boost::asio::write(tcp_socket_, boost::asio::buffer(remote_solution));

  // Receive Solution
  buffer.resize(remote_solution.size());
  boost::asio::read(tcp_socket_, boost::asio::buffer(buffer));

  // ACK
  ExchangeAck();
}

void FakeBranch::ExchangeAck() {
  auto buffer = utils::ByteVector{network::MessageType::kAcknowledge};
  boost::asio::write(tcp_socket_, boost::asio::buffer(buffer));
  boost::asio::read(tcp_socket_, boost::asio::buffer(buffer));
  EXPECT_EQ(buffer[0], network::MessageType::kAcknowledge);
}

TemporaryWorkdirGuard::TemporaryWorkdirGuard() {
  temp_path_ = fs::temp_directory_path() / fs::unique_path();
  fs::create_directory(temp_path_);
  old_working_dir_ = fs::current_path();
  fs::current_path(temp_path_);
}

TemporaryWorkdirGuard::TemporaryWorkdirGuard(TemporaryWorkdirGuard&& other) {
  old_working_dir_ = other.old_working_dir_;
  temp_path_ = other.temp_path_;
  other.temp_path_.clear();
}

TemporaryWorkdirGuard::~TemporaryWorkdirGuard() {
  if (!temp_path_.empty()) {
    fs::current_path(old_working_dir_);
    fs::remove_all(temp_path_);
  }
}

CommandLine::CommandLine(std::initializer_list<std::string> args) {
  argc = static_cast<int>(args.size() + 1);
  argv = new char*[static_cast<std::size_t>(argc)];

  std::string exe = "executable-name";
  argv[0] = new char[exe.size() + 1];
  std::copy(exe.begin(), exe.end(), argv[0]);
  argv[0][exe.size()] = '\0';

  auto it = args.begin();
  for (int i = 1; i < argc; ++i) {
    auto& arg = *it;
    argv[i] = new char[arg.size() + 1];
    std::copy(arg.begin(), arg.end(), argv[i]);
    argv[i][arg.size()] = '\0';
    ++it;
  }
}

CommandLine::~CommandLine() {
  for (int i = 0; i < argc; ++i) {
    delete[] argv[i];
  }

  delete[] argv;
}

std::string MakeVersionString(int major, int minor, int patch,
                              const std::string& suffix) {
  std::ostringstream ss;
  ss << major << '.' << minor << '.' << patch << suffix;
  return ss.str();
}

void SetupLogging(int verbosity) {
  // For calls through the public API
  int res = YOGI_ConfigureConsoleLogging(YOGI_VB_TRACE, YOGI_ST_STDERR,
                                         YOGI_TRUE, nullptr, nullptr);
  EXPECT_OK(res);

  res = YOGI_LoggerSetComponentsVerbosity("Yogi\\..*", verbosity, nullptr);
  EXPECT_OK(res);

  // For direct calls, circumventing the public API (needed because the shared
  // library and test memory space are separate)
  objects::log::Logger::SetSink(
      std::make_unique<objects::log::detail::ConsoleLogSink>(
          static_cast<api::Verbosity>(verbosity), stderr, true,
          api::kDefaultLogTimeFormat, api::kDefaultLogFormat));

  objects::log::Logger::SetComponentsVerbosity(
      std::regex("Yogi\\..*"), static_cast<api::Verbosity>(verbosity));
}

void* CreateContext() {
  void* context = nullptr;
  int res = YOGI_ContextCreate(&context);
  EXPECT_OK(res);
  EXPECT_NE(context, nullptr);
  return context;
}

void PollContext(void* context) {
  int res = YOGI_ContextPoll(context, nullptr);
  EXPECT_OK(res);
}

void PollContextOne(void* context) {
  int res = YOGI_ContextPollOne(context, nullptr);
  EXPECT_OK(res);
}

void RunContextInBackground(void* context) {
  int res = YOGI_ContextRunInBackground(context);
  EXPECT_OK(res);
  res = YOGI_ContextWaitForRunning(context, 1000000000);
  EXPECT_OK(res);
}

void* CreateBranch(void* context, const char* name, const char* net_name,
                   const char* password, const char* path, const char* adv_addr,
                   std::size_t transceive_byte_limit) {
  auto props = kBranchProps;
  props["description"] = "Description";
  if (name) props["name"] = name;
  if (net_name) props["network_name"] = net_name;
  if (password) props["network_password"] = password;
  if (path) props["path"] = path;
  if (adv_addr) props["advertising_address"] = adv_addr;

  if (transceive_byte_limit != std::numeric_limits<std::size_t>::max()) {
    props["_transceive_byte_limit"] = transceive_byte_limit;
  }

  char err[256];
  void* branch = nullptr;
  int res = YOGI_BranchCreate(&branch, context, MakeConfigFromJson(props),
                              nullptr, err, sizeof(err));
  EXPECT_OK(res) << err;

  return branch;
}

unsigned short GetBranchTcpServerPort(void* branch) {
  char json_str[1000] = {0};
  YOGI_BranchGetInfo(branch, nullptr, json_str, sizeof(json_str));

  auto json = nlohmann::json::parse(json_str);
  auto port = json["tcp_server_port"].get<unsigned short>();

  return port;
}

boost::uuids::uuid GetBranchUuid(void* branch) {
  boost::uuids::uuid uuid = {0};
  int res = YOGI_BranchGetInfo(branch, &uuid, nullptr, 0);
  EXPECT_OK(res);
  EXPECT_NE(uuid, boost::uuids::uuid{});
  return uuid;
}

nlohmann::json GetBranchInfo(void* branch) {
  char str[10000] = {0};
  int res = YOGI_BranchGetInfo(branch, nullptr, str, sizeof(str));
  EXPECT_OK(res);
  return nlohmann::json::parse(str);
}

void CheckJsonElementsAreEqual(const nlohmann::json& a, const nlohmann::json& b,
                               const std::string& key) {
  ASSERT_TRUE(a.count(key)) << "Key \"" << key << "\" does not exist in a";
  ASSERT_TRUE(b.count(key)) << "Key \"" << key << "\" does not exist in b";
  EXPECT_EQ(a[key].dump(), b[key].dump());
}

std::map<boost::uuids::uuid, nlohmann::json> GetConnectedBranches(
    void* branch) {
  struct Data {
    boost::uuids::uuid uuid;
    char json_str[1000] = {0};
    std::map<boost::uuids::uuid, nlohmann::json> branches;
  } data;

  int res = YOGI_BranchGetConnectedBranches(
      branch, &data.uuid, data.json_str, sizeof(data.json_str),
      [](int, void* userarg) {
        auto data = static_cast<Data*>(userarg);
        data->branches[data->uuid] = nlohmann::json::parse(data->json_str);
      },
      &data);
  EXPECT_OK(res);

  return data.branches;
}

std::string MakeTestDataPath(const std::string& data_path) {
  // YOGI_TEST_DATA_DIR is set in CMakeLists.txt
  return std::string(YOGI_TEST_DATA_DIR) + "/" + data_path;
}

std::string ReadFile(const std::string& filename) {
  std::ifstream f(filename);
  EXPECT_TRUE(f.is_open()) << filename;
  std::string content((std::istreambuf_iterator<char>(f)),
                      (std::istreambuf_iterator<char>()));
  return content;
}

void* MakeConfigFromJson(const nlohmann::json& json) {
  void* config;
  int res = YOGI_ConfigurationCreate(&config, YOGI_CFG_NONE);
  EXPECT_OK(res);

  res =
      YOGI_ConfigurationUpdateFromJson(config, json.dump().c_str(), nullptr, 0);
  EXPECT_OK(res);

  return config;
}

std::ostream& operator<<(std::ostream& os,
                         const std::chrono::nanoseconds& dur) {
  os << dur.count() << "ns";
  return os;
}

std::ostream& operator<<(std::ostream& os,
                         const std::chrono::microseconds& dur) {
  os << dur.count() << "us";
  return os;
}

std::ostream& operator<<(std::ostream& os,
                         const std::chrono::milliseconds& dur) {
  os << dur.count() << "ms";
  return os;
}

std::ostream& operator<<(std::ostream& os, const std::chrono::seconds& dur) {
  os << dur.count() << "s";
  return os;
}
