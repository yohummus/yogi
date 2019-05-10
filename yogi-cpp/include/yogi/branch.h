/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2018 Johannes Bergmann.
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

#ifndef YOGI_BRANCH_H
#define YOGI_BRANCH_H

//! \file
//!
//! Branches (entry points into the Yogi network).

#include "object.h"
#include "context.h"
#include "io.h"
#include "uuid.h"
#include "duration.h"
#include "timestamp.h"
#include "constants.h"
#include "json.h"
#include "string_view.h"
#include "payload_view.h"
#include "operation_id.h"
#include "buffer.h"
#include "internal/library.h"
#include "internal/flags.h"
#include "internal/query_string.h"

#include <unordered_map>

namespace yogi {

_YOGI_DEFINE_API_FN(int, YOGI_BranchCreate,
                    (void** branch, void* context, const char* props,
                     const char* section, char* err, int errsize))

_YOGI_DEFINE_API_FN(int, YOGI_BranchGetInfo,
                    (void* branch, void* uuid, char* json, int jsonsize))

_YOGI_DEFINE_API_FN(int, YOGI_BranchGetConnectedBranches,
                    (void* branch, void* uuid, char* json, int jsonsize,
                     void (*fn)(int res, void* userarg), void* userarg))

_YOGI_DEFINE_API_FN(int, YOGI_BranchAwaitEventAsync,
                    (void* branch, int events, void* uuid, char* json,
                     int jsonsize,
                     void (*fn)(int res, int event, int ev_res, void* userarg),
                     void* userarg))

_YOGI_DEFINE_API_FN(int, YOGI_BranchCancelAwaitEvent, (void* branch))

_YOGI_DEFINE_API_FN(int, YOGI_BranchSendBroadcast,
                    (void* branch, int enc, const void* data, int datasize,
                     int block))

_YOGI_DEFINE_API_FN(int, YOGI_BranchSendBroadcastAsync,
                    (void* branch, int enc, const void* data, int datasize,
                     int retry, void (*fn)(int res, int oid, void* userarg),
                     void* userarg))

_YOGI_DEFINE_API_FN(int, YOGI_BranchCancelSendBroadcast,
                    (void* branch, int oid))

_YOGI_DEFINE_API_FN(int, YOGI_BranchReceiveBroadcastAsync,
                    (void* branch, void* uuid, int enc, void* data,
                     int datasize, void (*fn)(int res, int size, void* userarg),
                     void* userarg))

_YOGI_DEFINE_API_FN(int, YOGI_BranchCancelReceiveBroadcast, (void* branch))

/// \addtogroup enums
/// @{

////////////////////////////////////////////////////////////////////////////////
/// %Branch events.
///
/// %Branch events are events that can be observed on a branch.
////////////////////////////////////////////////////////////////////////////////
enum class BranchEvents {
  /// No events.
  kNone = 0,

  /// A new branch has been discovered.
  kBranchDiscovered = (1 << 0),

  /// Querying a new branch for information finished.
  kBranchQueried = (1 << 1),

  /// Connecting to a branch finished.
  kConnectFinished = (1 << 2),

  /// The connection to a branch was lost.
  kConnectionLost = (1 << 3),

  /// Combination of all flags.
  kAll =
      kBranchDiscovered | kBranchQueried | kConnectFinished | kConnectionLost,
};

_YOGI_DEFINE_FLAG_OPERATORS(BranchEvents)

template <>
inline std::string ToString<BranchEvents>(const BranchEvents& events) {
  switch (events) {
    _YOGI_TO_STRING_ENUM_CASE(BranchEvents, kNone)
    _YOGI_TO_STRING_ENUM_CASE(BranchEvents, kBranchDiscovered)
    _YOGI_TO_STRING_ENUM_CASE(BranchEvents, kBranchQueried)
    _YOGI_TO_STRING_ENUM_CASE(BranchEvents, kConnectFinished)
    _YOGI_TO_STRING_ENUM_CASE(BranchEvents, kConnectionLost)
    _YOGI_TO_STRING_ENUM_CASE(BranchEvents, kAll)
  }

  std::string s;
  _YOGI_TO_STRING_FLAG_APPENDER(events, BranchEvents, kBranchDiscovered)
  _YOGI_TO_STRING_FLAG_APPENDER(events, BranchEvents, kBranchQueried)
  _YOGI_TO_STRING_FLAG_APPENDER(events, BranchEvents, kConnectFinished)
  _YOGI_TO_STRING_FLAG_APPENDER(events, BranchEvents, kConnectionLost)
  return s.substr(3);
}

/// @} enums

////////////////////////////////////////////////////////////////////////////////
/// Information about about a branch.
////////////////////////////////////////////////////////////////////////////////
class BranchInfo {
 public:
  virtual ~BranchInfo() {}

  /// Returns the UUID of the branch.
  ///
  /// \returns UUID of the branch.
  const Uuid& GetUuid() const { return uuid_; }

  /// Returns the name of the branch.
  ///
  /// \returns The name of the branch.
  std::string GetName() const { return json_["name"]; }

  /// Returns the description of the branch.
  ///
  /// \returns The description of the branch.
  std::string GetDescription() const { return json_["description"]; }

  /// Returns the name of the network.
  ///
  /// \returns The name of the network.
  std::string GetNetworkName() const { return json_["network_name"]; }

  /// Returns the  path of the branch.
  ///
  /// \returns The  path of the branch.
  std::string GetPath() const { return json_["path"]; }

  /// Returns the machine's hostname.
  ///
  /// \returns The machine's hostname.
  std::string GetHostname() const { return json_["hostname"]; }

  /// Returns the ID of the process.
  ///
  /// \returns The ID of the process.
  int GetPid() const { return json_["pid"]; }

  /// Returns the advertising interval.
  ///
  /// \returns The advertising interval.
  Duration GetAdvertisingInterval() const {
    float val = json_["advertising_interval"];
    return val < 0 ? Duration::kInfinity : Duration::FromSeconds(val);
  }

