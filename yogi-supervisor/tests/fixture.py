import sys
import os
import unittest
import subprocess
import time

UUT_BINARY   = os.path.dirname(os.path.abspath(__file__)) + '/../build/yogi-supervisor'
TMP_DIR      = '/tmp/yogi-supervisor'
STARTED_FILE = TMP_DIR + '/started'

class Fixture(unittest.TestCase):
    def setUp(self):
        subprocess.call(['rm', '-rf', TMP_DIR])
        os.mkdir(TMP_DIR)
        self.supervisor = None

    def tearDown(self):
        if self.supervisor:
            self.supervisor.terminate()
            self.supervisor.wait(timeout=1.0)

    def start(self, json_file, wait_for_startup_file=True):
        this_dir = os.path.dirname(os.path.abspath(__file__)) + '/';
        self.supervisor = subprocess.Popen([UUT_BINARY, this_dir + 'common.json', this_dir + json_file])
        if wait_for_startup_file:
            self.assertTrue(self.wait_for(lambda: os.path.exists(STARTED_FILE)))

    def wait_for(self, predicate, timeout=1.0):
        t = 0.000
        while t < timeout:
            if predicate():
                return True
            t += 0.001
            time.sleep(0.001)
        return False
