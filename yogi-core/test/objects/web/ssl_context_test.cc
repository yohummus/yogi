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
#include "../../../src/api/constants.h"
#include "../../../src/objects/web/detail/ssl_context.h"
using objects::web::detail::SslContext;

#include <boost/algorithm/string.hpp>

class SslContextTest : public TestFixture {};

TEST_F(SslContextTest, DefaultParameters) {
  SslContext params(nlohmann::json::object_t{}, "");
  EXPECT_EQ(params.GetPrivateKey(), api::kDefaultSslPrivateKey);
  EXPECT_TRUE(params.GetPrivateKeyPassword().empty());
  EXPECT_EQ(params.GetCertificateChain(), api::kDefaultSslCertificateChain);
  EXPECT_EQ(params.GetDhParams(), api::kDefaultSslDhParams);
}

TEST_F(SslContextTest, LoadParametersFromConfig) {
  auto cfg = nlohmann::json::parse(R"(
      { 
        "ssl": {
          "private_key": [
            "-----BEGIN PRIVATE KEY-----",
            "MIIBVgIBADANBgkqhkiG9w0BAQEFAASCAUAwggE8AgEAAkEAzbIgMDosEQjJ6ok5",
            "lsqefondagXzmXbZR9V4LtYti0TW2KwHmU0VRjCTh4x84V/C1GORZEEvPb1ssMMC",
            "NFNG3QIDAQABAkEApSxOMKvlyt/11AHRXsbhstbeF08kJ62KGObQFTwjYlq5ocYl",
            "gQefcC0GMIcFuWHAizpANhSJ1TIqCmicOcy4eQIhAPTlDLl6S/N+xW279XgPHbjV",
            "mPZGNXXhYBz6jtvAB1qbAiEA1wYFXb/8N21TUchLrQ3DZYftkKg0QXEqSUwp8P3D",
            "X+cCIFFTj3o5BugB/YaHmOkZ/ADZzkZa4n4XLlfxTWfkyYxpAiEAriIQInYh+Xkt",
            "6qM8te7ZeNX1zBqMBxtwVV+675Fysa0CIQDy23Z/NW5nKy7T2ZcaMHbfNuS8pk7x",
            "Ija+y+OXrS42zw==",
            "-----END PRIVATE KEY-----"
          ],
          "certificate_chain": "-----BEGIN CERTIFICATE-----\nMIIBpjCCAVACCQD6u9MP7vVzdDANBgkqhkiG9w0BAQsFADBaMQswCQYDVQQGEwJE\nRTEQMA4GA1UECAwHQmF2YXJpYTEPMA0GA1UEBwwGTXVuaWNoMREwDwYDVQQKDAhZ\nb2h1bW11czEVMBMGA1UEAwwMeW9odW1tdXMuY29tMB4XDTE5MDYxOTE5NTg0MFoX\nDTI5MDYxNjE5NTg0MFowWjELMAkGA1UEBhMCREUxEDAOBgNVBAgMB0JhdmFyaWEx\nDzANBgNVBAcMBk11bmljaDERMA8GA1UECgwIWW9odW1tdXMxFTATBgNVBAMMDHlv\naHVtbXVzLmNvbTBcMA0GCSqGSIb3DQEBAQUAA0sAMEgCQQDNsiAwOiwRCMnqiTmW\nyp5+id1qBfOZdtlH1Xgu1i2LRNbYrAeZTRVGMJOHjHzhX8LUY5FkQS89vWywwwI0\nU0bdAgMBAAEwDQYJKoZIhvcNAQELBQADQQADklORc8C+Lm29yOQETzChgyNMw4JB\nRrARS/tSChp8y7B1ncdi4/rvKqqDHWUyKVFYYcTbNsFIRyZla58qp8W3\n-----END CERTIFICATE-----",
          "dh_params": "-----BEGIN DH PARAMETERS-----\nMIGHAoGBAMO+hfoi2MZW+jCoN9IKxwaLM6Hi6LWz/pWsu2wa23cQbmgoT9TNh0se\nYonBuToobDRuztMSG6uIrZp7TmkPz293Xh540pXIoVnViucHaSiY9xBiMrjXDFZa\nhR0JWscJ8wruqDfH40sj8aEn3c+w39xXwuVsKw14LDix2fPQQwPzAgEC\n-----END DH PARAMETERS-----"
        }
      }
    )");

  SslContext params(cfg, "");

  EXPECT_NE(params.GetPrivateKey().find("MIIBVgIBADANB"), std::string::npos);
  EXPECT_NE(params.GetCertificateChain().find("MIIBpjCCAV"), std::string::npos);
  EXPECT_NE(params.GetDhParams().find("MIGHAoGBAMO+hfo"), std::string::npos);
  EXPECT_TRUE(params.GetPrivateKeyPassword().empty());

  cfg["ssl"]["certificate_chain"] = api::kDefaultSslCertificateChain;

  cfg["ssl"]["private_key"] = api::kDefaultSslPrivateKey;
  EXPECT_EQ(SslContext(cfg, "").GetPrivateKey(), api::kDefaultSslPrivateKey);

  std::vector<std::string> lines;
  boost::split(lines, api::kDefaultSslPrivateKey, boost::is_any_of("\n"));
  lines.pop_back();  // Last empty line

  cfg["ssl"]["private_key"] = lines;
  EXPECT_EQ(SslContext(cfg, "").GetPrivateKey(), api::kDefaultSslPrivateKey);
}