  /// Returns the address of the TCP server for incoming connections.
  ///
  /// \returns The address of the TCP server for incoming connections.
  std::string GetTcpServerAddress() const {
    return json_["tcp_server_address"];
  }

  /// Returns the listening port of the TCP server for incoming connections.
  ///
  /// \returns The listening port of the TCP server for incoming connections.
  int GetTcpServerPort() const { return json_["tcp_server_port"]; }

  /// Returns the time when the branch was started (UTC time).
  ///
  /// \returns The time when the branch was started (UTC time).
  Timestamp GetStartTime() const {
    return Timestamp::Parse(
        static_cast<const std::string&>(json_["start_time"]));
  }

  /// Returns the connection timeout.
  ///
  /// \returns The connection timeout.
  Duration GetTimeout() const {
    float val = json_["timeout"];
    return val < 0 ? Duration::kInfinity : Duration::FromSeconds(val);
  }

  /// Returns true if the branch is in ghost mode.
  ///
  /// \return True if the branch is in ghost mode.
  bool GetGhostMode() const { return json_["ghost_mode"]; }

  /// Returns the branch information as JSON-encoded string.
  ///
  /// \returns Branch information as JSON-encoded string.
  const std::string& ToString() const { return json_str_; }

  /// Returns the branch information as a JSON object.
  ///
  /// \return Branch information as a JSON object.
  const Json& ToJson() const { return json_; }

 protected:
  BranchInfo(const Uuid& uuid, std::string&& json_str)
      : BranchInfo(uuid, Json::parse(json_str), json_str) {}

  BranchInfo(const Uuid& uuid, Json json, std::string json_str)
      : uuid_(uuid), json_(json), json_str_(json_str) {}

 private:
  const Uuid uuid_;
  const Json json_;
  const std::string json_str_;
};

////////////////////////////////////////////////////////////////////////////////
/// Information about a remote branch.
////////////////////////////////////////////////////////////////////////////////
class RemoteBranchInfo : public BranchInfo {
  friend class BranchQueriedEventInfo;
  friend class Branch;

 protected:
  using BranchInfo::BranchInfo;
};

////////////////////////////////////////////////////////////////////////////////
/// Information about a local branch.
////////////////////////////////////////////////////////////////////////////////
class LocalBranchInfo : public BranchInfo {
  friend class Branch;

 public:
  /// Advertising IP address.
  ///
  /// \returns The advertising IP address.
  std::string GetAdvertisingAddress() const {
    return ToJson()["advertising_address"];
  }

  /// Advertising port.
  ///
  /// \returns The advertising port.
  int GetAdvertisingPort() const { return ToJson()["advertising_port"]; }

  /// Size of the send queues for remote branches.
  ///
  /// \returns The size of the send queues.
  int GetTxQueueSize() const { return ToJson()["tx_queue_size"]; }

  /// Size of the receive queues for remote branches.
  ///
  /// \returns The size of the receive queues.
  int GetRxQueueSize() const { return ToJson()["rx_queue_size"]; }

 protected:
  LocalBranchInfo(const Uuid& uuid, std::string&& json_str)
      : BranchInfo(uuid, std::move(json_str)) {}
};

////////////////////////////////////////////////////////////////////////////////
/// Information associated with a branch event.
////////////////////////////////////////////////////////////////////////////////
class BranchEventInfo {
  friend class Branch;

 public:
  virtual ~BranchEventInfo() {}

  /// Returns the UUID of the branch.
  ///
  /// \returns UUID of the branch.
  const Uuid& GetUuid() const { return uuid_; }

  /// Returns the event information as JSON-encoded string.
  ///
  /// \returns Event information as JSON-encoded string.
  const std::string& ToString() const { return json_str_; }

  /// Returns the event information as a JSON object.
  ///
  /// \return Event information as a JSON object.
  const Json& ToJson() const { return json_; }

 protected:
  BranchEventInfo() : BranchEventInfo({}, "{}") {}

  BranchEventInfo(const Uuid& uuid, std::string&& json_str)
      : uuid_(uuid),
        json_(Json::parse(json_str)),
        json_str_(std::move(json_str)) {}

 private:
  const Uuid uuid_;
  const Json json_;
  const std::string json_str_;
};

////////////////////////////////////////////////////////////////////////////////
/// Information associated with the kBranchDiscovered event.
////////////////////////////////////////////////////////////////////////////////
class BranchDiscoveredEventInfo : public BranchEventInfo {
  friend class Branch;

 public:
  /// Returns the address of the TCP server for incoming connections.
  ///
  /// \returns The address of the TCP server for incoming connections.
  std::string GetTcpServerAddress() const {
    return ToJson()["tcp_server_address"];
  }

  /// Returns the listening port of the TCP server for incoming connections.
  ///
  /// \returns The listening port of the TCP server for incoming connections.
  int GetTcpServerPort() const { return ToJson()["tcp_server_port"]; }

 protected:
  BranchDiscoveredEventInfo(const Uuid& uuid, std::string&& json_str)
      : BranchEventInfo(uuid, std::move(json_str)) {}
};

////////////////////////////////////////////////////////////////////////////////
/// Information associated with the kBranchQueried event.
////////////////////////////////////////////////////////////////////////////////
class BranchQueriedEventInfo : public BranchEventInfo {
  friend class Branch;

 public:
  /// Returns the name of the branch.
  ///
  /// \returns The name of the branch.
  std::string GetName() const { return ToJson()["name"]; }

  /// Returns the description of the branch.
  ///
  /// \returns The description of the branch.
  std::string GetDescription() const { return ToJson()["description"]; }

  /// Returns the name of the network.
  ///
  /// \returns The name of the network.
  std::string GetNetworkName() const { return ToJson()["network_name"]; }

  /// Returns the  path of the branch.
  ///
  /// \returns The  path of the branch.
  std::string GetPath() const { return ToJson()["path"]; }

  /// Returns the machine's hostname.
  ///
  /// \returns The machine's hostname.
  std::string GetHostname() const { return ToJson()["hostname"]; }

