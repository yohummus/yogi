import sys
import os
import unittest
import subprocess
import time
import shutil

UUT_BINARY = os.path.dirname(os.path.abspath(__file__)) + '/../build/yogi-supervisor'
TMP_DIR    = '/tmp/yogi-supervisor'

class Fixture(unittest.TestCase):
    def setUp(self):
        shutil.rmtree(TMP_DIR, ignore_errors=True)
        self.supervisor = None

    def tearDown(self):
        if self.supervisor:
            self.supervisor.terminate()
            self.supervisor.wait(timeout=1.0)

    def start(self, json_file):
        self.supervisor = subprocess.Popen([UUT_BINARY, os.path.dirname(os.path.abspath(__file__)) + '/' + json_file])
        self.assertTrue(self.wait_for(lambda: os.path.exists(TMP_DIR)))

    def wait_for(self, predicate, timeout=1.0):
        t = 0.000
        while t < timeout:
            if predicate():
                return True
            t += 0.001
            time.sleep(0.001)
        return False
