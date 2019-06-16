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
using System.Text;
using System.Linq;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

public static partial class Yogi
{
    /// <summary>
    /// Helper class for passing different types of JSON to functions.
    /// </summary>
    public class JsonView
    {
        /// <summary>
        /// Constructs a view from a string.
        /// </summary>
        /// <param name="s">String containing serialized JSON.</param>
        public JsonView(string s)
        {
            Data = System.Text.Encoding.UTF8.GetBytes(s + '\0');
        }

        public static implicit operator JsonView(string s)
        {
            return new JsonView(s);
        }

        /// <summary>
        /// Constructs a view from a JObject.
        /// </summary>
        /// <param name="json">The JSON object.</param>
        public JsonView(JObject json)
        : this(JsonConvert.SerializeObject(json))
        {
        }

        public static implicit operator JsonView(JObject json)
        {
            return new JsonView(json);
        }

        /// <summary>
        /// Constructs a view from a JArray.
        /// </summary>
        /// <param name="json">The JSON array.</param>
        public JsonView(JArray json)
        : this(JsonConvert.SerializeObject(json))
        {
        }

        public static implicit operator JsonView(JArray json)
        {
            return new JsonView(json);
        }

        public static bool operator ==(JsonView lhs, JsonView rhs)
        {
            if (object.ReferenceEquals(lhs, null))
            {
                return object.ReferenceEquals(rhs, null);
            }

            return lhs.Equals(rhs);
        }

        public static bool operator !=(JsonView lhs, JsonView rhs)
        {
            return !(lhs == rhs);
        }

        public override bool Equals(object obj)
        {
            if (!(obj is JsonView))
            {
                return false;
            }

            return Data.SequenceEqual(((JsonView)obj).Data);
        }

        public override int GetHashCode()
        {
            return Data.GetHashCode();
        }

        /// <summary>
        /// Buffer holding the serialized JSON data, including the trailing '\0.
        /// </summary>
        public byte[] Data { get; }

        /// <summary>
        /// Length of the serialized JSON data in bytes, including the trailing '\0'.
        /// </summary>
        public int Size
        {
            get { return Data == null ? 0 : Data.Length; }
        }
    }
}
