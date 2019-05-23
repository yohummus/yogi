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

using System;
using Xunit;

namespace test
{
    public class ConstantsTest : TestCase
    {
        [Fact]
        public void Version()
        {
            Assert.IsType<string>(Yogi.Constants.Version);
            Assert.Equal(Yogi.Constants.Version, GetCoreMacroString("YOGI_HDR_VERSION"));
        }

        [Fact]
        public void VersionMajor()
        {
            Assert.IsType<int>(Yogi.Constants.VersionMajor);
            Assert.Equal(Yogi.Constants.VersionMajor, GetCoreMacroInt("YOGI_HDR_VERSION_MAJOR"));
        }

        [Fact]
        public void VersionMinor()
        {
            Assert.IsType<int>(Yogi.Constants.VersionMinor);
            Assert.Equal(Yogi.Constants.VersionMinor, GetCoreMacroInt("YOGI_HDR_VERSION_MINOR"));
        }

        [Fact]
        public void VersionPatch()
        {
            Assert.IsType<int>(Yogi.Constants.VersionPatch);
            Assert.Equal(Yogi.Constants.VersionPatch, GetCoreMacroInt("YOGI_HDR_VERSION_PATCH"));
        }

        [Fact]
        public void VersionSuffix()
        {
            Assert.IsType<string>(Yogi.Constants.VersionSuffix);
            Assert.Equal(Yogi.Constants.VersionSuffix,
                         GetCoreMacroString("YOGI_HDR_VERSION_SUFFIX"));
        }

        [Fact]
        public void DefaultAdvInterfaces()
        {
            Assert.IsType<string>(Yogi.Constants.DefaultAdvInterfaces);
            Assert.Contains("localhost", Yogi.Constants.DefaultAdvInterfaces);
        }

        [Fact]
        public void DefaultAdvAddress()
        {
            Assert.IsType<string>(Yogi.Constants.DefaultAdvAddress);
            Assert.True(Yogi.Constants.DefaultAdvAddress.Length > 1);
        }

        [Fact]
        public void DefaultAdvPort()
        {
            Assert.IsType<int>(Yogi.Constants.DefaultAdvPort);
            Assert.True(Yogi.Constants.DefaultAdvPort >= 1024);
        }

        [Fact]
        public void DefaultAdvInterval()
        {
            Assert.IsType<Yogi.Duration>(Yogi.Constants.DefaultAdvInterval);
            Assert.True(Yogi.Constants.DefaultAdvInterval.TotalSeconds > 0.5);
        }

        [Fact]
        public void DefaultConnectionTimeout()
        {
            Assert.IsType<Yogi.Duration>(Yogi.Constants.DefaultConnectionTimeout);
            Assert.True(Yogi.Constants.DefaultConnectionTimeout.TotalSeconds > 0.5);
        }

        [Fact]
        public void DefaultLoggerVerbosity()
        {
            Assert.IsType<Yogi.Verbosity>(Yogi.Constants.DefaultLoggerVerbosity);
        }

        [Fact]
        public void DefaultLogTimeFormat()
        {
            Assert.IsType<string>(Yogi.Constants.DefaultLogTimeFormat);
            Assert.True(Yogi.Constants.DefaultLogTimeFormat.Length > 1);
        }

        [Fact]
        public void DefaultLogFormat()
        {
            Assert.IsType<string>(Yogi.Constants.DefaultLogFormat);
            Assert.True(Yogi.Constants.DefaultLogFormat.Length > 1);
        }

        [Fact]
        public void MaxMessagePayloadSize()
        {
            Assert.IsType<int>(Yogi.Constants.MaxMessagePayloadSize);
            Assert.True(Yogi.Constants.MaxMessagePayloadSize >= 1000);
        }

        [Fact]
        public void DefaultTimeFormat()
        {
            Assert.IsType<string>(Yogi.Constants.DefaultTimeFormat);
            Assert.Contains(".%3", Yogi.Constants.DefaultTimeFormat);
        }

        [Fact]
        public void DefaultInfiniteDurationString()
        {
            Assert.IsType<string>(Yogi.Constants.DefaultInfiniteDurationString);
            Assert.Contains("inf", Yogi.Constants.DefaultInfiniteDurationString);
        }

        [Fact]
        public void DefaultDurationFormat()
        {
            Assert.IsType<string>(Yogi.Constants.DefaultDurationFormat);
            Assert.Contains(".%3", Yogi.Constants.DefaultDurationFormat);
        }

        [Fact]
        public void DefaultInvalidHandleString()
        {
            Assert.IsType<string>(Yogi.Constants.DefaultInvalidHandleString);
            Assert.Contains("INVALID", Yogi.Constants.DefaultInvalidHandleString);
        }

