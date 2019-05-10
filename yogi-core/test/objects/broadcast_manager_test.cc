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

#include <chrono>
#include <atomic>
#include <thread>

class BroadcastReceiver {
 public:
  BroadcastReceiver(void* branch, int enc = YOGI_ENC_JSON)
      : branch_(branch), data_(16) {
    handler_called_ = false;

    auto res = YOGI_BranchReceiveBroadcastAsync(
        branch_, &src_uuid_, enc, data_.data(), GetBufferSize(),
        [](int res, int size, void* userarg) {
          auto self = static_cast<BroadcastReceiver*>(userarg);
          self->handler_result_ = res;
          self->data_.resize(static_cast<std::size_t>(size));
          self->handler_called_ = true;
        },
        this);
    EXPECT_OK(res);
  }

  boost::uuids::uuid GetSourceUuid() const { return src_uuid_; }
  int GetBufferSize() const { return static_cast<int>(data_.size()); }
  int GetHandlerResult() const { return handler_result_; }
  bool BroadcastReceived() const { return handler_called_; }
  std::vector<char> GetReceivedData() const { return data_; }

  template <int N>
  void CheckReceivedDataEquals(const char (&expected_data)[N]) const {
    EXPECT_EQ(data_, std::vector<char>(std::begin(expected_data),
                                       std::end(expected_data)));
  }

  void WaitForBroadcast() const {
    using namespace std::chrono_literals;
    using clock = std::chrono::steady_clock;

    auto start = clock::now();
    while (!BroadcastReceived()) {
      if (clock::now() > start + 1s) {
        throw std::runtime_error("No broadcast received within one second.");
      }
    }
  }

 private:
  void* branch_;
  boost::uuids::uuid src_uuid_;
  std::vector<char> data_;
  int handler_result_;
  std::atomic<bool> handler_called_;
};

class BroadcastManagerTest : public TestFixture {
 protected:
  BroadcastManagerTest()
      : context_(CreateContext()),
        branch_a_(CreateBranch(context_, "a")),
        branch_b_(CreateBranch(context_, "b")),
        branch_c_(
            CreateBranch(context_, "c", nullptr, nullptr, nullptr, nullptr, 5)),
        rcv_a_(branch_a_, YOGI_ENC_MSGPACK),
        rcv_b_(branch_b_),
        rcv_c_(branch_c_) {
    RunContextUntilBranchesAreConnected(context_,
                                        {branch_a_, branch_b_, branch_c_});
  }

  virtual void TearDown() {
    // To avoid potential seg faults from active receive broadcast operations
    EXPECT_EQ(YOGI_DestroyAll(), YOGI_OK);
  }

  static std::vector<char> MakeBigJsonData(std::size_t size = 10000) {
    std::vector<char> data{'[', '"', '"', ']', '\0'};
    data.insert(data.begin() + 2, size - data.size(), '.');
    return data;
  }

  void* context_;
  void* branch_a_;
  void* branch_b_;
  void* branch_c_;

  BroadcastReceiver rcv_a_;
  BroadcastReceiver rcv_b_;
  BroadcastReceiver rcv_c_;

  const char json_data_[8] = "[1,2,3]";
  const char msgpack_data_[4] = {-109, 1, 2, 3};
};

TEST_F(BroadcastManagerTest, SendJson) {
  RunContextInBackground(context_);

  int res = YOGI_BranchSendBroadcast(branch_a_, YOGI_ENC_JSON, json_data_,
                                     sizeof(json_data_), YOGI_FALSE);
  ASSERT_OK(res);

  rcv_b_.WaitForBroadcast();
  rcv_b_.CheckReceivedDataEquals(json_data_);
  rcv_c_.WaitForBroadcast();
  EXPECT_FALSE(rcv_a_.BroadcastReceived());
}

TEST_F(BroadcastManagerTest, SendMessagePack) {
  RunContextInBackground(context_);

  int res = YOGI_BranchSendBroadcast(branch_a_, YOGI_ENC_MSGPACK, msgpack_data_,
                                     sizeof(msgpack_data_), YOGI_FALSE);
  ASSERT_OK(res);

  rcv_b_.WaitForBroadcast();
  rcv_b_.CheckReceivedDataEquals(json_data_);
  rcv_c_.WaitForBroadcast();
  EXPECT_FALSE(rcv_a_.BroadcastReceived());
}

