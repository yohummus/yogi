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

#include "../../common.h"

namespace fs = boost::filesystem;

class ConfigurationTest : public TestFixture {
 protected:
  virtual void SetUp() override { cfg_ = MakeConfiguration(YOGI_CFG_NONE); }

  void* MakeConfiguration(
      int flags,
      const char* json = R"({"person": {"name": "Joe", "age": 42}})") {
    void* cfg = nullptr;
    int res = YOGI_ConfigurationCreate(&cfg, flags);
    EXPECT_OK(res);
    EXPECT_NE(cfg, nullptr);

    res = YOGI_ConfigurationUpdateFromJson(cfg, json, nullptr, 0);
    EXPECT_OK(res);

    return cfg;
  }

  TemporaryWorkdirGuard WriteTempFile(
      const char* filename,
      const char* content = R"({"person": {"age": 10}})") {
    TemporaryWorkdirGuard workdir;

    fs::ofstream file(filename);
    file << content;

    return workdir;
  }

  nlohmann::json DumpConfiguration(void* cfg) {
    char str[1000];
    int res = YOGI_ConfigurationDump(cfg, str, sizeof(str), YOGI_TRUE, -1);
    if (res == YOGI_ERR_NO_VARIABLE_SUPPORT) {
      res = YOGI_ConfigurationDump(cfg, str, sizeof(str), YOGI_FALSE, -1);
    }
    EXPECT_OK(res);

    return nlohmann::json::parse(str);
  }

  void CheckConfigurationIsOriginal() {
    auto expected =
        nlohmann::json::parse(R"({"person": {"name": "Joe", "age": 42}})");
    auto actual = DumpConfiguration(cfg_);
    EXPECT_TRUE(actual == expected) << "Expected:" << std::endl
                                    << expected.dump(2) << std::endl
                                    << "Actual:" << std::endl
                                    << actual.dump(2) << std::endl;
  }

  void* cfg_;
};

TEST_F(ConfigurationTest, UpdateFromJson) {
  char err_desc[1000];
  int res = YOGI_ConfigurationUpdateFromJson(cfg_, R"({"person": {"age": 10}})",
                                             err_desc, sizeof(err_desc));
  EXPECT_OK(res) << err_desc;
  EXPECT_STREQ(err_desc, "");

  auto json = DumpConfiguration(cfg_);
  ASSERT_TRUE(json.count("person")) << json;
  EXPECT_EQ(json["person"].value("name", "NOT FOUND"), "Joe");
  EXPECT_EQ(json["person"].value("age", -1), 10);
}

TEST_F(ConfigurationTest, UpdateFromCorruptJson) {
  char err_desc[1000];
  int res = YOGI_ConfigurationUpdateFromJson(cfg_, R"({"person": {"age": 10})",
                                             err_desc, sizeof(err_desc));
  EXPECT_ERR(res, YOGI_ERR_PARSING_JSON_FAILED);
  EXPECT_STRNE(err_desc, "");

  CheckConfigurationIsOriginal();
}

TEST_F(ConfigurationTest, UpdateFromCommandLine) {
  auto workdir = WriteTempFile("a.json");

  // clang-format off
  CommandLine cmdline{
    "--name", "My Branch",
    "a.json",
  };
  // clang-format on

  char err_desc[1000];
  int res = YOGI_ConfigurationUpdateFromCommandLine(cfg_, cmdline.argc,
                                                    cmdline.argv, YOGI_CLO_ALL,
                                                    err_desc, sizeof(err_desc));
  ASSERT_OK(res) << err_desc;
  EXPECT_STREQ(err_desc, "");

  auto json = DumpConfiguration(cfg_);
  ASSERT_TRUE(json.count("person")) << json;
  EXPECT_EQ(json["person"].value("name", "NOT FOUND"), "Joe");
  EXPECT_EQ(json["person"].value("age", -1), 10);
  ASSERT_TRUE(json.count("branch")) << json;
  EXPECT_EQ(json["branch"].value("name", "NOT FOUND"), "My Branch");
}

