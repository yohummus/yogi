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
    public class LicensesTest : TestCase
    {
        [Fact]
        public void License()
        {
            var lic = Yogi.License;
            Assert.IsType<string>(lic);
            Assert.True(lic.Length > 100);
        }

        [Fact]
        public void ThirdPartyLicenses()
        {
            var lic = Yogi.ThirdPartyLicenses;
            Assert.IsType<string>(lic);
            Assert.True(lic.Length > 100);
            Assert.NotEqual(Yogi.License, lic);
        }
    }
}
