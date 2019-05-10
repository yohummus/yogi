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
using System.Reflection;
using Xunit;

namespace test
{
    public class VersionTest : TestCase
    {
        [Fact]
        public void Version()
        {
            Assert.True(Yogi.Version.Length > 4);
        }

        [Fact]
        public void AssemblyVersion()
        {
            var v = Assembly.GetAssembly(typeof(Yogi)).GetName().Version;
            var s = string.Format("{0}.{1}.{2}", v.Major, v.Minor, v.Build);
            Assert.Equal(Yogi.Version, s);
        }
    }
}