TEST_F(SslContextTest, LoadParametersFromFiles) {
  auto key_file = MakeTestDataPath("ssl/key.pem");
  auto cert_file = MakeTestDataPath("ssl/cert.pem");
  auto dh_file = MakeTestDataPath("ssl/dh_params.pem");

  nlohmann::json cfg = {{"ssl",
                         {{"private_key_file", key_file},
                          {"certificate_chain_file", cert_file},
                          {"dh_params_file", dh_file}}}};

  SslContext params(cfg, "");

  EXPECT_EQ(params.GetPrivateKey(), ReadFile(key_file));
  EXPECT_EQ(params.GetCertificateChain(), ReadFile(cert_file));
  EXPECT_EQ(params.GetDhParams(), ReadFile(dh_file));
}

TEST_F(SslContextTest, LoadParametersFromNonExistingFile) {
  auto cfg = nlohmann::json::parse(R"(
      {
        "ssl": {
          "private_key_file": "does_not_exist.pem"
        }
      }
    )");

  EXPECT_THROW_DESCRIPTIVE_ERROR(SslContext(cfg, ""),
                                 YOGI_ERR_READ_FILE_FAILED);
}

TEST_F(SslContextTest, PasswordRequired) {
  nlohmann::json cfg = {
      {"ssl",
       {{"private_key_file", MakeTestDataPath("ssl/key_encrypted.pem")},
        {"certificate_chain_file", MakeTestDataPath("ssl/cert.pem")}}}};

  EXPECT_THROW_DESCRIPTIVE_ERROR(SslContext(cfg, ""),
                                 YOGI_ERR_CONFIG_NOT_VALID);

  cfg["ssl"]["private_key_password"] = "secret";

  EXPECT_NO_THROW(SslContext(cfg, ""));
}

TEST_F(SslContextTest, InvalidContent) {
  auto cfg = nlohmann::json::parse(R"(
      {
        "ssl": {
          "dh_params": "Jippie"
        }
      }
    )");

  EXPECT_THROW_DESCRIPTIVE_ERROR(SslContext(cfg, ""),
                                 YOGI_ERR_CONFIG_NOT_VALID);
}

TEST_F(SslContextTest, AmbiguousSources) {
  auto cfg = nlohmann::json::parse(R"(
      {
        "ssl": {
          "dh_params": "-----BEGIN DH PARAMETERS-----\nJippie",
          "dh_params_file": "dh_params.pem"
        }
      }
    )");

  EXPECT_THROW_DESCRIPTIVE_ERROR(SslContext(cfg, ""),
                                 YOGI_ERR_CONFIGURATION_VALIDATION_FAILED);
}
