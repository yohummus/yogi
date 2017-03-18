from .path import Path
from typing import *
import argparse
import json
import glob


class BadCommandLine(Exception):
    def __init__(self, description: str):
        self._description = description

    def __str__(self):
        return 'Cannot parse command line: {}'.format(self._description)


class BadConfiguration(Exception):
    def __init__(self, description: str):
        self._description = description

    def __str__(self):
        return 'Cannot parse configuration: {}'.format(self._description)


class Configuration:
    def __init__(self, argv: Optional[List[str]] = None):
        self._config = {}
        self.update('''
        {
            "yogi": {
                "location": "/",
                "connection": {
                    "target": null,
                    "timeout": null,
                    "identification": null
                }
            }
        }
        ''')

        if argv:
            self._parse_cmdline(argv)

    def __str__(self):
        return str(self._config)

    @property
    def config(self) -> DefaultDict:
        return self._config

    @property
    def location(self) -> Path:
        return Path(self._config['yogi']['location'])

    @property
    def connection_target(self) -> Optional[str]:
        return self._config['yogi']['connection']['target']

    @property
    def connection_timeout(self) -> Optional[float]:
        return self._config['yogi']['connection']['timeout']

    @property
    def connection_identification(self) -> Optional[str]:
        return self._config['yogi']['connection']['identification']

    def update(self, jsonStr: str) -> None:
        try:
            def merge(a, b, path=None):
                if path is None:
                    path = []
                for key in b:
                    if key in a:
                        if isinstance(a[key], dict) and isinstance(b[key], dict):
                            merge(a[key], b[key], path + [str(key)])
                        else:
                            a[key] = b[key]
                    else:
                        a[key] = b[key]
                return a

            self._config = merge(self._config, json.loads(jsonStr))
        except Exception as e:
            raise BadConfiguration(str(e))

    def _parse_cmdline(self, argv):
        class ThrowingArgumentParser(argparse.ArgumentParser):
            def __init__(self):
                argparse.ArgumentParser.__init__(self)

            def error(self, message):
                raise BadCommandLine(message)

        parser = ThrowingArgumentParser()
        parser.add_argument('--connection_target', '-c', dest='target', type=str, metavar='host:port',
                            help='YOGI server to connect to (e.g. "hostname:12000")')
        parser.add_argument('--connection_timeout', '-t', dest='timeout', type=float, metavar='seconds',
                            help='Connection timeout in seconds (-1 for infinity)')
        parser.add_argument('--connection_identification', '-i', dest='identification', type=str, metavar='string',
                            help='Identification for YOGI connections')
        parser.add_argument('--location', '-l', dest='location', type=str, metavar='path',
                            help='Location of the terminals for this process in the YOGI terminal tree')
        parser.add_argument('--json', '-j', dest='json_overrides', type=str, metavar='JSON', action='append',
                            help='Configuration overrides (in JSON format, e.g. \'{ "my-age": 42 }\')')
        parser.add_argument('config_files', metavar='config.json', nargs='*',
                            help='Configuration files in JSON format')
        pargs = parser.parse_args(argv)

        for pattern in pargs.config_files:
            for filename in glob.glob(pattern):
                with open(filename, 'r') as file:
                    self.update(file.read())

        if pargs.json_overrides:
            for json_str in pargs.json_overrides:
                self.update(json_str)

        if pargs.location:
            self.update('{"yogi": {"location": "' + pargs.location + '"}}')
        if pargs.target:
            self.update('{"yogi": {"connection": {"target": "' + pargs.target + '"}}}')
        if pargs.timeout:
            self.update('{"yogi": {"connection": {"timeout": ' + str(pargs.timeout) + '}}}')
        if pargs.identification:
            self.update('{"yogi": {"connection": {"identification": "' + pargs.identification + '"}}}')
