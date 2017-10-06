#!/usr/bin/env python3
from fixture import *

class TestServices(Fixture):
    def test_startup_command(self):
        self.start('service.json')

    def test_startup_failure(self):
        self.start('service_startup_failure.json', wait_for_startup_file=False)
        try:
            self.assertGreater(self.supervisor.wait(timeout=3.0), 0)
        except:
            self.fail('Process did not exit')

    def test_startup_timeout(self):
        self.start('service_startup_timeout.json', wait_for_startup_file=False)
        try:
            self.assertGreater(self.supervisor.wait(timeout=3.0), 0)
        except:
            self.fail('Process did not exit')

    def test_execution(self):
        self.start('service.json')
        file = TMP_DIR + '/executed'
        self.assertTrue(self.wait_for(lambda: os.path.exists(file)))

    def test_execution_restart(self):
        self.start('service.json')
        file = TMP_DIR + '/executed'
        self.assertTrue(self.wait_for(lambda: os.path.exists(file)))
        os.remove(file)
        self.assertTrue(self.wait_for(lambda: os.path.exists(file)))

    def test_log_file(self):
        self.start('service.json')
        file = TMP_DIR + '/fake-service.log'
        self.assertTrue(self.wait_for(lambda: os.path.exists(file)))
        self.assertTrue(self.wait_for(lambda: os.stat(file).st_size > 0))

    def test_watch_file(self):
        watched_file = TMP_DIR + '/watched'
        executed_file = TMP_DIR + '/executed'
        with open(watched_file, 'w') as f:
            pass
        self.start('service_watch_file.json')
        self.assertTrue(self.wait_for(lambda: os.path.exists(executed_file)))
        os.remove(executed_file)

        with open(watched_file, 'w') as f:
            f.write('Test')
        self.assertTrue(self.wait_for(lambda: os.path.exists(executed_file)))
        os.remove(executed_file)

        os.remove(watched_file)
        self.assertTrue(self.wait_for(lambda: os.path.exists(executed_file)))
        os.remove(executed_file)

        with open(watched_file, 'w') as f:
            f.write('Test')
        self.assertTrue(self.wait_for(lambda: os.path.exists(executed_file)))
        os.remove(executed_file)
        

if __name__ == '__main__':
    unittest.main()