  /// Returns the ID of the process.
  ///
  /// \returns The ID of the process.
  int GetPid() const { return ToJson()["pid"]; }

  /// Returns the advertising interval.
  ///
  /// \returns The advertising interval.
  Duration GetAdvertisingInterval() const {
    float val = ToJson()["advertising_interval"];
    return val < 0 ? Duration::kInfinity : Duration::FromSeconds(val);
  }

  /// Returns the address of the TCP server for incoming connections.
  ///
  /// \returns The address of the TCP server for incoming connections.
  std::string GetTcpServerAddress() const {
    return ToJson()["tcp_server_address"];
  }

  /// Returns the listening port of the TCP server for incoming connections.
  ///
  /// \returns The listening port of the TCP server for incoming connections.
  int GetTcpServerPort() const { return ToJson()["tcp_server_port"]; }

  /// Returns the time when the branch was started (UTC time).
  ///
  /// \returns The time when the branch was started (UTC time).
  Timestamp GetStartTime() const {
    return Timestamp::Parse(
        static_cast<const std::string&>(ToJson()["start_time"]));
  }

  /// Returns the connection timeout.
  ///
  /// \returns The connection timeout.
  Duration GetTimeout() const {
    float val = ToJson()["timeout"];
    return val < 0 ? Duration::kInfinity : Duration::FromSeconds(val);
  }

  /// Returns true if the branch is in ghost mode.
  ///
  /// \return True if the branch is in ghost mode.
  bool GetGhostMode() const { return ToJson()["ghost_mode"]; }

  /// Converts the event information to a RemoteBranchInfo object.
  ///
  /// \returns The converted RemoteBranchInfo object.
  RemoteBranchInfo ToRemoteBranchInfo() const {
    return RemoteBranchInfo(GetUuid(), ToJson(), ToString());
  }

 protected:
  BranchQueriedEventInfo(const Uuid& uuid, std::string&& json_str)
      : BranchEventInfo(uuid, std::move(json_str)) {}
};

////////////////////////////////////////////////////////////////////////////////
/// Information associated with the kConnectFinished event.
////////////////////////////////////////////////////////////////////////////////
class ConnectFinishedEventInfo : public BranchEventInfo {
  friend class Branch;

 protected:
  using BranchEventInfo::BranchEventInfo;
};

////////////////////////////////////////////////////////////////////////////////
/// Information associated with the kConnectionLost event.
////////////////////////////////////////////////////////////////////////////////
class ConnectionLostEventInfo : public BranchEventInfo {
  friend class Branch;

 protected:
  using BranchEventInfo::BranchEventInfo;
};

class Branch;

/// Shared pointer to a branch.
using BranchPtr = std::shared_ptr<Branch>;

////////////////////////////////////////////////////////////////////////////////
/// Entry point into a Yogi network.
///
/// A branch represents an entry point into a Yogi network. It advertises
/// itself via IP broadcasts/multicasts with its unique ID and information
/// required for establishing a connection. If a branch detects other branches
/// on the network, it connects to them via TCP to retrieve further
/// information such as their name, description and network name. If the
/// network names match, two branches attempt to authenticate with each other
/// by securely comparing passwords. Once authentication succeeds and there is
/// no other known branch with the same path then the branches can actively
/// communicate as part of the Yogi network.
///
/// \note
///   Even if the communication between branches is not encrypted, the
///   authentication process via passwords is always done in a secure manner.
////////////////////////////////////////////////////////////////////////////////
class Branch : public ObjectT<Branch> {
 public:
  /// Callback function used in AwaitEventAsync().
  ///
  /// \param res    %Result of the wait operation.
  /// \param event  The branch event that occurred.
  /// \param ev_res %Result associated with the event.
  /// \param info   Event information.
  using AwaitEventFn =
      std::function<void(const Result& res, BranchEvents event,
                         const Result& ev_res, BranchEventInfo& info)>;

  /// Callback function used in SendBroadcastAsync().
  ///
  /// \param res %Result of the send operation.
  /// \param oid ID of the send operation.
  using SendBroadcastFn =
      std::function<void(const Result& res, OperationId oid)>;

  /// Callback function used in ReceiveBroadcastAsync().
  ///
  /// \param res     %Result of the receive operation.
  /// \param source  UUID of the sending branch.
  /// \param payload View on the received payload.
  /// \param buffer  Buffer holding the payload.
  using ReceiveBroadcastFn =
      std::function<void(const Result& res, const Uuid& source,
                         const PayloadView& payload, BufferPtr&& buffer)>;

  /// Callback function used in ReceiveBroadcastAsync().
  ///
  /// As opposed to ReceiveBroadcastFn, this function does not include the
  /// buffer parameter.
  ///
  /// \param res     %Result of the receive operation.
  /// \param source  UUID of the sending branch.
  /// \param payload View on the received payload.
  using ReceiveBroadcastSimpleFn = std::function<void(
      const Result& res, const Uuid& source, const PayloadView& payload)>;