TEST_F(ConfigurationTest, UpdateFromCommandLineCorruptFile) {
  auto workdir = WriteTempFile("a.json", R"({"person": {"age": 10})");

  // clang-format off
  CommandLine cmdline{
    "--name", "My Branch",
    "a.json",
  };
  // clang-format on

  char err_desc[1000];
  int res = YOGI_ConfigurationUpdateFromCommandLine(cfg_, cmdline.argc,
                                                    cmdline.argv, YOGI_CLO_ALL,
                                                    err_desc, sizeof(err_desc));
  ASSERT_ERR(res, YOGI_ERR_PARSING_FILE_FAILED);
  EXPECT_STRNE(err_desc, "");

  CheckConfigurationIsOriginal();
}

TEST_F(ConfigurationTest, UpdateFromFile) {
  auto workdir = WriteTempFile("a.json");

  char err_desc[1000];
  int res = YOGI_ConfigurationUpdateFromFile(cfg_, "a.json", err_desc,
                                             sizeof(err_desc));
  ASSERT_OK(res) << err_desc;
  EXPECT_STREQ(err_desc, "");

  auto json = DumpConfiguration(cfg_);
  ASSERT_TRUE(json.count("person")) << json;
  EXPECT_EQ(json["person"].value("name", "NOT FOUND"), "Joe");
  EXPECT_EQ(json["person"].value("age", -1), 10);
}

TEST_F(ConfigurationTest, UpdateFromCorruptFile) {
  auto workdir = WriteTempFile("a.json", R"({"person": {"age": 10})");

  char err_desc[1000];
  int res = YOGI_ConfigurationUpdateFromFile(cfg_, "a.json", err_desc,
                                             sizeof(err_desc));
  ASSERT_ERR(res, YOGI_ERR_PARSING_FILE_FAILED);
  EXPECT_STRNE(err_desc, "");

  CheckConfigurationIsOriginal();
}

TEST_F(ConfigurationTest, Dump) {
  void* cfg1 = MakeConfiguration(YOGI_CFG_NONE);
  void* cfg2 = MakeConfiguration(YOGI_CFG_DISABLE_VARIABLES);

  char str[1000];
  int res = YOGI_ConfigurationDump(cfg1, str, sizeof(str), YOGI_TRUE, -1);
  EXPECT_OK(res);
  EXPECT_STRNE(str, "");
  EXPECT_EQ(std::string(str).find_first_of(" \n"), std::string::npos);

  res = YOGI_ConfigurationDump(cfg1, str, sizeof(str), YOGI_FALSE, -1);
  EXPECT_OK(res);
  EXPECT_STRNE(str, "");
  EXPECT_EQ(std::string(str).find_first_of(" \n"), std::string::npos);

  res = YOGI_ConfigurationDump(cfg1, str, sizeof(str), YOGI_TRUE, 2);
  EXPECT_OK(res);
  EXPECT_STRNE(str, "");
  EXPECT_NE(std::string(str).find('\n'), std::string::npos);
  EXPECT_NE(std::string(str).find("  "), std::string::npos);

  res = YOGI_ConfigurationDump(cfg2, str, sizeof(str), YOGI_FALSE, -1);
  EXPECT_OK(res);
  EXPECT_STRNE(str, "");
  EXPECT_EQ(std::string(str).find_first_of(" \n"), std::string::npos);

  res = YOGI_ConfigurationDump(cfg2, str, sizeof(str), YOGI_TRUE, -1);
  EXPECT_ERR(res, YOGI_ERR_NO_VARIABLE_SUPPORT);
}

TEST_F(ConfigurationTest, WriteToFile) {
  TemporaryWorkdirGuard workdir;
  void* cfg1 = MakeConfiguration(YOGI_CFG_NONE);
  void* cfg2 = MakeConfiguration(YOGI_CFG_DISABLE_VARIABLES);

  int res = YOGI_ConfigurationWriteToFile(cfg1, "a.json", YOGI_TRUE, -1);
  EXPECT_OK(res);
  auto content = ReadFile("a.json");
  EXPECT_FALSE(content.empty());
  EXPECT_EQ(std::string(content).find_first_of(" \n"), std::string::npos);

  res = YOGI_ConfigurationWriteToFile(cfg1, "b.json", YOGI_FALSE, -1);
  EXPECT_OK(res);
  content = ReadFile("b.json");
  EXPECT_FALSE(content.empty());
  EXPECT_EQ(std::string(content).find_first_of(" \n"), std::string::npos);

  res = YOGI_ConfigurationWriteToFile(cfg1, "c.json", YOGI_TRUE, 2);
  EXPECT_OK(res);
  content = ReadFile("c.json");
  EXPECT_FALSE(content.empty());
  EXPECT_NE(std::string(content).find('\n'), std::string::npos);
  EXPECT_NE(std::string(content).find("  "), std::string::npos);

  res = YOGI_ConfigurationWriteToFile(cfg2, "d.json", YOGI_FALSE, -1);
  EXPECT_OK(res);
  content = ReadFile("d.json");
  EXPECT_FALSE(content.empty());
  EXPECT_EQ(std::string(content).find_first_of(" \n"), std::string::npos);

  res = YOGI_ConfigurationWriteToFile(cfg2, "e.json", YOGI_TRUE, -1);
  EXPECT_ERR(res, YOGI_ERR_NO_VARIABLE_SUPPORT);
}

