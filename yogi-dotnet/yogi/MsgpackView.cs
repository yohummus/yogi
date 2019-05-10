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
using System.Text;
using System.Linq;
using MessagePack;

public static partial class Yogi
{
    /// <summary>
    /// Helper class for passing different types of MessagePack data to functions.
    /// </summary>
    public class MsgpackView
    {
        /// <summary>
        /// Constructs a view from a buffer containing serialized MessagePack data.
        /// </summary>
        /// <param name="data">Buffer containing serialized MessagePack data.</param>
        public MsgpackView(byte[] data)
        {
            Data = data;
        }

        public static implicit operator MsgpackView(byte[] data)
        {
            return new MsgpackView(data);
        }

        public static bool operator ==(MsgpackView lhs, MsgpackView rhs)
        {
            if (object.ReferenceEquals(lhs, null))
            {
                return object.ReferenceEquals(rhs, null);
            }

            return lhs.Equals(rhs);
        }

        public static bool operator !=(MsgpackView lhs, MsgpackView rhs)
        {
            return !(lhs == rhs);
        }

        public override bool Equals(object obj)
        {
            if (!(obj is MsgpackView))
            {
                return false;
            }

            return Data.SequenceEqual(((MsgpackView)obj).Data);
        }

        public override int GetHashCode()
        {
            return Data.GetHashCode();
        }

        /// <summary>
        /// Buffer holding the serialized MessagePack data.
        /// </summary>
        public byte[] Data { get; }

        /// <summary>
        /// Length of the serialized MessagePack data in bytes.
        /// </summary>
        public int Size
        {
            get { return Data == null ? 0 : Data.Length; }
        }
    }
}