  /// Creates a branch.
  ///
  /// The branch is configured via the \p props parameter. The supplied JSON
  /// must have the following structure:
  ///
  /// \code
  ///   {
  ///     "name":                 "Fan Controller",
  ///     "description":          "Controls a fan via PWM",
  ///     "path":                 "/Cooling System/Fan Controller",
  ///     "network_name":         "Hardware Control",
  ///     "network_password":     "secret",
  ///     "advertising_interfaces": ["localhost"],
  ///     "advertising_address":  "ff02::8000:2439",
  ///     "advertising_port":     13531,
  ///     "advertising_interval": 1.0,
  ///     "timeout":              3.0,
  ///     "ghost_mode":           false,
  ///     "tx_queue_size":        1000000,
  ///     "rx_queue_size":        100000
  ///   }
  /// \endcode
  ///
  /// All of the properties are optional and if unspecified (or set to _null_),
  /// their respective default values will be used (see \ref constants). The
  /// properties have the following meaning:
  ///  - __name__: Name of the branch (default: PID\@hostname without the
  ///    backslash).
  ///  - __description__: Description of the branch.
  ///  - __path__: Path of the branch in the network (default: /name where name
  ///    is the name of the branch). Must start with a slash.
  ///  - __network_name__: Name of the network to join (default: the machine's
  ///    hostname).
  ///  - __network_password__: Password for the network (default: no password).
  ///  - __advertising_interfaces__: Network interfaces to use for advertising
  ///    and for branch connections. Valid strings are Unix device names
  ///    ("eth0", "en5", "wlan0"), adapter names on Windows ("Ethernet",
  ///     "VMware Network Adapter WMnet1") or MAC addresses
  ///     ("11:22:33:44:55:66"). Furthermore, the special strings "localhost"
  ///     and "all" can be used to denote loopback and all available interfaces
  ///     respectively.
  ///  - __advertising_address__: Multicast address to use for advertising, e.g.
  ///    239.255.0.1 for IPv4 or ff02::8000:1234 for IPv6.
  ///  - __advertising_port__: Port to use for advertising.
  ///  - __advertising_interval__: Time between advertising messages. Must be at
  ///    least 1 ms.
  ///  - __ghost_mode__: Set to true to activate ghost mode (default: false).
  ///  - __tx_queue_size__: Size of the send queues for remote branches.
  ///  - __rx_queue_size__: Size of the receive queues for remote branches.
  ///
  /// Advertising and establishing connections can be limited to certain network
  /// interfaces via the _interface_ property. The default is to use all
  /// available interfaces.
  ///
  /// Setting the _ghost_mode_ property to _true_ prevents the branch from
  /// actively participating in the Yogi network, i.e. the branch will not
  /// advertise itself and it will not authenticate in order to join a network.
  /// However, the branch will temporarily connect to other branches in order to
  /// obtain more detailed information such as name, description, network name
  /// and so on. This is useful for obtaining information about active branches
  /// without actually becoming part of the Yogi network.
  ///
  /// \attention
  ///   The _tx_queue_size_ and _rx_queue_size_ properties affect every branch
  ///   connection and can therefore consume a large amount of memory. For
  ///   example, in a network of 10 branches where these properties are set to
  ///   1 MB, the resulting memory used for the queues would be
  ///   10 x 2 x 1 MB = 20 MB for each of the 10 branches. This value grows with
  ///   the number of branches squared.
  ///
  /// \param context %Context to use
  /// \param props   %Branch properties as serialized JSON
  /// \param section Section in \p props to use instead of the root section;
  ///                syntax is JSON pointer (RFC 6901)
  ///
  /// \returns The created branch.
  static BranchPtr Create(ContextPtr context, const JsonView& props = {},
                          const StringView& section = {}) {
    return BranchPtr(new Branch(context, props, section));
  }

  /// Returns information about the local branch.
  ///
  /// \returns Information about the local branch.
  const LocalBranchInfo& GetInfo() const { return info_; }

  /// Returns the UUID of the branch.
  ///
  /// \returns UUID of the branch.
  const Uuid& GetUuid() const { return info_.GetUuid(); }

  /// Returns the name of the branch.
  ///
  /// \returns The name of the branch.
  std::string GetName() const { return info_.GetName(); }

  /// Returns the description of the branch.
  ///
  /// \returns The description of the branch.
  std::string GetDescription() const { return info_.GetDescription(); }

  /// Returns the name of the network.
  ///
  /// \returns The name of the network.
  std::string GetNetworkName() const { return info_.GetNetworkName(); }

  /// Returns the  path of the branch.
  ///
  /// \returns The  path of the branch.
  std::string GetPath() const { return info_.GetPath(); }

  /// Returns the machine's hostname.
  ///
  /// \returns The machine's hostname.
  std::string GetHostname() const { return info_.GetHostname(); }

  /// Returns the ID of the process.
  ///
  /// \returns The ID of the process.
  int GetPid() const { return info_.GetPid(); }

  /// Returns the advertising interval.
  ///
  /// \returns The advertising interval.
  Duration GetAdvertisingInterval() const {
    return info_.GetAdvertisingInterval();
  }

  /// Returns the address of the TCP server for incoming connections.
  ///
  /// \returns The address of the TCP server for incoming connections.
  std::string GetTcpServerAddress() const {
    return info_.GetTcpServerAddress();
  }

  /// Returns the listening port of the TCP server for incoming connections.
  ///
  /// \returns The listening port of the TCP server for incoming connections.
  int GetTcpServerPort() const { return info_.GetTcpServerPort(); }

  /// Returns the time when the branch was started (UTC time).
  ///
  /// \returns The time when the branch was started (UTC time).
  Timestamp GetStartTime() const { return info_.GetStartTime(); }

  /// Returns the connection timeout.
  ///
  /// \returns The connection timeout.
  Duration GetTimeout() const { return info_.GetTimeout(); }

  /// Returns true if the branch is in ghost mode.
  ///
  /// \return True if the branch is in ghost mode.
  bool GetGhostMode() const { return info_.GetGhostMode(); }

  /// Advertising IP address.
  ///
  /// \returns The advertising IP address.
  std::string GetAdvertisingAddress() const {
    return info_.GetAdvertisingAddress();
  }

  /// Advertising port.
  ///
  /// \returns The advertising port.
  int GetAdvertisingPort() const { return info_.GetAdvertisingPort(); }

  /// Size of the send queues for remote branches.
  ///
  /// \returns The size of the send queues.
  int GetTxQueueSize() const { return info_.GetTxQueueSize(); }

  /// Size of the receive queues for remote branches.
  ///
  /// \returns The size of the receive queues.
  int GetRxQueueSize() const { return info_.GetRxQueueSize(); }

