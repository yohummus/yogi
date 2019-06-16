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

#include "system.h"
#include "../api/errors.h"
#include "../utils/algorithm.h"

#include <boost/asio/ip/host_name.hpp>
#include <boost/algorithm/string.hpp>

#include <memory>
#include <cstdio>
#include <locale>
#include <codecvt>

#ifdef _WIN32
#include <process.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#endif

#ifdef __APPLE__
#include <net/if_dl.h>
#endif

#ifdef __linux__
#include <linux/if_packet.h>
#endif

namespace utils {
namespace {

void AppendIpAddress(NetworkInterfaceInfo* info, const sockaddr* sa) {
  char str[INET6_ADDRSTRLEN];
  boost::asio::ip::address addr;

  if (sa->sa_family == AF_INET) {
    auto sin = reinterpret_cast<const sockaddr_in*>(sa);
    if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str))) {
      addr = boost::asio::ip::make_address_v4(str);
    }
  } else {
    auto sin = reinterpret_cast<const sockaddr_in6*>(sa);
    if (inet_ntop(AF_INET6, &sin->sin6_addr, str, sizeof(str))) {
      auto v6addr = boost::asio::ip::make_address_v6(str);
      v6addr.scope_id(sin->sin6_scope_id);
      addr = v6addr;
    }
  }

  if (!addr.is_unspecified()) {
    info->addresses.push_back(addr);
    if (addr.is_loopback()) {
      info->is_loopback = true;
    }
  }
}

void RemoveUnneededIpv6LoopbackAddresses(NetworkInterfaceInfosVector* ifs) {
  for (auto& info : *ifs) {
    if (!info.is_loopback) continue;

    bool has_proper_v6_addr =
        utils::contains_if(info.addresses, [](auto& addr) {
          return addr.is_v6() && addr.to_v6().scope_id() != 0;
        });
    if (!has_proper_v6_addr) continue;

    utils::remove_erase_if(info.addresses, [](auto& addr) {
      return addr.is_v6() && addr.to_v6().scope_id() == 0;
    });
  }
}

}  // anonymous namespace

std::string GetHostname() {
  boost::system::error_code ec;
  auto hostname = boost::asio::ip::host_name(ec);
  if (ec) {
    hostname = "unknown hostname";
  }

  return hostname;
}

int GetProcessId() {
#ifdef _WIN32
  return _getpid();
#else
  return getpid();
#endif
}

int GetCurrentThreadId() {
#if defined(_WIN32)
  auto id = ::GetCurrentThreadId();
#elif defined(__APPLE__)
  std::uint64_t id;
  pthread_threadid_np(NULL, &id);
#else
  auto id = syscall(SYS_gettid);
#endif

  return static_cast<int>(id);
}

NetworkInterfaceInfosVector GetNetworkInterfaces() {
  NetworkInterfaceInfosVector ifs;

#ifdef _WIN32
  std::vector<char> buffer(4096);
  PIP_ADAPTER_ADDRESSES adapters;

  DWORD res;
  do {
    buffer.resize(buffer.size() * 2);
    adapters = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());
    auto n = static_cast<ULONG>(buffer.size());
    res = GetAdaptersAddresses(AF_UNSPEC,
                               GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST |
                                   GAA_FLAG_SKIP_DNS_SERVER,
                               nullptr, adapters, &n);
  } while (res == ERROR_BUFFER_OVERFLOW);

  if (res != NO_ERROR) {
    std::string s;
    LPSTR msg = nullptr;
    auto n = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, res, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPTSTR>(&msg), 0, nullptr);
    if (n) {
      s.assign(msg, msg + n);
      LocalFree(msg);
    }

    throw api::DescriptiveError(YOGI_ERR_ENUMERATE_NETWORK_INTERFACES_FAILED)
        << "GetAdaptersAddresses() failed: " << s;
  }

  for (auto adapter = adapters; adapter != nullptr; adapter = adapter->Next) {
    NetworkInterfaceInfo info;
    info.identifier = std::to_string(adapter->Ipv6IfIndex);

    std::wstring name(adapter->FriendlyName);
    info.name =
        std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(name);

    if (adapter->PhysicalAddressLength == 6) {
      auto p = adapter->PhysicalAddress;
      char mac[18];
      snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x", *p, *(p + 1),
               *(p + 2), *(p + 3), *(p + 4), *(p + 5));
      info.mac = mac;
    }

    for (auto uc_addr = adapter->FirstUnicastAddress; uc_addr != nullptr;
         uc_addr = uc_addr->Next) {
      AppendIpAddress(&info, uc_addr->Address.lpSockaddr);
    }

    if (!info.addresses.empty()) {
      ifs.push_back(info);
    }
  }
