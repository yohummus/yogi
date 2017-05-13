#!/usr/bin/env python3
from fixture import *

class TestServices(Fixture):
    def test_startup_command(self):
        self.start('service.json')
        self.assertTrue(self.wait_for(lambda: os.path.exists(TMP_DIR + '/fake-service.started')))

    def test_startup_failure(self):
        self.start('service_startup_failure.json')
        try:
            self.assertGreater(self.supervisor.wait(timeout=3.0), 0)
        except:
            self.fail('Process did not exit')

    def test_startup_timeout(self):
        self.start('service_startup_timeout.json')
        try:
            self.assertGreater(self.supervisor.wait(timeout=3.0), 0)
        except:
            self.fail('Process did not exit')

    def test_execution_command(self):
        self.start('service.json')
        file = TMP_DIR + '/fake-service.executed'
        self.assertTrue(self.wait_for(lambda: os.path.exists(file)))

    def test_execution_command_restart(self):
        self.start('service.json')
        file = TMP_DIR + '/fake-service.executed'
        self.assertTrue(self.wait_for(lambda: os.path.exists(file)))
        os.remove(file)
        self.assertTrue(self.wait_for(lambda: os.path.exists(file)))

if __name__ == '__main__':
    unittest.main()
