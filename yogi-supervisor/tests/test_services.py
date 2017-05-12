#!/usr/bin/env python3
import sys
import os
import unittest
import subprocess
import time
import shutil

UUT_BINARY = os.path.dirname(os.path.abspath(__file__)) + '/../build/yogi-supervisor'
JSON_FILE  = os.path.dirname(os.path.abspath(__file__)) + '/../yogi-supervisor.json'
TMP_DIR    = '/tmp/yogi-supervisor'

class TestServices(unittest.TestCase):
    def setUp(self):
        shutil.rmtree(TMP_DIR, ignore_errors=True)
        self.supervisor = subprocess.Popen([UUT_BINARY, JSON_FILE])
        self.assertTrue(self.wait_for(lambda: os.path.exists(TMP_DIR)))
        self.assertIsNone(self.supervisor.poll())

    def tearDown(self):
        self.supervisor.terminate()
        self.supervisor.wait(timeout=1.0)

    def test_startup_command(self):
        self.assertTrue(self.wait_for(lambda: os.path.exists(TMP_DIR + '/fake-service.started')))

    def wait_for(self, predicate, timeout=1.0):
        t = 0.000
        while t < timeout:
            if predicate():
                return True
            t += 0.001
            time.sleep(0.001)
        return False



if __name__ == '__main__':
    unittest.main()