  /// Retrieves information about all connected remote branches.
  ///
  /// \returns A map mapping the UUID of each connected remote branch to a
  ///          RemoteBranchInfo object with detailed information about the
  ///          branch.
  std::unordered_map<Uuid, RemoteBranchInfo> GetConnectedBranches() const {
    struct CallbackData {
      Uuid uuid;
      const char* str;
      std::unordered_map<Uuid, RemoteBranchInfo> branches;
    } data;

    internal::QueryString([&](auto str, auto size) {
      data.str = str;
      data.branches.clear();

      return internal::YOGI_BranchGetConnectedBranches(
          this->GetHandle(), &data.uuid, str, size,
          [](int res, void* userarg) {
            if (res == static_cast<int>(ErrorCode::kOk)) {
              auto data = static_cast<CallbackData*>(userarg);
              data->branches.emplace(std::make_pair(
                  data->uuid, RemoteBranchInfo(data->uuid, data->str)));
            }
          },
          &data);
    });

    return data.branches;
  }

  /// Waits for a branch event to occur.
  ///
  /// This function will register the handler fn to be executed once one of the
  /// given branch events occurs. If this function is called while a previous
  /// wait operation is still active then the previous opertion will be
  /// canceled, i.e. the handler \p fn for the previous operation will be called
  /// with a cancellation error.
  ///
  /// If successful, the event information passed to the handler function \p fn
  /// contains at least the UUID of the remote branch.
  ///
  /// In case that the internal buffer for reading the event information is too
  /// small, fn will be called with the corresponding error and the event
  /// information is lost. You can set the size of this buffer via the
  /// \p buffer_size parameter.
  ///
  /// \param events      Events to observe.
  /// \param fn          Handler function to call.
  /// \param buffer_size Size of the internal buffer for reading event
  ///                    information.
  void AwaitEventAsync(BranchEvents events, AwaitEventFn fn,
                       int buffer_size = 1024) {
    struct CallbackData {
      AwaitEventFn fn;
      Uuid uuid;
      std::vector<char> json;
    };

    auto data = std::make_unique<CallbackData>();
    data->fn = fn;
    data->json.resize(static_cast<std::size_t>(buffer_size));

    int res = internal::YOGI_BranchAwaitEventAsync(
        GetHandle(), static_cast<int>(events), &data->uuid, data->json.data(),
        buffer_size,
        [](int res, int event, int ev_res, void* userarg) {
          auto data = std::unique_ptr<CallbackData>(
              static_cast<CallbackData*>(userarg));
          if (!data->fn) return;

          auto be = static_cast<BranchEvents>(event);

          if (Result(res) && be != BranchEvents::kNone) {
            switch (be) {
              case BranchEvents::kBranchDiscovered:
                CallAwaitEventFn<BranchDiscoveredEventInfo>(res, be, ev_res,
                                                            data);
                break;

              case BranchEvents::kBranchQueried:
                CallAwaitEventFn<BranchQueriedEventInfo>(res, be, ev_res, data);
                break;

              case BranchEvents::kConnectFinished:
                CallAwaitEventFn<ConnectFinishedEventInfo>(res, be, ev_res,
                                                           data);
                break;

              case BranchEvents::kConnectionLost:
                CallAwaitEventFn<ConnectionLostEventInfo>(res, be, ev_res,
                                                          data);
                break;

              default: {
                bool should_never_get_here = false;
                assert(should_never_get_here);
              }
            }
          } else {
            CallAwaitEventFn<BranchEventInfo>(res, be, ev_res, data);
          }
        },
        data.get());

    internal::CheckErrorCode(res);
    data.release();
  }

  /// Cancels waiting for a branch event.
  ///
  /// Calling this function will cause the handler registered via
  /// AwaitEventAsync() to be called with a cancellation error.
  ///
  /// \return True if the wait operation was cancelled successfully.
  bool CancelAwaitEvent() {
    int res = internal::YOGI_BranchCancelAwaitEvent(GetHandle());
    return internal::FalseIfSpecificErrorElseThrow(
        res, ErrorCode::kOperationNotRunning);
  }

  /// Sends a broadcast message to all connected branches.
  ///
  /// Broadcast messages contain arbitrary data encoded as JSON or MessagePack.
  /// As opposed to sending messages via terminals, broadcast messages don't
  /// have to comply with a defined schema for the payload; any data that can be
  /// encoded is valid. This implies that validating the data is entirely up to
  /// the user code.
  ///
  /// Setting the \p block parameter to _false_ will cause the function to
  /// skip sending the message to branches that have a full send queue. If at
  /// least one branch was skipped, the function will return _false_. If the
  /// parameter is set to _true_ instead, the function will block until the
  /// message has been put into the send queues of all connected branches.
  ///
  /// \attention
  ///   Calling this function from within a handler function executed through
  ///   the branch's _context_  with \p block set to _true_ will cause a
  ///   dead-lock if any send queue is full!
  ///
  /// \param payload Payload to send.
  /// \param block   Block until message has been put into all send buffers.
  ///
  /// \return _true_ if the message was successfully put into all send buffers.
  bool SendBroadcast(const PayloadView& payload, bool block = true) {
    int res = internal::YOGI_BranchSendBroadcast(
        GetHandle(), static_cast<int>(payload.Encoding()), payload.Data(),
        payload.Size(), block ? 1 : 0);

    if (res == static_cast<int>(ErrorCode::kTxQueueFull)) {
      return false;
    } else {
      internal::CheckErrorCode(res);
      return true;
    }
  }

