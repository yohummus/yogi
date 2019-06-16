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
using System.Net;
using Xunit;

namespace test
{
    public class TimerTest : TestCase
    {
        Yogi.Context context = new Yogi.Context();

        [Fact]
        public void Start()
        {
            Yogi.Timer timer = new Yogi.Timer(context);

            bool called = false;
            timer.StartAsync(Yogi.Duration.FromMilliseconds(1), (res) => {
                Assert.IsType<Yogi.Success>(res);
                Assert.Equal(Yogi.ErrorCode.Ok, res.ErrorCode);
                called = true;
            });

            GC.Collect();

            while (!called)
            {
                context.RunOne();
            }

            Assert.True(called);

            GC.KeepAlive(timer);
        }

        [Fact]
        public void Cancel()
        {
            Yogi.Timer timer = new Yogi.Timer(context);

            Assert.False(timer.Cancel());

            bool called = false;
            timer.StartAsync(Yogi.Duration.Infinity, (res) => {
                Assert.IsType<Yogi.Failure>(res);
                Assert.Equal(Yogi.ErrorCode.Canceled, res.ErrorCode);
                called = true;
            });

            GC.Collect();
            Assert.True(timer.Cancel());
            GC.Collect();

            while (!called)
            {
                context.RunOne();
            }

            Assert.True(called);

            GC.KeepAlive(timer);
        }
    }
}
