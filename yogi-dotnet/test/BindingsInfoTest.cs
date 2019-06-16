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

using System;
using Xunit;

namespace test
{
    public class BindingsInfoTest : TestCase
    {
        [Fact]
        public void Version()
        {
            Assert.Equal(GetCoreMacroString("YOGI_HDR_VERSION"), Yogi.BindingsInfo.Version);
        }

        [Fact]
        public void VersionMajor()
        {
            Assert.Equal(GetCoreMacroInt("YOGI_HDR_VERSION_MAJOR"), Yogi.BindingsInfo.VersionMajor);
        }

        [Fact]
        public void VersionMinor()
        {
            Assert.Equal(GetCoreMacroInt("YOGI_HDR_VERSION_MINOR"), Yogi.BindingsInfo.VersionMinor);
        }

        [Fact]
        public void VersionPatch()
        {
            Assert.Equal(GetCoreMacroInt("YOGI_HDR_VERSION_PATCH"), Yogi.BindingsInfo.VersionPatch);
        }

        [Fact]
        public void VersionSuffix()
        {
            Assert.Equal(GetCoreMacroString("YOGI_HDR_VERSION_SUFFIX"),
                         Yogi.BindingsInfo.VersionSuffix);
        }
    }
}
