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
import json
import tempfile
import shutil
import os.path

from .common import TestCase


class TestConfigurations(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp_dir = tempfile.mkdtemp()

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.temp_dir)

    def test_configuration_options_enum(self):
        self.assertEnumMatches("YOGI_CFG_", yogi.ConfigurationFlags)

    def test_command_line_options_enum(self):
        for opt in yogi.CommandLineOptions:
            if opt is yogi.CommandLineOptions.ALL:
                self.assertFlagCombinationMatches("YOGI_CLO_", opt, [])
            elif opt is yogi.CommandLineOptions.BRANCH_ALL:
                exc = [x for x in yogi.CommandLineOptions
                       if not x.name.startswith("BRANCH_")]
                self.assertFlagCombinationMatches("YOGI_CLO_", opt, exc)
            else:
                self.assertFlagMatches("YOGI_CLO_", opt)

    def test_create_from_json(self):
        cfg = yogi.Configuration(json={"age": 42})
        jsn = json.loads(cfg.dump())
        self.assertEqual(jsn["age"], 42)

    def test_flags_property(self):
        cfg = yogi.Configuration(yogi.ConfigurationFlags.MUTABLE_CMD_LINE)
        self.assertEqual(cfg.flags, yogi.ConfigurationFlags.MUTABLE_CMD_LINE)

    def test_update_from_command_line(self):
        cfg = yogi.Configuration()
        cfg.update_from_command_line(
            ["exe", "-o", '{"age": 25}'], yogi.CommandLineOptions.OVERRIDES)

        jsn = json.loads(cfg.dump())
        self.assertEqual(jsn["age"], 25)

    def test_update_from_json(self):
        cfg = yogi.Configuration()

        cfg.update_from_json({"age": 42})
        jsn = json.loads(cfg.dump())
        self.assertEqual(jsn["age"], 42)

        cfg.update_from_json(yogi.JsonView({"age": 33}))
        jsn = json.loads(cfg.dump())
        self.assertEqual(jsn["age"], 33)

    def test_update_from_file(self):
        filename = os.path.join(self.temp_dir, "cfg.json")
        with open(filename, "w") as f:
            f.write('{"age": 66}')

        cfg = yogi.Configuration()
        cfg.update_from_file(filename)

        jsn = json.loads(cfg.dump())
        self.assertEqual(jsn["age"], 66)

    def test_dump(self):
        cfg = yogi.Configuration(yogi.ConfigurationFlags.DISABLE_VARIABLES)
        cfg.update_from_json({"age": 42})

        self.assertRaises(yogi.FailureException, lambda: cfg.dump(True))

        self.assertFalse(" " in cfg.dump())
        self.assertFalse("\n" in cfg.dump())
        self.assertTrue(" " in cfg.dump(indentation=2))
        self.assertTrue("\n" in cfg.dump(indentation=2))

    def test_to_json(self):
        cfg = yogi.Configuration(yogi.ConfigurationFlags.DISABLE_VARIABLES)
        cfg.update_from_json({"age": 42})

        self.assertRaises(yogi.FailureException, lambda: cfg.to_json(True))

        jsn = cfg.to_json()
        self.assertEqual(jsn["age"], 42)

    def test_write_to_file(self):
        filename = os.path.join(self.temp_dir, "dump.json")

        cfg = yogi.Configuration(yogi.ConfigurationFlags.DISABLE_VARIABLES)
        cfg.update_from_json({"age": 11})

        self.assertRaises(yogi.FailureException, lambda: cfg.write_to_file(
            filename, True))

        cfg.write_to_file(filename)
        with open(filename) as f:
            content = f.read()
            self.assertFalse(" " in content)
            self.assertFalse("\n" in content)

            jsn = json.loads(content)
            self.assertEqual(jsn["age"], 11)

        cfg.write_to_file(filename, indentation=2)
        with open(filename) as f:
            content = f.read()
            self.assertTrue(" " in content)
            self.assertTrue("\n" in content)

            jsn = json.loads(content)
            self.assertEqual(jsn["age"], 11)

    def test_validate(self):
        cfg = yogi.Configuration(json='''
            {
              "person": {
              "name": "Joe",
              "age": 42
              }
            }
            ''')

        scm = yogi.Configuration(json='''
            {
              "$schema": "http://json-schema.org/draft-07/schema#",
              "title": "Test schema",
              "properties": {
                "name": {
                  "description": "Name",
                  "type": "string"
                },
                "age": {
                  "description": "Age of the person",
                  "type": "number",
                  "minimum": 2,
                  "maximum": 200
                }
              },
              "required": ["name", "age"],
              "type": "object"
            }
            ''')

        cfg.validate(scm, section="/person")

        try:
            cfg.validate(scm)
            self.fail('No exception thrown')
        except yogi.DescriptiveFailureException as e:
            self.assertEqual(e.failure.error_code,
                             yogi.ErrorCode.CONFIGURATION_VALIDATION_FAILED)
            self.assertTrue("not found" in str(e))


if __name__ == '__main__':
    unittest.main()
