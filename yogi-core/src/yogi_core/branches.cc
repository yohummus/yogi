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

#include "macros.h"
#include "helpers.h"
#include "../api/constants.h"
#include "../objects/branch/branch.h"

#include <nlohmann/json.hpp>
#include <string>
using namespace std::string_literals;

YOGI_API int YOGI_BranchCreate(void** branch, void* context, void* config,
                               const char* section, char* err, int errsize) {
  CHECK_PARAM(branch != nullptr);
  CHECK_PARAM(context != nullptr);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);
    auto cfg = UserSuppliedConfigToJson(config, section);

    auto brn = objects::branch::Branch::Create(ctx, cfg);
    brn->Start();

    *branch = api::ObjectRegister::Register(brn);
  }
  CATCH_DESCRIPTIVE_AND_RETURN(err, errsize);
}

YOGI_API int YOGI_BranchGetInfo(void* branch, void* uuid, char* json,
                                int jsonsize) {
  CHECK_PARAM(branch != nullptr);
  CHECK_PARAM(json == nullptr || jsonsize > 0);

  try {
    auto brn = api::ObjectRegister::Get<objects::branch::Branch>(branch);
    CopyUuidToUserBuffer(brn->GetUuid(), uuid);

    if (!CopyStringToUserBuffer(brn->MakeInfoString(), json, jsonsize)) {
      return YOGI_ERR_BUFFER_TOO_SMALL;
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_BranchGetConnectedBranches(void* branch, void* uuid,
                                             char* json, int jsonsize,
                                             void (*fn)(int, void*),
                                             void* userarg) {
  CHECK_PARAM(branch != nullptr);
  CHECK_PARAM(json == nullptr || jsonsize > 0);
  CHECK_PARAM(fn != nullptr);

  try {
    auto brn = api::ObjectRegister::Get<objects::branch::Branch>(branch);

    auto buffer_too_small = false;
    for (auto& entry : brn->MakeConnectedBranchesInfoStrings()) {
      CopyUuidToUserBuffer(entry.first, uuid);

      if (CopyStringToUserBuffer(entry.second, json, jsonsize)) {
        fn(YOGI_OK, userarg);
      } else {
        fn(YOGI_ERR_BUFFER_TOO_SMALL, userarg);
        buffer_too_small = true;
      }
    }

    if (buffer_too_small) {
      return YOGI_ERR_BUFFER_TOO_SMALL;
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_BranchAwaitEventAsync(void* branch, int events, void* uuid,
                                        char* json, int jsonsize,
                                        void (*fn)(int, int, int, void*),
                                        void* userarg) {
  CHECK_PARAM(branch != nullptr);
  CHECK_FLAGS(events, api::BranchEvents::kAllEvents);
  CHECK_PARAM(json == nullptr || jsonsize > 0);
  CHECK_PARAM(fn != nullptr);

  try {
    auto brn = api::ObjectRegister::Get<objects::branch::Branch>(branch);

    brn->AwaitEventAsync(
        ConvertFlags(events, api::BranchEvents::kNoEvent),
        [=](auto& res, auto event, auto& evres, auto& tmp_uuid,
            auto& tmp_json) {
          if (res != api::kSuccess) {
            fn(res.GetValue(), event, evres.GetValue(), userarg);
            return;
          }

          CopyUuidToUserBuffer(tmp_uuid, uuid);
          if (CopyStringToUserBuffer(tmp_json, json, jsonsize)) {
            fn(res.GetValue(), event, evres.GetValue(), userarg);
          } else {
            fn(YOGI_ERR_BUFFER_TOO_SMALL, event, evres.GetValue(), userarg);
          }
        });
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_BranchCancelAwaitEvent(void* branch) {
  CHECK_PARAM(branch != nullptr);

  try {
    auto brn = api::ObjectRegister::Get<objects::branch::Branch>(branch);
    if (!brn->CancelAwaitEvent()) {
      return YOGI_ERR_OPERATION_NOT_RUNNING;
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_BranchSendBroadcast(void* branch, int enc, const void* data,
                                      int datasize, int block) {
  CHECK_PARAM(branch != nullptr);
  CHECK_PARAM(enc == api::Encoding::kJson || enc == api::Encoding::kMsgPack);
  CHECK_PARAM(data != nullptr);
  CHECK_PARAM(datasize > 0);
  CHECK_PARAM(block == YOGI_TRUE || block == YOGI_FALSE);

  try {
    auto brn = api::ObjectRegister::Get<objects::branch::Branch>(branch);
    auto encoding = static_cast<api::Encoding>(enc);
    auto buffer = boost::asio::buffer(data, static_cast<std::size_t>(datasize));

    return brn
        ->SendBroadcast(network::Payload(buffer, encoding), block == YOGI_TRUE)
        .GetErrorCode();
  }
  CATCH_AND_RETURN_ERRORS_ONLY;
}

YOGI_API int YOGI_BranchSendBroadcastAsync(
    void* branch, int enc, const void* data, int datasize, int retry,
    void (*fn)(int res, int oid, void* userarg), void* userarg) {
  CHECK_PARAM(branch != nullptr);
  CHECK_PARAM(enc == api::Encoding::kJson || enc == api::Encoding::kMsgPack);
  CHECK_PARAM(data != nullptr);
  CHECK_PARAM(datasize > 0);
  CHECK_PARAM(retry == YOGI_TRUE || retry == YOGI_FALSE);
  CHECK_PARAM(fn != nullptr);

  try {
    auto brn = api::ObjectRegister::Get<objects::branch::Branch>(branch);
    auto encoding = static_cast<api::Encoding>(enc);
    auto buffer = boost::asio::buffer(data, static_cast<std::size_t>(datasize));

    return brn->SendBroadcastAsync(
        network::Payload(buffer, encoding), retry == YOGI_TRUE,
        [=](auto& res, auto oid) { fn(res.GetErrorCode(), oid, userarg); });
  }
  CATCH_AND_RETURN_ERRORS_ONLY;
}

YOGI_API int YOGI_BranchCancelSendBroadcast(void* branch, int oid) {
  CHECK_PARAM(branch != nullptr);
  CHECK_PARAM(oid > 0);

  try {
    auto brn = api::ObjectRegister::Get<objects::branch::Branch>(branch);
    if (!brn->CancelSendBroadcast(oid)) {
      return YOGI_ERR_INVALID_OPERATION_ID;
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_BranchReceiveBroadcastAsync(
    void* branch, void* uuid, int enc, void* data, int datasize,
    void (*fn)(int res, int size, void* userarg), void* userarg) {
  CHECK_PARAM(branch != nullptr);
  CHECK_PARAM(enc == api::Encoding::kJson || enc == api::Encoding::kMsgPack);
  CHECK_PARAM(data != nullptr || datasize == 0);
  CHECK_PARAM(fn != nullptr);

  try {
    auto brn = api::ObjectRegister::Get<objects::branch::Branch>(branch);
    brn->ReceiveBroadcast(
        static_cast<api::Encoding>(enc),
        boost::asio::buffer(data, static_cast<std::size_t>(datasize)),
        [=](auto& res, auto& src_uuid, auto size) {
          if (uuid) {
            CopyUuidToUserBuffer(src_uuid, uuid);
          }

          fn(res.GetValue(), static_cast<int>(size), userarg);
        });
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_BranchCancelReceiveBroadcast(void* branch) {
  CHECK_PARAM(branch != nullptr);

  try {
    auto brn = api::ObjectRegister::Get<objects::branch::Branch>(branch);
    if (!brn->CancelReceiveBroadcast()) {
      return YOGI_ERR_OPERATION_NOT_RUNNING;
    }
  }
  CATCH_AND_RETURN;
}