  /// Sends a broadcast message to all connected branches.
  ///
  /// Broadcast messages contain arbitrary data encoded as JSON or MessagePack.
  /// As opposed to sending messages via terminals, broadcast messages don't
  /// have to comply with a defined schema for the payload; any data that can be
  /// encoded is valid. This implies that validating the data is entirely up to
  /// the user code.
  ///
  /// Setting the \p retry parameter to false will cause the function to skip
  /// sending the message to branches that have a full send queue. If at least
  /// one branch was skipped, the handler \p fn will be called with the
  /// #kTxQueueFull error. If the parameter is set to true instead, \p fn will
  /// be called once the message has been put into the send queues of all
  /// connected branches.
  ///
  /// The function returns an ID which uniquely identifies this send operation
  /// until \p fn has been called. It can be used in a subsequent
  /// CancelSendBroadcast() call to abort the operation.
  ///
  /// \note
  ///   The payload will be copied if necessary, i.e. \p payload only needs to
  ///   remain valid until the function returns.
  ///
  /// \param payload Payload to send.
  /// \param retry   Retry sending the message if a send queue is full.
  /// \param fn      Handler to call once the operation finishes.
  ///
  /// \return ID of the send operation.
  OperationId SendBroadcastAsync(const PayloadView& payload, bool retry,
                                 SendBroadcastFn fn) {
    struct CallbackData {
      SendBroadcastFn fn;
    };

    auto data = std::make_unique<CallbackData>();
    data->fn = fn;

    int res = internal::YOGI_BranchSendBroadcastAsync(
        GetHandle(), static_cast<int>(payload.Encoding()), payload.Data(),
        payload.Size(), retry ? 1 : 0,
        [](int res, int oid, void* userarg) {
          auto data = std::unique_ptr<CallbackData>(
              static_cast<CallbackData*>(userarg));
          if (!data->fn) return;

          internal::WithErrorCodeToResult(res, data->fn,
                                          internal::MakeOperationId(oid));
        },
        data.get());

    internal::CheckErrorCode(res);
    data.release();

    return internal::MakeOperationId(res);
  }

  /// Sends a broadcast message to all connected branches.
  ///
  /// Broadcast messages contain arbitrary data encoded as JSON or MessagePack.
  /// As opposed to sending messages via terminals, broadcast messages don't
  /// have to comply with a defined schema for the payload; any data that can be
  /// encoded is valid. This implies that validating the data is entirely up to
  /// the user code.
  ///
  /// The handler function \p fn will be called once the message has been put
  /// into the send queues of all connected branches.
  ///
  /// The function returns an ID which uniquely identifies this send operation
  /// until \p fn has been called. It can be used in a subsequent
  /// CancelSendBroadcast() call to abort the operation.
  ///
  /// \note
  ///   The payload will be copied if necessary, i.e. \p payload only needs to
  ///   remain valid until the function returns.
  ///
  /// \param payload Payload to send.
  /// \param fn      Handler to call once the operation finishes.
  ///
  /// \return ID of the send operation.
  OperationId SendBroadcastAsync(const PayloadView& payload,
                                 SendBroadcastFn fn) {
    return SendBroadcastAsync(payload, true, fn);
  }

  /// Cancels a send broadcast operation.
  ///
  /// Calling this function will cause the send operation with the specified
  /// operation ID \p oid to be canceled, resulting in the handler function
  /// registered via the SendBroadcastAsync() call that returned the same \p oid
  /// to be called with the #kCanceled error.
  ///
  /// \note
  ///   If the send operation has already been carried out but the handler
  ///   function has not been called yet, then cancelling the operation will
  ///   fail and _false_ will be returned.
  ///
  /// \param oid ID of the send operation.
  ///
  /// \returns True if the operation has been canceled successfully.
  bool CancelSendBroadcast(OperationId oid) {
    int res =
        internal::YOGI_BranchCancelSendBroadcast(GetHandle(), oid.Value());
    return internal::FalseIfSpecificErrorElseThrow(
        res, ErrorCode::kInvalidOperationId);
  }

  /// Receives a broadcast message from any of the connected branches.
  ///
  /// Broadcast messages contain arbitrary data encoded as JSON or MessagePack.
  /// As opposed to sending messages via terminals, broadcast messages don't
  /// have to comply with a defined schema for the payload; any data that can be
  /// encoded is valid. This implies that validating the data is entirely up to
  /// the user code.
  ///
  /// This function will register \p fn to be called once a broadcast message
  /// has been received. The payload will be encoded as MessagePack.
  ///
  /// \note
  ///   This function internally allocates a buffer large enough to hold the
  ///   largest allowed payload. Use a different overload if you need to
  ///   fine-tune the behaviour.
  ///
  /// If this function is called while a previous receive operation is still
  /// active then the previous operation will be canceled with the
  /// #kCanceled error.
  ///
  /// \attention
  ///   Broadcast messages do not get queued, i.e. if a branch is not actively
  ///   receiving broadcast messages then they will be discarded. To ensure that
  ///   no messages get missed, call ReceiveBroadcastAsync() again from within
  ///   the handler \p fn.
  ///
  /// \param fn Handler to call for the received broadcast message.
  void ReceiveBroadcastAsync(ReceiveBroadcastFn fn) {
    auto buffer = std::make_unique<Buffer>(constants::kMaxMessagePayloadSize);
    ReceiveBroadcastAsync(std::move(buffer), fn);
  }

  /// Receives a broadcast message from any of the connected branches.
  ///
  /// Broadcast messages contain arbitrary data encoded as JSON or MessagePack.
  /// As opposed to sending messages via terminals, broadcast messages don't
  /// have to comply with a defined schema for the payload; any data that can be
  /// encoded is valid. This implies that validating the data is entirely up to
  /// the user code.
  ///
  /// This function will register \p fn to be called once a broadcast message
  /// has been received. The payload will be encoded as per \p enc.
  ///
  /// \note
  ///   This function internally allocates a buffer large enough to hold the
  ///   largest allowed payload. Use a different overload if you need to
  ///   fine-tune the behaviour.
  ///
  /// If this function is called while a previous receive operation is still
  /// active then the previous operation will be canceled with the
  /// #kCanceled error.
  ///
  /// \attention
  ///   Broadcast messages do not get queued, i.e. if a branch is not actively
  ///   receiving broadcast messages then they will be discarded. To ensure that
  ///   no messages get missed, call ReceiveBroadcastAsync() again from within
  ///   the handler \p fn.
  ///
  /// \param enc Encoding to use for the received payload.
  /// \param fn  Handler to call for the received broadcast message.
  void ReceiveBroadcastAsync(EncodingType enc, ReceiveBroadcastFn fn) {
    auto buffer = std::make_unique<Buffer>(constants::kMaxMessagePayloadSize);
    ReceiveBroadcastAsync(enc, std::move(buffer), fn);
  }

