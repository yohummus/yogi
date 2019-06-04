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

#include <vector>
#include <sstream>
#include <string.h>

class LicensesTest : public TestFixture {
 protected:
  std::vector<std::string> GetIndividual3rdPartyLicenses() {
    std::string all = YOGI_Get3rdPartyLicenses();

    std::vector<std::string> licenses;
    std::string::size_type start = 0;
    while (start < all.size()) {
      auto end = all.find("\n\n\n", start);
      licenses.push_back(all.substr(start, end - start));
      start = end + 3;
    }

    return licenses;
  }

  void Check3rdPartyLicense(std::vector<std::string> strings_to_find) {
    auto licenses = GetIndividual3rdPartyLicenses();
    for (auto& license : licenses) {
      std::size_t strings_found = 0;
      for (auto& str : strings_to_find) {
        if (license.find(str) != std::string::npos) {
          ++strings_found;
        }
      }

      if (strings_found == strings_to_find.size()) {
        SUCCEED();
        return;
      }
    }

    FAIL() << "Required strings have not been found in the 3rd party licenses";
  }
};

TEST_F(LicensesTest, YogiLicense) {
  auto license = YOGI_GetLicense();
  EXPECT_NE(strstr(license, "GNU GENERAL PUBLIC LICENSE"), nullptr);
  EXPECT_NE(strstr(license, "Version 3, 29 June 2007"), nullptr);
}

TEST_F(LicensesTest, JsonLicense) {
  std::stringstream ss;
  ss << "version " << NLOHMANN_JSON_VERSION_MAJOR << '.'
     << NLOHMANN_JSON_VERSION_MINOR << '.' << NLOHMANN_JSON_VERSION_PATCH;

  // clang-format off
  Check3rdPartyLicense({
    "JSON for Modern C++",
    "https://github.com/nlohmann/json",
    "Licensed under the MIT License",
    "2013-2019 Niels Lohmann",
    ss.str()
  });
  // clang-format on
}

TEST_F(LicensesTest, JsonSchemaValidatorLicense) {
  // clang-format off
  Check3rdPartyLicense({
    "Modern C++ JSON schema validator",
    "licensed under the MIT License",
    "2016 Patrick Boettcher"
  });
  // clang-format on
}

TEST_F(LicensesTest, OpenSslLicense) {
  // clang-format off
  Check3rdPartyLicense({
    "OpenSSL License",
    "The OpenSSL Project",
    "http://www.openssl.org",
    "Original SSLeay License"
  });
  // clang-format on
}

TEST_F(LicensesTest, BoostLicense) {
  // clang-format off
  Check3rdPartyLicense({
    "Boost Software License - Version 1.0",
    "WITHOUT WARRANTY OF ANY KIND"
  });
  // clang-format on
}

TEST_F(LicensesTest, MessagePackLicense) {
  // clang-format off
  Check3rdPartyLicense({
    "Msgpack for C/C++",
    "Boost Software License - Version 1.0",
    "FURUHASHI Sadayuki"
  });
  // clang-format on
}
