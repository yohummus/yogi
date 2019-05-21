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

#include "../common.h"
#include "../../src/utils/system.h"
#include "../../src/utils/algorithm.h"
#include "../../src/api/constants.h"

#include <nlohmann/json.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <regex>

class BranchTest : public TestFixture {
 protected:
  virtual void SetUp() override {
    context_ = CreateContext();
    branch_ = CreateBranch(context_);
  }

  void* context_;
  void* branch_;
};

TEST_F(BranchTest, CreateWithDefaults) {
  void* branch;
  int res = YOGI_BranchCreate(&branch, context_, nullptr, nullptr, nullptr, 0);
  EXPECT_OK(res);
}

TEST_F(BranchTest, CreateWithJsonPointer) {
  nlohmann::json props;
  props["arr"] = {"some string", kBranchProps, 123};
  props["arr"][1]["name"] = "Samosa";

  char err[100];

  void* branch;
  int res = YOGI_BranchCreate(&branch, context_, MakeConfigFromJson(props),
                              "/blabla", err, sizeof(err));
  EXPECT_ERR(res, YOGI_ERR_CONFIGURATION_SECTION_NOT_FOUND);
  EXPECT_STRNE(err, "");

  res = YOGI_BranchCreate(&branch, context_, MakeConfigFromJson(props),
                          "/arr/1", err, sizeof(err));
  EXPECT_OK(res);
  EXPECT_STREQ(err, "");
  auto info = GetBranchInfo(branch);
  EXPECT_EQ(info.value("name", "NOT FOUND"), "Samosa");
}

TEST_F(BranchTest, DefaultQueueSizes) {
  void* branch;
  int res = YOGI_BranchCreate(&branch, context_, nullptr, nullptr, nullptr, 0);
  ASSERT_OK(res);
  auto info = GetBranchInfo(branch);
  EXPECT_EQ(info.value("tx_queue_size", -1), api::kDefaultTxQueueSize);
  EXPECT_EQ(info.value("rx_queue_size", -1), api::kDefaultRxQueueSize);
}

TEST_F(BranchTest, CustomQueueSizes) {
  nlohmann::json props;
  props["tx_queue_size"] = api::kMaxTxQueueSize;
  props["rx_queue_size"] = api::kMaxRxQueueSize;

  void* branch;
  int res = YOGI_BranchCreate(&branch, context_, MakeConfigFromJson(props),
                              nullptr, nullptr, 0);
  ASSERT_OK(res);
  auto info = GetBranchInfo(branch);
  EXPECT_EQ(info.value("tx_queue_size", -1), api::kMaxTxQueueSize);
  EXPECT_EQ(info.value("rx_queue_size", -1), api::kMaxRxQueueSize);
}

TEST_F(BranchTest, InvalidQueueSizes) {
  std::vector<std::pair<const char*, int>> entries = {
      {"tx_queue_size", api::kMinTxQueueSize - 1},
      {"tx_queue_size", api::kMaxTxQueueSize + 1},
      {"rx_queue_size", api::kMinTxQueueSize - 1},
      {"rx_queue_size", api::kMaxTxQueueSize + 1},
  };

  for (auto entry : entries) {
    nlohmann::json props;
    props[entry.first] = entry.second;

    char err[100];

    void* branch;
    int res = YOGI_BranchCreate(&branch, context_, MakeConfigFromJson(props),
                                nullptr, err, sizeof(err));
    EXPECT_ERR(res, YOGI_ERR_INVALID_PARAM);
    EXPECT_NE(std::string(err).find(entry.first), std::string::npos);
  }
}

TEST_F(BranchTest, GetInfoBufferTooSmall) {
  char json[3];
  int res = YOGI_BranchGetInfo(branch_, nullptr, json, sizeof(json));
  EXPECT_ERR(res, YOGI_ERR_BUFFER_TOO_SMALL);
  EXPECT_NE(json[sizeof(json) - 2], '\0');
  EXPECT_EQ(json[sizeof(json) - 1], '\0');
}

TEST_F(BranchTest, GetInfoUuid) { GetBranchUuid(branch_); }

TEST_F(BranchTest, GetInfoJson) {
  boost::uuids::uuid uuid;
  char json_str[1000] = {0};
  int res = YOGI_BranchGetInfo(branch_, &uuid, json_str, sizeof(json_str));
  EXPECT_OK(res);
  EXPECT_NE(std::count(json_str, json_str + sizeof(json_str), '\0'), 0);
  EXPECT_STRNE(json_str, "");
  auto json = nlohmann::json::parse(json_str);

  auto time_regex = std::regex(
      "^20\\d\\d-\\d\\d-\\d\\dT[0-2]\\d:[0-5]\\d:[0-5]\\d\\.\\d\\d\\dZ$");
  auto default_name =
      std::to_string(utils::GetProcessId()) + '@' + utils::GetHostname();

  EXPECT_EQ(json.value("uuid", "NOT FOUND"), boost::uuids::to_string(uuid));
  EXPECT_EQ(json.value("name", "NOT FOUND"), default_name);
  EXPECT_FALSE(json.value("description", "").empty());
  EXPECT_EQ(json.value("network_name", "NOT FOUND"), utils::GetHostname());
  EXPECT_EQ(json.value("path", "NOT FOUND"), std::string("/") + default_name);
  EXPECT_EQ(json.value("hostname", "NOT FOUND"), utils::GetHostname());
  EXPECT_EQ(json.value("pid", -1), utils::GetProcessId());
  EXPECT_EQ(json.value("tcp_server_address", "NOT FOUND"), "::");
  EXPECT_GT(json.value("tcp_server_port", 0), 1024);
  EXPECT_TRUE(
      std::regex_match(json.value("start_time", "NOT FOUND"), time_regex));
  EXPECT_EQ(json.value("timeout", -1.0f), kBranchProps["timeout"]);
  EXPECT_EQ(json.value("advertising_address", "NOT FOUND"), kAdvAddress);
  EXPECT_EQ(json.value("advertising_port", -1), static_cast<int>(kAdvPort));
  EXPECT_EQ(json.value("advertising_interval", -1.0f),
            static_cast<float>(kBranchProps["advertising_interval"]));
  EXPECT_EQ(json.value("ghost_mode", true), false);
  EXPECT_EQ(json.value("tx_queue_size", -1), api::kDefaultTxQueueSize);
  EXPECT_EQ(json.value("rx_queue_size", -1), api::kDefaultRxQueueSize);

  auto ifs = json["advertising_interfaces"];
  ASSERT_TRUE(ifs.is_array());
  ASSERT_FALSE(ifs.empty());
  EXPECT_TRUE(ifs[0]["name"].is_string());
  EXPECT_TRUE(ifs[0]["identifier"].is_string());
  EXPECT_TRUE(ifs[0]["mac"].is_string());
  EXPECT_TRUE(ifs[0]["is_loopback"].is_boolean());
  EXPECT_TRUE(!ifs[0].value("mac", "").empty() ||
              ifs[0].value("is_loopback", false));

  auto addrs = ifs[0]["addresses"];
  ASSERT_TRUE(addrs.is_array());
  ASSERT_FALSE(addrs.empty());
  EXPECT_TRUE(addrs[0].is_string());

  bool loopback_found = utils::contains_if(
      ifs, [](auto& info) { return info["is_loopback"].template get<bool>(); });
  EXPECT_TRUE(loopback_found);
}
