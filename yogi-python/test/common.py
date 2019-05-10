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

import sys
import os.path
import re
import time
import unittest
import yogi

from typing import Union, List

sys.stderr = sys.stdout

yogi_core_h = None  # type: str

with open(os.path.join(os.path.dirname(__file__),
                       "../../yogi-core/include/yogi_core.h")) as f:
    yogi_core_h = f.read()


class TestCase(unittest.TestCase):
    def get_core_macro(self, macro_name: str) -> Union[str, int]:
        regex = re.compile("#define {} ([^ \\s]*).*".format(macro_name))
        m = re.search(regex, yogi_core_h)
        if not m:
            raise Exception(
                "Macro {} not found in yogi_core.h".format(macro_name))

        if m.group(1).startswith('('):
            regex = re.compile("#define {} (\\(.+\\)).*".format(macro_name))
            m = re.search(regex, yogi_core_h)
            if not m:
                raise Exception(
                    "Cannot parse macro {} in yogi_core.h".format(macro_name))

        return eval(m.group(1))

    def run_context_until_branches_are_connected(self, context: yogi.Context,
                                                 branches: List[yogi.Branch]
                                                 ) -> None:
        uuids = {}
        for branch in branches:
            uuids[branch] = [x.uuid for x in branches if x is not branch]

        start = time.time()

        while uuids:
            context.poll()

            branch = next(iter(uuids))
            for uuid in branch.get_connected_branches():
                if uuid in uuids[branch]:
                    uuids[branch].remove(uuid)

            if not uuids[branch]:
                del uuids[branch]

            if time.time() - start > 3:
                print(uuids)
                raise TimeoutError("Branches did not connect")

    def assertEnumElementMatches(self, macro_prefix, elem):
        macro_name = macro_prefix + elem.name
        macro_val = self.get_core_macro(macro_name)
        self.assertEqual(elem, macro_val)

    def assertEnumMatches(self, macro_prefix, enum):
        for elem in enum:
            self.assertEnumElementMatches(macro_prefix, elem)

    def assertFlagMatches(self, macro_prefix, enum_elem):
        self.assertEnumElementMatches(macro_prefix, enum_elem)

    def assertFlagCombinationMatches(self, macro_prefix, enum_elem,
                                     exceptions):
        macro_name = macro_prefix + enum_elem.name
        regex = re.compile("#define {} .*".format(macro_name))
        macro_found = bool(re.search(regex, yogi_core_h))
        self.assertTrue(macro_found)

        expected = 0
        for ev in type(enum_elem):
            if ev is not enum_elem and ev not in exceptions:
                expected |= ev
        self.assertEqual(enum_elem, expected)