  /// Receives a broadcast message from any of the connected branches.
  ///
  /// Broadcast messages contain arbitrary data encoded as JSON or MessagePack.
  /// As opposed to sending messages via terminals, broadcast messages don't
  /// have to comply with a defined schema for the payload; any data that can be
  /// encoded is valid. This implies that validating the data is entirely up to
  /// the user code.
  ///
  /// This function will register \p fn to be called once a broadcast message
  /// has been received. The payload will be encoded as MessagePack.
  ///
  /// \attention
  ///   If the received payload does not fit into \p buffer then \p fn will be
  ///   called with the #kBufferTooSmall error and \p buffer containing as much
  ///   received data as possible. In this case, the payload view passed to
  ///   \p fn will be invalid.
  ///
  /// If this function is called while a previous receive operation is still
  /// active then the previous operation will be canceled with the
  /// #kCanceled error.
  ///
  /// \attention
  ///   Broadcast messages do not get queued, i.e. if a branch is not actively
  ///   receiving broadcast messages then they will be discarded. To ensure that
  ///   no messages get missed, call ReceiveBroadcastAsync() again from within
  ///   the handler \p fn.
  ///
  /// \param buffer Buffer to use for receiving the payload.
  /// \param fn     Handler to call for the received broadcast message.
  void ReceiveBroadcastAsync(BufferPtr&& buffer, ReceiveBroadcastFn fn) {
    ReceiveBroadcastAsync(EncodingType::kMsgpack, std::move(buffer), fn);
  }

  /// Receives a broadcast message from any of the connected branches.
  ///
  /// Broadcast messages contain arbitrary data encoded as JSON or MessagePack.
  /// As opposed to sending messages via terminals, broadcast messages don't
  /// have to comply with a defined schema for the payload; any data that can be
  /// encoded is valid. This implies that validating the data is entirely up to
  /// the user code.
  ///
  /// This function will register \p fn to be called once a broadcast message
  /// has been received. The payload will be encoded as per \p enc.
  ///
  /// \attention
  ///   If the received payload does not fit into \p buffer then \p fn will be
  ///   called with the #kBufferTooSmall error and \p buffer containing as much
  ///   received data as possible. In this case, the payload view passed to
  ///   \p fn will be invalid.
  ///
  /// If this function is called while a previous receive operation is still
  /// active then the previous operation will be canceled with the
  /// #kCanceled error.
  ///
  /// \attention
  ///   Broadcast messages do not get queued, i.e. if a branch is not actively
  ///   receiving broadcast messages then they will be discarded. To ensure that
  ///   no messages get missed, call ReceiveBroadcastAsync() again from within
  ///   the handler \p fn.
  ///
  /// \param enc    Encoding to use for the received payload.
  /// \param buffer Buffer to use for receiving the payload.
  /// \param fn     Handler to call for the received broadcast message.
  void ReceiveBroadcastAsync(EncodingType enc, BufferPtr&& buffer,
                             ReceiveBroadcastFn fn) {
    struct CallbackData {
      ReceiveBroadcastFn fn;
      Uuid uuid;
      BufferPtr buffer;
      EncodingType enc;
    };

    auto data = std::make_unique<CallbackData>();
    data->fn = fn;
    data->buffer = std::move(buffer);
    data->enc = enc;

    int res = internal::YOGI_BranchReceiveBroadcastAsync(
        GetHandle(), &data->uuid, static_cast<int>(enc), data->buffer->data(),
        static_cast<int>(data->buffer->size()),
        [](int res, int size, void* userarg) {
          auto data = std::unique_ptr<CallbackData>(
              static_cast<CallbackData*>(userarg));
          if (!data->fn) return;

          PayloadView payload;
          if (Result(res)) {
            payload = PayloadView(data->buffer->data(), size, data->enc);
          }

          internal::WithErrorCodeToResult(res, data->fn, data->uuid, payload,
                                          std::move(data->buffer));
        },
        data.get());

    internal::CheckErrorCode(res);
    data.release();
  }

  /// Receives a broadcast message from any of the connected branches.
  ///
  /// Broadcast messages contain arbitrary data encoded as JSON or MessagePack.
  /// As opposed to sending messages via terminals, broadcast messages don't
  /// have to comply with a defined schema for the payload; any data that can be
  /// encoded is valid. This implies that validating the data is entirely up to
  /// the user code.
  ///
  /// This function will register \p fn to be called once a broadcast message
  /// has been received. The payload will be encoded as MessagePack.
  ///
  /// \note
  ///   This function internally allocates a buffer large enough to hold the
  ///   largest allowed payload. Use a different overload if you need to
  ///   fine-tune the behaviour.
  ///
  /// If this function is called while a previous receive operation is still
  /// active then the previous operation will be canceled with the
  /// #kCanceled error.
  ///
  /// \attention
  ///   Broadcast messages do not get queued, i.e. if a branch is not actively
  ///   receiving broadcast messages then they will be discarded. To ensure that
  ///   no messages get missed, call ReceiveBroadcastAsync() again from within
  ///   the handler \p fn.
  ///
  /// \param fn Handler to call for the received broadcast message.
  void ReceiveBroadcastAsync(ReceiveBroadcastSimpleFn fn) {
    auto buffer = std::make_unique<Buffer>(constants::kMaxMessagePayloadSize);
    ReceiveBroadcastAsync(std::move(buffer), fn);
  }

