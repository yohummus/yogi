#!/usr/bin/env python3
import sys
import setuptools

assert sys.version_info >= (3, 5), "YOGI for Python requires Python version 3.5 or higher. This is version {}.{}." \
    .format(sys.version_info.major, sys.version_info.minor)

config = {
    'description'     : 'Python bindings for the YOGI framework',
    'author'          : 'Johannes Bergmann',
    'author_email'    : 'mail@johannes-bergmann.de',
    'url'             : 'https://github.com/jmbergmann/yogi',
    'download_url'    : 'https://github.com/jmbergmann/yogi',
    'version'         : '0.0.1',
    'packages'        : setuptools.find_packages('.', exclude=['tests', 'tests.*']),
    'scripts'         : [
    ],
    'name'            : 'yogi-python',
    'license'         : 'GPLv3',
    'keywords'        : 'yogi',
    'test_suite'      : 'tests',
    'classifiers'     : [
        'Development Status :: 3 - Alpha',
        'License :: OSI Approved :: GPLv3',
        'Intended Audience :: Developers',
        'Topic :: Software Development :: Libraries',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6'
    ]
}

setuptools.setup(**config)
