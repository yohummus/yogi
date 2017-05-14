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

    def test_log_file(self):
        self.start('service.json')
        file = TMP_DIR + '/fake-service.log'
        self.assertTrue(self.wait_for(lambda: os.path.exists(file)))
        self.assertTrue(self.wait_for(lambda: os.stat(file).st_size > 0))

    def test_watch_file(self):
        watched_file = '/tmp/yogi-supervisor.fake-service.watched'
        executed_file = TMP_DIR + '/fake-service.executed'
        with open(watched_file, 'w') as f:
            pass
        self.start('service_watch_file.json')
        self.assertTrue(self.wait_for(lambda: os.path.exists(executed_file)))
        os.remove(executed_file)
        with open(watched_file, 'w') as f:
            f.write('Test')
        self.assertTrue(self.wait_for(lambda: os.path.exists(executed_file)))

if __name__ == '__main__':
    unittest.main()