TEST_F(BroadcastManagerTest, SendBlock) {
  RunContextInBackground(context_);

  auto data = MakeBigJsonData();

  for (int i = 0; i < 3; ++i) {
    int res =
        YOGI_BranchSendBroadcast(branch_c_, YOGI_ENC_JSON, data.data(),
                                 static_cast<int>(data.size()), YOGI_TRUE);
    EXPECT_OK(res);
  }
}

TEST_F(BroadcastManagerTest, SendNoBlock) {
  RunContextInBackground(context_);

  auto data = MakeBigJsonData();
  int res;
  do {
    res = YOGI_BranchSendBroadcast(branch_c_, YOGI_ENC_JSON, data.data(),
                                   static_cast<int>(data.size()), YOGI_FALSE);
  } while (res == YOGI_OK);

  EXPECT_ERR(res, YOGI_ERR_TX_QUEUE_FULL);
}

TEST_F(BroadcastManagerTest, AsyncSendJson) {
  int oid = -1;
  int res = YOGI_BranchSendBroadcastAsync(branch_a_, YOGI_ENC_JSON, json_data_,
                                          sizeof(json_data_), YOGI_TRUE,
                                          [](int res, int oid, void* userarg) {
                                            EXPECT_OK(res);
                                            EXPECT_GT(oid, 0);
                                            *static_cast<int*>(userarg) = oid;
                                          },
                                          &oid);
  ASSERT_GT(res, 0);
  EXPECT_EQ(oid, -1);

  RunContextInBackground(context_);

  rcv_b_.WaitForBroadcast();
  rcv_b_.CheckReceivedDataEquals(json_data_);
  rcv_c_.WaitForBroadcast();
  EXPECT_FALSE(rcv_a_.BroadcastReceived());

  EXPECT_EQ(oid, res);
}

TEST_F(BroadcastManagerTest, AsyncSendMessagePack) {
  int oid = -1;
  int res =
      YOGI_BranchSendBroadcastAsync(branch_a_, YOGI_ENC_MSGPACK, msgpack_data_,
                                    sizeof(msgpack_data_), YOGI_TRUE,
                                    [](int res, int oid, void* userarg) {
                                      EXPECT_OK(res);
                                      EXPECT_GT(oid, 0);
                                      *static_cast<int*>(userarg) = oid;
                                    },
                                    &oid);
  ASSERT_GT(res, 0);
  EXPECT_EQ(oid, -1);

  RunContextInBackground(context_);

  rcv_b_.WaitForBroadcast();
  rcv_b_.CheckReceivedDataEquals(json_data_);
  rcv_c_.WaitForBroadcast();
  EXPECT_FALSE(rcv_a_.BroadcastReceived());

  EXPECT_EQ(oid, res);
}

TEST_F(BroadcastManagerTest, AsyncSendRetry) {
  auto data = MakeBigJsonData();

  const int n = 3;
  std::vector<int> errs;
  for (int i = 0; i < n; ++i) {
    int oid = YOGI_BranchSendBroadcastAsync(
        branch_c_, YOGI_ENC_JSON, data.data(), static_cast<int>(data.size()),
        YOGI_TRUE,
        [](int res, int, void* userarg) {
          static_cast<decltype(errs)*>(userarg)->push_back(res);
        },
        &errs);
    EXPECT_GT(oid, 0);
  }

  while (errs.size() != n) {
    PollContext(context_);
  }

  for (int err : errs) {
    EXPECT_EQ(err, YOGI_OK);
  }
}

TEST_F(BroadcastManagerTest, AsyncSendNoRetry) {
  auto data = MakeBigJsonData();

  int err = YOGI_OK;
  do {
    int oid = YOGI_BranchSendBroadcastAsync(
        branch_c_, YOGI_ENC_JSON, data.data(), static_cast<int>(data.size()),
        YOGI_FALSE,
        [](int res, int, void* userarg) { *static_cast<int*>(userarg) = res; },
        &err);
    EXPECT_GT(oid, 0);

    PollContextOne(context_);
  } while (err == YOGI_OK);

  EXPECT_EQ(err, YOGI_ERR_TX_QUEUE_FULL);
}