TEST_F(ConfigurationTest, ImmutableCommandLine) {
  auto workdir = WriteTempFile("a.json");

  // clang-format off
  CommandLine cmdline{
    "--name", "My Branch",
    "--override", "{\"person\":{\"age\":55}}",
    "a.json",
  };

  nlohmann::json json_update = {
    {"person", {
      {"age", 88}
    }},
    {"branch", {
      {"name", "Edgar"}
    }}
  };
  // clang-format on

  // Immutable command line
  void* cfg1 = MakeConfiguration(YOGI_CFG_NONE);
  int res = YOGI_ConfigurationUpdateFromCommandLine(
      cfg1, cmdline.argc, cmdline.argv, YOGI_CLO_ALL, nullptr, 0);
  EXPECT_OK(res);
  auto json = DumpConfiguration(cfg1);
  ASSERT_TRUE(json.count("person")) << json;
  EXPECT_EQ(json["person"].value("age", -1), 55);
  ASSERT_TRUE(json.count("branch")) << json;
  EXPECT_EQ(json["branch"].value("name", "NOT FOUND"), "My Branch");

  res = YOGI_ConfigurationUpdateFromJson(cfg1, json_update.dump().c_str(),
                                         nullptr, 0);
  EXPECT_OK(res);
  json = DumpConfiguration(cfg1);
  ASSERT_TRUE(json.count("person")) << json;
  EXPECT_EQ(json["person"].value("age", -1), 55);
  ASSERT_TRUE(json.count("branch")) << json;
  EXPECT_EQ(json["branch"].value("name", "NOT FOUND"), "My Branch");

  // Mutable command line
  void* cfg2 = MakeConfiguration(YOGI_CFG_MUTABLE_CMD_LINE);
  res = YOGI_ConfigurationUpdateFromCommandLine(
      cfg2, cmdline.argc, cmdline.argv, YOGI_CLO_ALL, nullptr, 0);
  EXPECT_OK(res);
  json = DumpConfiguration(cfg2);
  ASSERT_TRUE(json.count("person")) << json;
  EXPECT_EQ(json["person"].value("age", -1), 55);
  ASSERT_TRUE(json.count("branch")) << json;
  EXPECT_EQ(json["branch"].value("name", "NOT FOUND"), "My Branch");

  res = YOGI_ConfigurationUpdateFromJson(cfg2, json_update.dump().c_str(),
                                         nullptr, 0);
  EXPECT_OK(res);
  json = DumpConfiguration(cfg2);
  ASSERT_TRUE(json.count("person")) << json;
  EXPECT_EQ(json["person"].value("age", -1), 88);
  ASSERT_TRUE(json.count("branch")) << json;
  EXPECT_EQ(json["branch"].value("name", "NOT FOUND"), "Edgar");
}

TEST_F(ConfigurationTest, Variables) {
  // clang-format off
  nlohmann::json json_update = {
    {"person", {
      {"age", "${AGE}"},
      {"age-string", "${AGE} years"},
      {"name", "${NAME} Wayne"}
    }},
    {"variables", {
      {"AGE", 33},
      {"NAME", "Joe ${MIDDLENAME}"}
    }}
  };
  // clang-format on

  // clang-format off
  CommandLine cmdline{
    "--var", "MIDDLENAME=Rob",
  };
  // clang-format on

  char err_desc[1000];
  int res = YOGI_ConfigurationUpdateFromCommandLine(cfg_, cmdline.argc,
                                                    cmdline.argv, YOGI_CLO_ALL,
                                                    err_desc, sizeof(err_desc));
  ASSERT_OK(res);
  EXPECT_STREQ(err_desc, "");

  res = YOGI_ConfigurationUpdateFromJson(cfg_, json_update.dump().c_str(),
                                         err_desc, sizeof(err_desc));
  ASSERT_OK(res) << err_desc;
  EXPECT_STREQ(err_desc, "");

  auto json = DumpConfiguration(cfg_);
  ASSERT_TRUE(json.count("person")) << json;
  EXPECT_EQ(json["person"].value("age", -1), 33);
  EXPECT_EQ(json["person"].value("age-string", "NOT FOUND"), "33 years");
  EXPECT_EQ(json["person"].value("name", "NOT FOUND"), "Joe Rob Wayne");
  ASSERT_TRUE(json.count("variables")) << json;
  EXPECT_EQ(json["variables"].value("AGE", -1), 33);
  EXPECT_EQ(json["variables"].value("NAME", "NOT FOUND"), "Joe Rob");
  EXPECT_EQ(json["variables"].value("MIDDLENAME", "NOT FOUND"), "Rob");
}

