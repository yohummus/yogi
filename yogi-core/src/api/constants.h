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

#pragma once

#include "../config.h"

namespace api {

// clang-format off
#define SCC static constexpr const
SCC char*     kVersion                       = YOGI_HDR_VERSION;
SCC int       kVersionMajor                  = YOGI_HDR_VERSION_MAJOR;
SCC int       kVersionMinor                  = YOGI_HDR_VERSION_MINOR;
SCC int       kVersionPatch                  = YOGI_HDR_VERSION_PATCH;
SCC char*     kVersionSuffix                 = YOGI_HDR_VERSION_SUFFIX;
SCC char*     kDefaultAdvInterfaces          = "[\"localhost\"]";
SCC char*     kDefaultAdvAddress             = "ff02::8000:2439";
SCC int       kDefaultAdvPort                = 13531;
SCC long long kDefaultAdvInterval            = 1'000'000'000ll;
SCC long long kDefaultConnectionTimeout      = 3'000'000'000ll;
SCC int       kDefaultLoggerVerbosity        = YOGI_VB_INFO;
SCC char*     kDefaultLogTimeFormat          = "%F %T.%3";
SCC char*     kDefaultLogFormat              = "$t [T$T] $<$s $c: $m$>";
SCC int       kMaxMessagePayloadSize         = 32'768;
SCC char*     kDefaultTimeFormat             = "%FT%T.%3Z";
SCC char*     kDefaultInfiniteDurationString = "%-inf";
SCC char*     kDefaultDurationFormat         = "%-%dd %T.%3%6%9";
SCC char*     kDefaultInvalidHandleString    = "INVALID HANDLE";
SCC char*     kDefaultObjectFormat           = "$T [$x]";
SCC int       kMinTxQueueSize                = 35'000;
SCC int       kMaxTxQueueSize                = 10'000'000;
SCC int       kDefaultTxQueueSize            = kMinTxQueueSize;
SCC int       kMinRxQueueSize                = 35'000;
SCC int       kMaxRxQueueSize                = 10'000'000;
SCC int       kDefaultRxQueueSize            = kMinRxQueueSize;
SCC int       kDefaultWebPort                = 8443;
SCC char*     kDefaultWebInterfaces          = "[\"localhost\"]";
SCC long long kDefaultWebTimeout             = 30'000'000'000ll;
SCC int       kDefaultWebCacheSize           = 25'000'000;
SCC int       kMaxWebCacheSize               = 1'000'000'000;
SCC char*     kDefaultAdminUser              = "admin";
SCC char*     kDefaultAdminPassword          = "admin";
// clang-format on

SCC char* kDefaultSslPrivateKey =
    R"(-----BEGIN PRIVATE KEY-----
MIIEvwIBADANBgkqhkiG9w0BAQEFAASCBKkwggSlAgEAAoIBAQDccyREZEfU28sW
kdtTxK8XA5pMdAlulFTizhgh9KTs62uKgHWq2zx2ISlZ+4cQfhLTATZBVhwQVLuD
RiatrDqPL/cN2m5XER/vFUJw3vJm2u7qKmJGWQ0i4j3O7Yle+uYJHn+35TIUfEX5
CgYnpt65lAjfbN1tl2ACbESa4E2nymZMSyOzTFd8xlL+nID2eG3CvKEKsg/bzXIV
kf6IlNfpu1tCyeGlwKYaNMymP4Rgp/8Y+Gdp2NZh1JOjqZ9Cavn2divMdEb856hx
nnqmoR/IiRyXJU8BdY1x2+NjbBmdcMNHTGioqhmzny+L1Pdfq6KgN4sIj0dQqe2j
OfpMvJR1AgMBAAECggEBAJTmSLbRLRt/IduqxQP9s23OByhgsAY6tmNDZVV6EvPb
x5WTUucGkf4QRAf9NqtTHI/dH7Jd4TK+qLDwbyubSypv8WUwBptUe8dXCruGOyBx
/DG6UwrFF3Rab/kWxFSdD0dVjFq/umXP0st1k9awhXu/m4ad16owNq8sReJ+YrVT
ZTIgowAklZ9QKnOCjZ3gbDWS8nn6dgonwU0INJD7iKJ3Mw4rv7Q18/7G7T+YmwHO
QdTy8M7MBLKu+ifQjgh9khFZZ8G0/jdV9ZkLbTNR/OfyO7XsPhWMQehRWryN9x2a
TekT8K0xJRu6NytDvvdeb139JlQjH6DnVrtMj8I4R/kCgYEA+ADkhZnI3ceHS2Kf
+kUbbOJI6/ncm9eHOZ/dEVyIK+iYJO9Eba2XqjIMXl6scZpxRAakHG/zRA23fc+c
R0mKEIRxJz9kLxpocW8ib+0LIeb0XdK8xt6JabW4EOtLExu3lIXpa+hkH7xr30Bb
+OQZdUpDkk6gBtKbk433w9WfQPcCgYEA447Pn2zgE+Oj8sHEp1qdYbj7CMo0JcQj
u4mmb5+BT2FiAlmuetoFGo2/0uGm4ha4cFtj1u58y00Ai8c+dKOrv9LzDHe09jaD
uGu4vbwCC0l8wiTKrz5m/Wl3oGAi2tWYUEpDyFBYj9yUEsOtZnhY8S0e2LbrPV6L
IftsXcZs5vMCgYEAh3JtXK417zJ3KOvvdY9iwCACNaccHp0ixI0T/tRrppd86Mco
t0IU0CZPbQcF5XG1JLL/1GwsD2Hycir+Lkzys7yx0/6iJ7ujiThqNXjgGJ77nXZo
FbEamXpe0ha/xOrhY6OTbZTZgh+1RpPu50skwFNT/kckzTUfNQJXbERymtcCgYBn
XTJwP1tVjXymQ2AZiK6eehEFgBvJ39s/T5pbb7+kGB8V5Di+MxRIaITuvHM6lYl3
9/tU4QWsrzCFTZ7QCpx0SNUAH8wCXubcOYo0HRdsmxfhdCbhDV5o06R0I9cRQh6q
seEyN1TTHCXilCHKvvXfr1gopoYaXTyj1fn2arCDAwKBgQDHc9V+9IpZ2F+ZwSYn
didwWQfsAZx5EiXtnXnyx0hkNjgebFQAgDJY4vvRphaD0bumywfNMOz/59jjjVZg
cWXs9H91XtA7cT7wJi+xiGe6R8zRcVPfImwQchSsCoMGuyLCygJTJPMzGT+YbpBA
Umrn/FzslGZsXxwMCR0E8654Kw==
-----END PRIVATE KEY-----
)";