        [Fact]
        public void DefaultObjectFormat()
        {
            Assert.IsType<string>(Yogi.Constants.DefaultObjectFormat);
            Assert.Contains("$T", Yogi.Constants.DefaultObjectFormat);
        }

        [Fact]
        public void MinTxQueueSize()
        {
            Assert.IsType<int>(Yogi.Constants.MinTxQueueSize);
            Assert.True(Yogi.Constants.MinTxQueueSize >= Yogi.Constants.MaxMessagePayloadSize);
        }

        [Fact]
        public void MaxTxQueueSize()
        {
            Assert.IsType<int>(Yogi.Constants.MaxTxQueueSize);
            Assert.True(Yogi.Constants.MaxTxQueueSize > Yogi.Constants.MinTxQueueSize);
        }

        [Fact]
        public void DefaultTxQueueSize()
        {
            Assert.IsType<int>(Yogi.Constants.DefaultTxQueueSize);
            Assert.True(Yogi.Constants.DefaultTxQueueSize < Yogi.Constants.MaxTxQueueSize);
        }

        [Fact]
        public void MinRxQueueSize()
        {
            Assert.IsType<int>(Yogi.Constants.MinRxQueueSize);
            Assert.True(Yogi.Constants.MinRxQueueSize >= Yogi.Constants.MaxMessagePayloadSize);
        }

        [Fact]
        public void MaxRxQueueSize()
        {
            Assert.IsType<int>(Yogi.Constants.MaxRxQueueSize);
            Assert.True(Yogi.Constants.MaxRxQueueSize > Yogi.Constants.MinRxQueueSize);
        }

        [Fact]
        public void DefaultRxQueueSize()
        {
            Assert.IsType<int>(Yogi.Constants.DefaultRxQueueSize);
            Assert.True(Yogi.Constants.DefaultRxQueueSize < Yogi.Constants.MaxRxQueueSize);
        }

        [Fact]
        public void DefaultWebPort()
        {
            Assert.IsType<int>(Yogi.Constants.DefaultWebPort);
            Assert.True(Yogi.Constants.DefaultWebPort > 1024);
            Assert.True(Yogi.Constants.DefaultWebPort < 65535);
        }

        [Fact]
        public void DefaultWebInterfaces()
        {
            Assert.IsType<string>(Yogi.Constants.DefaultWebInterfaces);
            Assert.Contains("localhost", Yogi.Constants.DefaultWebInterfaces);
        }

        [Fact]
        public void DefaultWebTimeout()
        {
            Assert.IsType<Yogi.Duration>(Yogi.Constants.DefaultWebTimeout);
            Assert.True(Yogi.Constants.DefaultWebTimeout.TotalSeconds > 0.5);
        }

        [Fact]
        public void DefaultWebCacheSize()
        {
            Assert.IsType<int>(Yogi.Constants.DefaultWebCacheSize);
            Assert.True(Yogi.Constants.DefaultWebCacheSize >= 1000000);
            Assert.True(Yogi.Constants.DefaultWebCacheSize < Yogi.Constants.MaxWebCacheSize);
        }

        [Fact]
        public void MaxWebCacheSize()
        {
            Assert.IsType<int>(Yogi.Constants.MaxWebCacheSize);
            Assert.True(Yogi.Constants.MaxWebCacheSize >= 100000000);
            Assert.True(Yogi.Constants.MaxWebCacheSize <= 1000000000);
        }

        [Fact]
        public void DefaultAdminUser()
        {
            Assert.IsType<string>(Yogi.Constants.DefaultAdminUser);
            Assert.True(Yogi.Constants.DefaultAdminUser.Length >= 3);
        }

        [Fact]
        public void DefaultAdminPassword()
        {
            Assert.IsType<string>(Yogi.Constants.DefaultAdminPassword);
            Assert.True(Yogi.Constants.DefaultAdminPassword.Length >= 3);
        }

        [Fact]
        public void DefaultSslPrivateKey()
        {
            Assert.IsType<string>(Yogi.Constants.DefaultSslPrivateKey);
            Assert.Contains("-----BEGIN PRIVATE KEY-----", Yogi.Constants.DefaultSslPrivateKey);
        }

        [Fact]
        public void DefaultSslCertificateChain()
        {
            Assert.IsType<string>(Yogi.Constants.DefaultSslCertificateChain);
            Assert.Contains("-----BEGIN CERTIFICATE-----",
                            Yogi.Constants.DefaultSslCertificateChain);
        }

        [Fact]
        public void DefaultSslDhParams()
        {
            Assert.IsType<string>(Yogi.Constants.DefaultSslDhParams);
            Assert.Contains("-----BEGIN DH PARAMETERS-----", Yogi.Constants.DefaultSslDhParams);
        }
    }
}
