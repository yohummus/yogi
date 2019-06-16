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
#include "../objects/web/web_server.h"
#include "../utils/system.h"

#include <nlohmann/json.hpp>
#include <string>
using namespace std::string_literals;

YOGI_API int YOGI_WebServerCreate(void** server, void* context, void* branch,
                                  void* config, const char* section, char* err,
                                  int errsize) {
  CHECK_PARAM(server != nullptr);
  CHECK_PARAM(context != nullptr || branch != nullptr);

  try {
    objects::branch::BranchPtr brn;
    if (branch) brn = api::ObjectRegister::Get<objects::branch::Branch>(branch);

    objects::ContextPtr ctx;
    if (context) {
      ctx = api::ObjectRegister::Get<objects::Context>(context);
    } else {
      ctx = brn->GetContext();
    }

    auto cfg = UserSuppliedConfigToJson(config, section);

    auto svr = objects::web::WebServer::Create(ctx, brn, cfg);
    svr->Start();

    *server = api::ObjectRegister::Register(svr);
  }
  CATCH_DESCRIPTIVE_AND_RETURN(err, errsize);
}

YOGI_API int YOGI_WebServerAddWorker(void* server, void* context) {
  return YOGI_ERR_UNKNOWN;
}

YOGI_API int YOGI_WebServerRemoveWorker(void* server, void* context) {
  return YOGI_ERR_UNKNOWN;
}

YOGI_API int YOGI_WebRouteCreate(
    void** route, void* server, const char* baseuri,
    void (*fn)(int res, int rid, const char* user, const char* owner,
               int method, const char* uri, const char* const* params,
               void* userarg),
    void* userarg) {
  return YOGI_ERR_UNKNOWN;
}

YOGI_API int YOGI_WebRouteRespond(void* route, int rid, int finished,
                                  int status, const char* contype,
                                  const char* content) {
  return YOGI_ERR_UNKNOWN;
}

YOGI_API int YOGI_WebProcessCreate(void** wproc, void* server, const char* name,
                                   void (*fn)(int res, int wpid,
                                              const char* user, int wpa,
                                              const void* data,
                                              const int* sizes, void* userarg),
                                   void* userarg) {
  return YOGI_ERR_UNKNOWN;
}

YOGI_API int YOGI_WebProcessUpdate(void* wproc, int wpid, int wpu, int exitcode,
                                   const void* output, int outsize) {
  return YOGI_ERR_UNKNOWN;
}