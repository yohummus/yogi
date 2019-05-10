# This file is part of the Yogi distribution https://github.com/yohummus/yogi.
# Copyright (c) 2018 Johannes Bergmann.
#
# This library is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this library. If not, see <http://www.gnu.org/licenses/>.

import yogi
import unittest
import datetime

from .common import TestCase


class TestTime(TestCase):
    def test_get_current_time(self):
        dt = yogi.get_current_time().to_datetime()
        now = datetime.datetime.now(datetime.timezone.utc)
        delta = datetime.timedelta(seconds=1)
        self.assertGreater(dt, now - delta)
        self.assertLess(dt, now + delta)


if __name__ == '__main__':
    unittest.main()
