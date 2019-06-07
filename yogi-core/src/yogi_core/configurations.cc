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

#include "macros.h"
#include "helpers.h"
#include "../objects/configuration.h"
#include "../../3rd_party/json-schema-validator/src/json-schema.hpp"

#include <iostream>

YOGI_API int YOGI_ConfigurationCreate(void** config, int flags) {
  using Flags = api::ConfigurationFlags;

  CHECK_PARAM(config != nullptr);
  CHECK_FLAGS(flags, Flags::kAllFlags);

  try {
    auto cfg =
        objects::Configuration::Create(ConvertFlags(flags, Flags::kNoFlags));
    *config = api::ObjectRegister::Register(cfg);
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_ConfigurationUpdateFromCommandLine(void* config, int argc,
                                                     const char* const* argv,
                                                     int options, char* err,
                                                     int errsize) {
  using Options = api::CommandLineOptions;

  CHECK_PARAM(config != nullptr);
  CHECK_PARAM(argc > 0);
  CHECK_PARAM(argv != nullptr);
  CHECK_FLAGS(options, Options::kAllOptions);
  CHECK_PARAM(err == nullptr || errsize > 0);

  try {
    auto cfg = api::ObjectRegister::Get<objects::Configuration>(config);
    cfg->UpdateFromCommandLine(argc, argv,
                               ConvertFlags(options, Options::kNoOptions));
  }
  CATCH_DESCRIPTIVE_AND_RETURN(err, errsize);
}

YOGI_API int YOGI_ConfigurationUpdateFromJson(void* config, const char* json,
                                              char* err, int errsize) {
  CHECK_PARAM(config != nullptr);
  CHECK_PARAM(json != nullptr);
  CHECK_PARAM(err == nullptr || errsize > 0);

  try {
    auto cfg = api::ObjectRegister::Get<objects::Configuration>(config);
    cfg->UpdateFromString(json);
  }
  CATCH_DESCRIPTIVE_AND_RETURN(err, errsize);
}

YOGI_API int YOGI_ConfigurationUpdateFromFile(void* config,
                                              const char* filename, char* err,
                                              int errsize) {
  CHECK_PARAM(config != nullptr);
  CHECK_PARAM(filename != nullptr);
  CHECK_PARAM(err == nullptr || errsize > 0);

  try {
    auto cfg = api::ObjectRegister::Get<objects::Configuration>(config);
    cfg->UpdateFromFile(filename);
  }
  CATCH_DESCRIPTIVE_AND_RETURN(err, errsize);
}

YOGI_API int YOGI_ConfigurationDump(void* config, char* json, int jsonsize,
                                    int resvars, int indent) {
  CHECK_PARAM(config != nullptr);
  CHECK_PARAM(json != nullptr);
  CHECK_PARAM(jsonsize > 0);
  CHECK_PARAM(resvars == YOGI_TRUE || resvars == YOGI_FALSE);
  CHECK_PARAM(indent >= -1);

  try {
    auto cfg = api::ObjectRegister::Get<objects::Configuration>(config);
    auto str = cfg->Dump(resvars == YOGI_TRUE, indent);
    if (!CopyStringToUserBuffer(str, json, jsonsize)) {
      return YOGI_ERR_BUFFER_TOO_SMALL;
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_ConfigurationWriteToFile(void* config, const char* filename,
                                           int resvars, int indent) {
  CHECK_PARAM(config != nullptr);
  CHECK_PARAM(filename != nullptr);
  CHECK_PARAM(resvars == YOGI_TRUE || resvars == YOGI_FALSE);
  CHECK_PARAM(indent >= -1);

  try {
    auto cfg = api::ObjectRegister::Get<objects::Configuration>(config);
    cfg->WriteToFile(filename, resvars == YOGI_TRUE, indent);
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_ConfigurationValidate(void* config, const char* section,
                                        void* schema, char* err, int errsize) {
  CHECK_PARAM(config != nullptr);
  CHECK_PARAM(schema != nullptr);
  CHECK_PARAM(err == nullptr || errsize > 0);

  try {
    auto jsn = UserSuppliedConfigToJson(config, section);
    auto sma = api::ObjectRegister::Get<objects::Configuration>(schema);

    try {
      nlohmann::json_schema::json_validator validator;
      validator.set_root_schema(sma->GetJson());
      validator.validate(jsn);
    } catch (const std::exception& e) {
      throw api::DescriptiveError(YOGI_ERR_CONFIGURATION_VALIDATION_FAILED)
          << e.what();
    }
  }
  CATCH_DESCRIPTIVE_AND_RETURN(err, errsize);
}