#else
  std::unique_ptr<ifaddrs, decltype(&freeifaddrs)> if_addr_list(nullptr,
                                                                &freeifaddrs);
  {
    ifaddrs* p;
    if (getifaddrs(&p) == -1) {
      char str[100] = {0};
      strerror_r(errno, str, sizeof(str));
      throw api::DescriptiveError(YOGI_ERR_ENUMERATE_NETWORK_INTERFACES_FAILED)
          << str;
    }
    if_addr_list.reset(p);

    for (auto ifa = if_addr_list.get(); ifa != nullptr; ifa = ifa->ifa_next) {
      auto info = utils::find_if(
          ifs, [&](auto& info) { return info.name == ifa->ifa_name; });

      if (info == ifs.end()) {
        info = ifs.insert(ifs.end(), NetworkInterfaceInfo{});
        info->name = ifa->ifa_name;
        info->identifier = info->name;
      }

      switch (ifa->ifa_addr->sa_family) {
#ifdef __APPLE__
        case AF_LINK: {
          auto addr = reinterpret_cast<sockaddr_dl*>(ifa->ifa_addr);
          auto p =
              reinterpret_cast<unsigned char*>(addr->sdl_data + addr->sdl_nlen);
#else
        case AF_PACKET: {
          auto addr = reinterpret_cast<sockaddr_ll*>(ifa->ifa_addr);
          auto p = addr->sll_addr;
#endif
          if (*p || (*(p + 1)) || (*(p + 2)) || (*(p + 3)) || (*(p + 4)) ||
              (*(p + 5))) {
            char mac[18];
            snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x", *p,
                     *(p + 1), *(p + 2), *(p + 3), *(p + 4), *(p + 5));
            info->mac = mac;
          }

          break;
        }

        case AF_INET:
        case AF_INET6:
          AppendIpAddress(&*info, ifa->ifa_addr);
          break;
      }
    }
  }

  RemoveUnneededIpv6LoopbackAddresses(&ifs);
#endif

  auto it = ifs.begin();
  while (it != ifs.end()) {
    if (it->mac.empty() && it->addresses.empty()) {
      it = ifs.erase(it);
    } else {
      ++it;
    }
  }

  return ifs;
}

NetworkInterfaceInfosVector GetFilteredNetworkInterfaces(
    const std::vector<std::string>& adv_if_strings, IpVersion ip_version) {
  NetworkInterfaceInfosVector ifs;
  for (auto& string : adv_if_strings) {
    for (auto& info : GetNetworkInterfaces()) {
      bool all = boost::iequals(string, "all");
      bool same_name = string == info.name;
      bool same_mac = boost::iequals(string, info.mac);
      bool both_are_localhost =
          boost::iequals(string, "localhost") && info.is_loopback;

      if (!all && !same_name && !same_mac && !both_are_localhost) continue;

      auto ifc = info;
      if (ip_version != IpVersion::kAny) {
        remove_erase_if(ifc.addresses, [&](auto& addr) {
          return addr.is_v6() != (ip_version == IpVersion::k6);
        });
      }

      if (!ifc.addresses.empty()) {
        ifs.push_back(ifc);
      }
    }
  }

  return ifs;
}

NetworkInterfaceInfosVector GetFilteredNetworkInterfaces(
    const std::vector<std::string>& adv_if_strings,
    const boost::asio::ip::udp& protocol) {
  if (protocol == boost::asio::ip::udp::v4()) {
    return GetFilteredNetworkInterfaces(adv_if_strings, IpVersion::k4);
  } else if (protocol == boost::asio::ip::udp::v6()) {
    return GetFilteredNetworkInterfaces(adv_if_strings, IpVersion::k6);
  } else {
    return GetFilteredNetworkInterfaces(adv_if_strings, IpVersion::kAny);
  }
}

}  // namespace utils
