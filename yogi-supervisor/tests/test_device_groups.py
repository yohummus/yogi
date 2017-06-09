#!/usr/bin/env python3
from fixture import *

class TestDeviceGroups(Fixture):
    def test_startup_command(self):
        self.start('device_group.json')

    def test_startup_failure(self):
        self.start('device_group_startup_failure.json', wait_for_startup_file=False)
        try:
            self.assertGreater(self.supervisor.wait(timeout=3.0), 0)
        except:
            self.fail('Process did not exit')

    def test_startup_timeout(self):
        self.start('device_group_startup_timeout.json', wait_for_startup_file=False)
        try:
            self.assertGreater(self.supervisor.wait(timeout=3.0), 0)
        except:
            self.fail('Process did not exit')

    def test_validation(self):
        self.start('device_group.json')
        self.assertTrue(self.wait_for(lambda: os.path.exists(TMP_DIR + '/pre-executed.dev2')))
        time.sleep(0.1)
        self.assertFalse(os.path.exists(TMP_DIR + '/pre-executed.dev1'))

    def test_validation_timeout(self):
        self.start('device_group_validation_timeout.json')
        self.assertFalse(self.wait_for(lambda: os.path.exists(TMP_DIR + '/pre-executed.dev2'), timeout=0.1))

    def test_pre_execution(self):
        self.start('device_group.json')
        self.assertTrue(self.wait_for(lambda: os.path.exists(TMP_DIR + '/pre-executed.dev2')))

    def test_pre_execution_failure(self):
        self.start('device_group_pre_execution_failure.json')
        self.assertFalse(self.wait_for(lambda: os.path.exists(TMP_DIR + '/executed.dev2'), timeout=0.1))

    def test_pre_execution_timeout(self):
        self.start('device_group_pre_execution_timeout.json')
        self.assertFalse(self.wait_for(lambda: os.path.exists(TMP_DIR + '/executed.dev2'), timeout=0.1))

    def test_restart_pre_execution(self):
        self.start('device_group_restart_pre_execution.json')
        file = TMP_DIR + '/pre-executed.dev2'
        self.assertTrue(self.wait_for(lambda: os.path.exists(file)))
        os.remove(file)
        self.assertTrue(self.wait_for(lambda: os.path.exists(file)))
        self.assertFalse(os.path.exists(TMP_DIR + '/executed.dev2'))

    def test_restart_execution(self):
        self.start('device_group_restart_execution.json')
        fileA = TMP_DIR + '/pre-executed.dev2'
        fileB = TMP_DIR + '/executed.dev2'
        self.assertTrue(self.wait_for(lambda: os.path.exists(fileA)))
        self.assertTrue(self.wait_for(lambda: os.path.exists(fileB)))
        os.remove(fileA)
        os.remove(fileB)
        self.assertTrue(self.wait_for(lambda: os.path.exists(fileA)))
        self.assertTrue(self.wait_for(lambda: os.path.exists(fileB)))

    def test_log_file(self):
        self.start('device_group.json')
        file = TMP_DIR + '/log.dev2'
        self.assertTrue(self.wait_for(lambda: os.path.exists(file)))
        self.assertTrue(self.wait_for(lambda: os.stat(file).st_size > 0))

    def test_watch_file(self):
        watched_file = TMP_DIR + '/watched'
        executed_file = TMP_DIR + '/executed.dev2'
        with open(watched_file, 'w') as f:
            pass
        self.start('device_group_watch_file.json')
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
