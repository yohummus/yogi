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
    public class TimeTest : TestCase
    {
        [Fact]
        public void CurrentTime()
        {
            var start = Yogi.Timestamp.Now;
            var t = Yogi.CurrentTime;
            Assert.IsType<Yogi.Timestamp>(t);
            Assert.InRange(t, start, Yogi.Timestamp.Now);
        }
    }
}