  /// Receives a broadcast message from any of the connected branches.
  ///
  /// Broadcast messages contain arbitrary data encoded as JSON or MessagePack.
  /// As opposed to sending messages via terminals, broadcast messages don't
  /// have to comply with a defined schema for the payload; any data that can be
  /// encoded is valid. This implies that validating the data is entirely up to
  /// the user code.
  ///
  /// This function will register \p fn to be called once a broadcast message
  /// has been received. The payload will be encoded as per \p enc.
  ///
  /// \note
  ///   This function internally allocates a buffer large enough to hold the
  ///   largest allowed payload. Use a different overload if you need to
  ///   fine-tune the behaviour.
  ///
  /// If this function is called while a previous receive operation is still
  /// active then the previous operation will be canceled with the
  /// #kCanceled error.
  ///
  /// \attention
  ///   Broadcast messages do not get queued, i.e. if a branch is not actively
  ///   receiving broadcast messages then they will be discarded. To ensure that
  ///   no messages get missed, call ReceiveBroadcastAsync() again from within
  ///   the handler \p fn.
  ///
  /// \param enc Encoding to use for the received payload.
  /// \param fn  Handler to call for the received broadcast message.
  void ReceiveBroadcastAsync(EncodingType enc, ReceiveBroadcastSimpleFn fn) {
    auto buffer = std::make_unique<Buffer>(constants::kMaxMessagePayloadSize);
    ReceiveBroadcastAsync(enc, std::move(buffer), fn);
  }

  /// Receives a broadcast message from any of the connected branches.
  ///
  /// Broadcast messages contain arbitrary data encoded as JSON or MessagePack.
  /// As opposed to sending messages via terminals, broadcast messages don't
  /// have to comply with a defined schema for the payload; any data that can be
  /// encoded is valid. This implies that validating the data is entirely up to
  /// the user code.
  ///
  /// This function will register \p fn to be called once a broadcast message
  /// has been received. The payload will be encoded as MessagePack.
  ///
  /// \attention
  ///   If the received payload does not fit into \p buffer then \p fn will be
  ///   called with the #kBufferTooSmall error and \p buffer containing as much
  ///   received data as possible. In this case, the payload view passed to
  ///   \p fn will be invalid.
  ///
  /// If this function is called while a previous receive operation is still
  /// active then the previous operation will be canceled with the
  /// #kCanceled error.
  ///
  /// \attention
  ///   Broadcast messages do not get queued, i.e. if a branch is not actively
  ///   receiving broadcast messages then they will be discarded. To ensure that
  ///   no messages get missed, call ReceiveBroadcastAsync() again from within
  ///   the handler \p fn.
  ///
  /// \param buffer Buffer to use for receiving the payload.
  /// \param fn     Handler to call for the received broadcast message.
  void ReceiveBroadcastAsync(BufferPtr&& buffer, ReceiveBroadcastSimpleFn fn) {
    ReceiveBroadcastAsync(EncodingType::kMsgpack, std::move(buffer), fn);
  }

  /// Receives a broadcast message from any of the connected branches.
  ///
  /// Broadcast messages contain arbitrary data encoded as JSON or MessagePack.
  /// As opposed to sending messages via terminals, broadcast messages don't
  /// have to comply with a defined schema for the payload; any data that can be
  /// encoded is valid. This implies that validating the data is entirely up to
  /// the user code.
  ///
  /// This function will register \p fn to be called once a broadcast message
  /// has been received. The payload will be encoded as MessagePack.
  ///
  /// \attention
  ///   If the received payload does not fit into \p buffer then \p fn will be
  ///   called with the #kBufferTooSmall error and \p buffer containing as much
  ///   received data as possible. In this case, the payload view passed to
  ///   \p fn will be invalid.
  ///
  /// If this function is called while a previous receive operation is still
  /// active then the previous operation will be canceled with the
  /// #kCanceled error.
  ///
  /// \attention
  ///   Broadcast messages do not get queued, i.e. if a branch is not actively
  ///   receiving broadcast messages then they will be discarded. To ensure that
  ///   no messages get missed, call ReceiveBroadcastAsync() again from within
  ///   the handler \p fn.
  ///
  /// \param enc    Encoding to use for the received payload.
  /// \param buffer Buffer to use for receiving the payload.
  /// \param fn     Handler to call for the received broadcast message.
  void ReceiveBroadcastAsync(EncodingType enc, BufferPtr&& buffer,
                             ReceiveBroadcastSimpleFn fn) {
    ReceiveBroadcastAsync(enc, std::move(buffer),
                          [=](auto& res, auto& source, auto& payload, auto&&) {
                            fn(res, source, payload);
                          });
  }

  /// Cancels receiving a broadcast message.
  ///
  /// Calling this function will cause the handler registered via
  /// ReceiveBroadcastAsync() to be called with the kCanceled error.
  ///
  /// \note
  ///   If the receive handler has already been scheduled for execution,
  ///   this function will return _false_.
  ///
  /// \returns True if the operation was canceled successfully.
  bool CancelReceiveBroadcast() {
    int res = internal::YOGI_BranchCancelReceiveBroadcast(GetHandle());
    return internal::FalseIfSpecificErrorElseThrow(
        res, ErrorCode::kOperationNotRunning);
  }

 private:
  Branch(ContextPtr context, const JsonView& props, const StringView& section)
      : ObjectT(internal::CallApiCreateWithDescriptiveErrorCode(
                    internal::YOGI_BranchCreate, GetForeignHandle(context),
                    props, section),
                {context}),
        info_(QueryInfo()) {}

  LocalBranchInfo QueryInfo() {
    Uuid uuid;
    auto json = internal::QueryString([&](auto str, auto size) {
      return internal::YOGI_BranchGetInfo(this->GetHandle(), &uuid, str, size);
    });

    return LocalBranchInfo(uuid, std::move(json));
  }

  template <typename EventInfo, typename CallbackData>
  static void CallAwaitEventFn(int res, BranchEvents be, int ev_res,
                               CallbackData&& data) {
    internal::WithErrorCodeToResult(res, [&](const auto& result) {
      if (result) {
        EventInfo info(data->uuid, data->json.data());
        internal::WithErrorCodeToResult(ev_res, [&](const auto& ev_result) {
          data->fn(result, be, ev_result, info);
        });
      } else {
        BranchEventInfo info;
        internal::WithErrorCodeToResult(ev_res, [&](const auto& ev_result) {
          data->fn(result, be, ev_result, info);
        });
      }
    });
  }

  const LocalBranchInfo info_;
};

}  // namespace yogi

#endif  // YOGI_BRANCH_H