TEST_F(BroadcastManagerTest, CancelSend) {
  auto data = MakeBigJsonData();

  int oid;
  int res;
  std::map<int, int> oid_to_res;
  do {
    oid = YOGI_BranchSendBroadcastAsync(
        branch_c_, YOGI_ENC_JSON, data.data(), static_cast<int>(data.size()),
        YOGI_TRUE,
        [](int res, int oid, void* userarg) {
          (*static_cast<decltype(oid_to_res)*>(userarg))[oid] = res;
        },
        &oid_to_res);
    EXPECT_GT(oid, 0);

    oid_to_res[oid] = YOGI_ERR_UNKNOWN;

    res = YOGI_BranchCancelSendBroadcast(branch_c_, oid);
  } while (res == YOGI_ERR_INVALID_OPERATION_ID);

  ASSERT_OK(res);

  PollContext(context_);
  EXPECT_EQ(oid_to_res[oid], YOGI_ERR_CANCELED);
}

TEST_F(BroadcastManagerTest, ReceiveSourceUuid) {
  int oid = YOGI_BranchSendBroadcastAsync(branch_a_, YOGI_ENC_JSON, json_data_,
                                          sizeof(json_data_), YOGI_TRUE,
                                          [](int, int, void*) {}, nullptr);
  ASSERT_GT(oid, 0);

  while (!rcv_b_.BroadcastReceived()) PollContext(context_);

  EXPECT_EQ(rcv_b_.GetSourceUuid(), GetBranchUuid(branch_a_));
}

TEST_F(BroadcastManagerTest, ReceiveJson) {
  int oid = YOGI_BranchSendBroadcastAsync(branch_a_, YOGI_ENC_JSON, json_data_,
                                          sizeof(json_data_), YOGI_TRUE,
                                          [](int, int, void*) {}, nullptr);
  ASSERT_GT(oid, 0);

  while (!rcv_b_.BroadcastReceived()) PollContext(context_);

  EXPECT_EQ(rcv_b_.GetHandlerResult(), YOGI_OK);
  rcv_b_.CheckReceivedDataEquals(json_data_);
}

TEST_F(BroadcastManagerTest, ReceiveJsonBufferTooSmall) {
  auto data = MakeBigJsonData();
  int oid = YOGI_BranchSendBroadcastAsync(
      branch_a_, YOGI_ENC_JSON, data.data(), static_cast<int>(data.size()),
      YOGI_TRUE, [](int, int, void*) {}, nullptr);
  ASSERT_GT(oid, 0);

  while (!rcv_b_.BroadcastReceived()) PollContext(context_);
  EXPECT_ERR(rcv_b_.GetHandlerResult(), YOGI_ERR_BUFFER_TOO_SMALL);

  auto rcv_data = rcv_b_.GetReceivedData();
  EXPECT_EQ(static_cast<int>(rcv_data.size()), rcv_b_.GetBufferSize());
  EXPECT_EQ(rcv_data.back(), '\0');
}

TEST_F(BroadcastManagerTest, ReceiveMessagePack) {
  int oid = YOGI_BranchSendBroadcastAsync(branch_b_, YOGI_ENC_JSON, json_data_,
                                          sizeof(json_data_), YOGI_TRUE,
                                          [](int, int, void*) {}, nullptr);
  ASSERT_GT(oid, 0);

  while (!rcv_a_.BroadcastReceived()) PollContext(context_);

  EXPECT_EQ(rcv_a_.GetHandlerResult(), YOGI_OK);
  rcv_a_.CheckReceivedDataEquals(msgpack_data_);  // rcv_a_ receives MsgPack
}

TEST_F(BroadcastManagerTest, ReceiveMessagePackBufferTooSmall) {
  auto data = MakeBigJsonData();
  int oid = YOGI_BranchSendBroadcastAsync(
      branch_b_, YOGI_ENC_JSON, data.data(), static_cast<int>(data.size()),
      YOGI_TRUE, [](int, int, void*) {}, nullptr);
  ASSERT_GT(oid, 0);

  while (!rcv_a_.BroadcastReceived()) PollContext(context_);
  EXPECT_ERR(rcv_a_.GetHandlerResult(), YOGI_ERR_BUFFER_TOO_SMALL);

  auto rcv_data = rcv_a_.GetReceivedData();
  EXPECT_EQ(static_cast<int>(rcv_data.size()), rcv_b_.GetBufferSize());
  EXPECT_NE(rcv_data.back(), '\0');
}

TEST_F(BroadcastManagerTest, CancelReceive) {
  int res = YOGI_BranchCancelReceiveBroadcast(branch_a_);
  EXPECT_OK(res);

  PollContext(context_);
  EXPECT_TRUE(rcv_a_.BroadcastReceived());
  EXPECT_EQ(rcv_a_.GetHandlerResult(), YOGI_ERR_CANCELED);
}
