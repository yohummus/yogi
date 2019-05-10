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
using System.Net;
using Xunit;

namespace test
{
    public class SignalsTest : TestCase
    {
        Yogi.Context context = new Yogi.Context();

        [Fact]
        public void SignalsEnum()
        {
            foreach (var elem in GetEnumElements<Yogi.Signals>())
            {
                if (elem == Yogi.Signals.All)
                {
                    AssertFlagCombinationMatches("YOGI_SIG_", elem);
                }
                else
                {
                    AssertFlagMatches("YOGI_SIG_", elem);
                }
            }
        }

        [Fact]
        public void RaiseSignalCleanupHandler()
        {
            bool called = false;
            Yogi.RaiseSignal(Yogi.Signals.Usr1, () => {
                called = true;
            });
            Assert.True(called);

            called = false;
            Yogi.RaiseSignal<string>(Yogi.Signals.Usr2, null, (sigarg) => {
                Assert.Null(sigarg);
                called = true;
            });
            Assert.True(called);

            called = false;
            Yogi.RaiseSignal(Yogi.Signals.Usr2, "Hello", (sigarg) => {
                Assert.Equal("Hello", sigarg);
                called = true;
            });
            Assert.True(called);
        }

        [Fact]
        public void AwaitSignal()
        {
            var sigset = new Yogi.SignalSet(context, Yogi.Signals.Term | Yogi.Signals.Usr5);

            bool called = false;
            sigset.AwaitSignalAsync((res, signal) => {
                Assert.IsType<Yogi.Success>(res);
                Assert.Equal(Yogi.ErrorCode.Ok, res.ErrorCode);
                Assert.Equal(Yogi.Signals.Term, signal);
                called = true;
            });
            GC.Collect();
            Yogi.RaiseSignal(Yogi.Signals.Term, "123");
            GC.Collect();
            context.Poll();
            GC.Collect();
            Assert.True(called);

            called = false;
            sigset.AwaitSignalAsync<string>((res, signal, sigarg) => {
                Assert.IsType<Yogi.Success>(res);
                Assert.Equal(Yogi.ErrorCode.Ok, res.ErrorCode);
                Assert.Equal(Yogi.Signals.Term, signal);
                Assert.Null(sigarg);
                called = true;
            });
            Yogi.RaiseSignal(Yogi.Signals.Term);
            context.Poll();
            Assert.True(called);

            called = false;
            sigset.AwaitSignalAsync<string>((res, signal, sigarg) => {
                Assert.IsType<Yogi.Success>(res);
                Assert.Equal(Yogi.ErrorCode.Ok, res.ErrorCode);
                Assert.Equal(Yogi.Signals.Term, signal);
                Assert.Equal("Hello", sigarg);
                called = true;
            });
            Yogi.RaiseSignal(Yogi.Signals.Term, "Hello");
            context.Poll();
            Assert.True(called);
        }

        [Fact]
        public void CancelAwaitSignal()
        {
            var sigset = new Yogi.SignalSet(context, Yogi.Signals.Term);

            bool called = false;
            sigset.AwaitSignalAsync<string>((res, signal, sigarg) => {
                Assert.IsType<Yogi.Failure>(res);
                Assert.Equal(Yogi.ErrorCode.Canceled, res.ErrorCode);
                Assert.IsType<Yogi.Signals>(signal);
                Assert.Equal(Yogi.Signals.None, signal);
                Assert.Null(sigarg);
                called = true;
            });
            GC.Collect();
            sigset.CancelAwaitSignal();
            GC.Collect();
            context.Poll();
            Assert.True(called);
        }
    }
}
