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

#include "schema.h"
#include "../api/errors.h"
#include "../../3rd_party/json-schema-validator/src/json-schema.hpp"

#include <unordered_map>

// Generated through build system;
// Mapping is filename => content, e.g. "branch.schema.json" => "{...}"
extern std::map<std::string, std::string> Generated_GetSchemaFiles();

namespace utils {
namespace internal {
namespace {

using ValidatorsMap =
    std::unordered_map<std::string, nlohmann::json_schema::json_validator>;

ValidatorsMap MakeValidators() {
  ValidatorsMap map;

  for (auto& elem : Generated_GetSchemaFiles()) {
    map[elem.first].set_root_schema(nlohmann::json::parse(elem.second));
  }

  return map;
}

const ValidatorsMap validators = MakeValidators();

}  // anonymous namespace
}  // namespace internal

void ValidateJson(const nlohmann::json& json,
                  const std::string& schema_filename) {
  try {
    YOGI_ASSERT(internal::validators.count(schema_filename) > 0);
    internal::validators.at(schema_filename).validate(json);
  } catch (const std::exception& e) {
    throw api::DescriptiveError(YOGI_ERR_CONFIGURATION_VALIDATION_FAILED)
        << e.what();
  }
}

}  // namespace utils