SCC char* kDefaultSslCertificateChain =
    R"(-----BEGIN CERTIFICATE-----
MIICvjCCAaYCCQCL4CgivAdrbzANBgkqhkiG9w0BAQsFADAhMQswCQYDVQQGEwJV
UzESMBAGA1UEAwwJbG9jYWxob3N0MB4XDTE5MDUxODA3MDQyOFoXDTI5MDUxNTA3
MDQyOFowITELMAkGA1UEBhMCVVMxEjAQBgNVBAMMCWxvY2FsaG9zdDCCASIwDQYJ
KoZIhvcNAQEBBQADggEPADCCAQoCggEBANxzJERkR9TbyxaR21PErxcDmkx0CW6U
VOLOGCH0pOzra4qAdarbPHYhKVn7hxB+EtMBNkFWHBBUu4NGJq2sOo8v9w3ablcR
H+8VQnDe8mba7uoqYkZZDSLiPc7tiV765gkef7flMhR8RfkKBiem3rmUCN9s3W2X
YAJsRJrgTafKZkxLI7NMV3zGUv6cgPZ4bcK8oQqyD9vNchWR/oiU1+m7W0LJ4aXA
pho0zKY/hGCn/xj4Z2nY1mHUk6Opn0Jq+fZ2K8x0RvznqHGeeqahH8iJHJclTwF1
jXHb42NsGZ1ww0dMaKiqGbOfL4vU91+roqA3iwiPR1Cp7aM5+ky8lHUCAwEAATAN
BgkqhkiG9w0BAQsFAAOCAQEAQQezEoFlVkRjB7x1QljRUAVqUSPpk4bERkynB3Nb
hajLKKwZMeO8F4GDkAnY2F7ZD6KPQkOlKMVJJVmtVi3OTvutDJnc8jDXZUyejWe+
tSwLU2Uo7scPjYynyyPHcNkut+V7XjvhLJr267X0O6iymYnyJBGweVppwwgfxAmH
Rzv2uFLf+U7odtQmSC1g/VdImDoJsfASqnycXcqkmiDfv6Pjqp1dvUm9aDCIFdkf
DXShGXE+SVXQ61FVMhV62OsNY36mM5lR2kMXwgybRNMWla8Cmu8OhCkftOvVLdW2
tAVd+K6fpZe/mdCCuN3pXCCqu/0vPlFoLNMGZrKbLU8W6Q==
-----END CERTIFICATE-----
)";

SCC char* kDefaultSslDhParams =
    R"(-----BEGIN DH PARAMETERS-----
MIIBCAKCAQEA4iRXTwOzm97m19J21Pg67Vda5Ocw1PS6/CS+0ks//CpvNaYz0hJA
YbvCPzQLmxYyUH8NwYutfNrESWtBFs3qEKiZ9zx09cpacXf/gw4VBfclIl2HAlNo
5jWzh9VQBc3CxSNJqCRiJUvgnVCx1ec47cH3vkEucw0ewzxdwkpXgOGbja5BxCuO
Mtwb+qTXm4XozdAPPWlwryFvwJL60pvtsF7f9S8xZHIe309yeCNnUajmqyKwdJ18
P8RNYFHDe8vvaJ7/cnNCMqWFl16hVmBoIYy11H+R8WAphniJKV6fdGTs+7OLSc7Q
4/QFcIxwOY+wIaH2OEuOxTrKjAivFrGz8wIBAg==
-----END DH PARAMETERS-----
)";

#undef SCC

void GetConstant(void* dest, int constant);

}  // namespace api
