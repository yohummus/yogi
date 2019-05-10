#!/usr/bin/env python3
import os
import re
import setuptools

pkg_dir = os.path.dirname(os.path.realpath(__file__))
req_file = pkg_dir + '/requirements.txt'
core_hdr_file = pkg_dir + '/../yogi-core/include/yogi_core.h'

requirements = []
with open(req_file) as f:
    requirements = f.read().splitlines()

for _, line in enumerate(open(core_hdr_file)):
    if line.startswith('#define YOGI_HDR_VERSION '):
        version = re.search('"(.*)"', line).group(1)

config = {
    'description': 'Python bindings for the Yogi framework',
    'author': 'Johannes Bergmann',
    'author_email': 'j-bergmann@outlook.com',
    'url': 'https://github.com/jmbergmann/yogi',
    'download_url': 'https://github.com/jmbergmann/yogi',
    'version': version,
    'packages': setuptools.find_packages('.', exclude=['test', 'test.*']),
    'name': 'yogi-python',
    'license': 'GPLv3',
    'keywords': 'yogi decoupling networking',
    'test_suite': 'test',
    'python_requires': '>=3.5',
    'install_requires': requirements,
    'classifiers': [
        'Development Status :: 3 - Alpha',
        'License :: OSI Approved :: GPLv3',
        'Intended Audience :: Developers',
        'Topic :: Software Development :: Libraries'
    ]
}

setuptools.setup(**config)