TEST_F(ConfigurationTest, BadVariables) {
  // clang-format off
  nlohmann::json json = {
    {"name", "My ${NAME} please"}
  };
  // clang-format on

  char err_desc[1000];
  int res = YOGI_ConfigurationUpdateFromJson(cfg_, json.dump().c_str(),
                                             err_desc, sizeof(err_desc));
  ASSERT_ERR(res, YOGI_ERR_UNDEFINED_VARIABLES) << err_desc;
  EXPECT_NE(std::string(err_desc).find("NAME"), std::string::npos);

  // clang-format off
  json = {
    {"name", "Var ${NAME is unterminated"}
  };
  // clang-format on

  res = YOGI_ConfigurationUpdateFromJson(cfg_, json.dump().c_str(), err_desc,
                                         sizeof(err_desc));
  ASSERT_ERR(res, YOGI_ERR_UNDEFINED_VARIABLES) << err_desc;
  EXPECT_NE(std::string(err_desc).find("NAME is unterminated"),
            std::string::npos);

  // clang-format off
  json = {
    {"variables", {
      {"ABX", "${ABO}"},
      {"ABO", "${ABI}"},
      {"ABI", "${ABX}"},
    }}
  };
  // clang-format on

  res = YOGI_ConfigurationUpdateFromJson(cfg_, json.dump().c_str(), err_desc,
                                         sizeof(err_desc));
  ASSERT_ERR(res, YOGI_ERR_UNDEFINED_VARIABLES) << err_desc;
  EXPECT_NE(std::string(err_desc).find("AB"), std::string::npos);
}

TEST_F(ConfigurationTest, Validate) {
  void* scm = MakeConfiguration(YOGI_CFG_NONE, R"(
    {
      "$schema": "http://json-schema.org/draft-07/schema#",
      "title": "Test schema",
      "properties": {
        "name": {
          "description": "Name",
          "type": "string"
        },
        "age": {
          "description": "Age of the person",
          "type": "number",
          "minimum": 2,
          "maximum": 200
        }
      },
      "required": ["name", "age"],
      "type": "object"
    }
  )");

  char err[256] = {0};
  int res = YOGI_ConfigurationValidate(cfg_, nullptr, scm, err, sizeof(err));
  EXPECT_ERR(res, YOGI_ERR_CONFIGURATION_VALIDATION_FAILED) << err;
  EXPECT_NE(std::string(err).find("not found"), std::string::npos) << err;

  res = YOGI_ConfigurationValidate(cfg_, "/person", scm, err, sizeof(err));
  EXPECT_OK(res) << err;
  EXPECT_STREQ(err, "") << err;

  res = YOGI_ConfigurationValidate(cfg_, "/someone", scm, err, sizeof(err));
  EXPECT_ERR(res, YOGI_ERR_CONFIGURATION_SECTION_NOT_FOUND) << err;
  EXPECT_NE(std::string(err).find("someone"), std::string::npos) << err;

  res = YOGI_ConfigurationUpdateFromJson(cfg_, R"({"person": {"age": 500}})",
                                         err, sizeof(err));
  EXPECT_OK(res) << err;

  res = YOGI_ConfigurationValidate(cfg_, "/person", scm, err, sizeof(err));
  EXPECT_ERR(res, YOGI_ERR_CONFIGURATION_VALIDATION_FAILED) << err;
  EXPECT_NE(std::string(err).find("/age"), std::string::npos) << err;
  EXPECT_NE(std::string(err).find("200"), std::string::npos) << err;
}