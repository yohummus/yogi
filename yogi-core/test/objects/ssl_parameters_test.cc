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

#include "../common.h"
#include "../../src/api/constants.h"
#include "../../src/objects/detail/web/ssl_parameters.h"
using objects::detail::web::SslParameters;

#include <boost/algorithm/string.hpp>

class SslParametersTest : public TestFixture {};

TEST_F(SslParametersTest, DefaultParameters) {
  SslParameters params(nlohmann::json::object_t{}, "");
  EXPECT_EQ(params.GetPrivateKey(), api::kDefaultSslPrivateKey);
  EXPECT_TRUE(params.GetPrivateKeyPassword().empty());
  EXPECT_EQ(params.GetCertificateChain(), api::kDefaultSslCertificateChain);
  EXPECT_EQ(params.GetDhParams(), api::kDefaultSslDhParams);
}

TEST_F(SslParametersTest, LoadParametersFromConfig) {
  auto ssl_cfg = nlohmann::json::parse(R"(
        {
          "private_key": [
            "-----BEGIN PRIVATE KEY-----",
            "Hello world",
            "-----END PRIVATE KEY-----"
          ],
          "certificate_chain": "-----BEGIN CERTIFICATE-----\nGood bye",
          "dh_params": "-----BEGIN DH PARAMETERS-----\nJippie"
        }
    )");

  SslParameters params(ssl_cfg, "");

  EXPECT_NE(params.GetPrivateKey().find("Hello world"), std::string::npos);
  EXPECT_NE(params.GetCertificateChain().find("Good bye"), std::string::npos);
  EXPECT_NE(params.GetDhParams().find("Jippie"), std::string::npos);
  EXPECT_TRUE(params.GetPrivateKeyPassword().empty());

  ssl_cfg["private_key"] = api::kDefaultSslPrivateKey;
  EXPECT_EQ(SslParameters(ssl_cfg, "").GetPrivateKey(),
            api::kDefaultSslPrivateKey);

  std::vector<std::string> lines;
  boost::split(lines, api::kDefaultSslPrivateKey, boost::is_any_of("\n"));
  lines.pop_back();  // Last empty line

  ssl_cfg["private_key"] = lines;
  EXPECT_EQ(SslParameters(ssl_cfg, "").GetPrivateKey(),
            api::kDefaultSslPrivateKey);
}

TEST_F(SslParametersTest, LoadParametersFromFiles) {
  auto key_file = MakeTestDataPath("ssl/key.pem");
  auto cert_file = MakeTestDataPath("ssl/cert.pem");
  auto dh_file = MakeTestDataPath("ssl/dh_params.pem");

  nlohmann::json ssl_cfg = {{"private_key_file", key_file},
                            {"certificate_chain_file", cert_file},
                            {"dh_params_file", dh_file}};

  SslParameters params(ssl_cfg, "");

  EXPECT_EQ(params.GetPrivateKey(), ReadFile(key_file));
  EXPECT_EQ(params.GetCertificateChain(), ReadFile(cert_file));
  EXPECT_EQ(params.GetDhParams(), ReadFile(dh_file));
}

TEST_F(SslParametersTest, PasswordRequired) {
  auto ssl_cfg = nlohmann::json::parse(R"(
        {
          "private_key": "-----BEGIN ENCRYPTED PRIVATE KEY-----"
        }
    )");

  EXPECT_THROW_DESCRIPTIVE_ERROR(SslParameters(ssl_cfg, ""),
                                 YOGI_ERR_CONFIG_NOT_VALID);

  ssl_cfg["private_key_password"] = "secret";
  EXPECT_NO_THROW(SslParameters(ssl_cfg, ""));
}

TEST_F(SslParametersTest, InvalidContent) {
  auto ssl_cfg = nlohmann::json::parse(R"(
        {
          "dh_params": "Jippie"
        }
    )");

  EXPECT_THROW_DESCRIPTIVE_ERROR(SslParameters(ssl_cfg, ""),
                                 YOGI_ERR_CONFIG_NOT_VALID);
}

TEST_F(SslParametersTest, AmbiguousSources) {
  auto ssl_cfg = nlohmann::json::parse(R"(
        {
          "dh_params": "-----BEGIN DH PARAMETERS-----\nJippie",
          "dh_params_file": "dh_params.pem"
        }
    )");

  EXPECT_THROW_DESCRIPTIVE_ERROR(SslParameters(ssl_cfg, ""),
                                 YOGI_ERR_CONFIG_NOT_VALID);
}
