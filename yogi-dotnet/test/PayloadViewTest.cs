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
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace test
{
    public class PayloadViewTest : TestCase
    {
        [Fact]
        public void FromBuffer()
        {
            var data = new byte[] { 1, 2, 3, 4, 0 };

            var view = new Yogi.PayloadView(data, 3, Yogi.EncodingType.Msgpack);
            Assert.Equal(view.Data, data);
            Assert.Equal(3, view.Size);
            Assert.Equal(Yogi.EncodingType.Msgpack, view.Encoding);

            view = new Yogi.PayloadView(data, Yogi.EncodingType.Json);
            Assert.Equal(view.Data, data);
            Assert.Equal(view.Size, data.Length);
            Assert.Equal(Yogi.EncodingType.Json, view.Encoding);
        }

        [Fact]
        public void FromJsonView()
        {
            var json_view = new Yogi.JsonView("Hello");

            var view = new Yogi.PayloadView(json_view);
            Assert.Equal(view.Data, json_view.Data);
            Assert.Equal(view.Size, json_view.Size);
            Assert.Equal(Yogi.EncodingType.Json, view.Encoding);
        }

        [Fact]
        public void FromMsgpackView()
        {
            var msgpack_view = new Yogi.MsgpackView(new byte[] { 1, 2, 3 });

            var view = new Yogi.PayloadView(msgpack_view);
            Assert.Equal(view.Data, msgpack_view.Data);
            Assert.Equal(view.Size, msgpack_view.Size);
            Assert.Equal(Yogi.EncodingType.Msgpack, view.Encoding);
        }

        [Fact]
        public void Comparison()
        {
            var view1 = new Yogi.PayloadView(new Yogi.JsonView("Hello"));
            var view2 = new Yogi.PayloadView(new Yogi.JsonView("Hello"));
            var view3 = new Yogi.PayloadView(new Yogi.JsonView("World"));
            var view4 = new Yogi.PayloadView(view1.Data, Yogi.EncodingType.Msgpack);

            Assert.True(view1 == view2);
            Assert.False(view1 == view3);
            Assert.False(view1 == view4);

            Assert.False(view1 != view2);
            Assert.True(view1 != view3);
            Assert.True(view1 != view4);

            Assert.False(view1.Equals(new Exception()));
            Assert.False(view1.Equals(null));
            Assert.True(view1.Equals(view2));
            Assert.False(view1.Equals(view3));
            Assert.False(view1.Equals(view4));

            var view5 = new Yogi.PayloadView(new byte[] { 1, 2, 0 }, Yogi.EncodingType.Json);
            var view6 = new Yogi.PayloadView(new byte[] { 1, 2, 0, 9 }, 3, Yogi.EncodingType.Json);
            Assert.True(view5.Equals(view6));
        }

        [Fact]
        public void GetHashCodeMethod()
        {
            var view1 = new Yogi.PayloadView(new Yogi.JsonView("Hello"));
            var view2 = new Yogi.PayloadView(new Yogi.JsonView("World"));

            Assert.NotEqual(view1.GetHashCode(), view2.GetHashCode());
        }
    }
}
