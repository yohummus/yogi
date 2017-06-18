from os.path import dirname, basename, isfile
from glob import glob


# import all modules in this package
modules = []
for file in glob(dirname(__file__) + '/*.py'):
    if isfile(file) and not file.endswith('__init__.py'):
        module = basename(file)[:-3]
        modules.append(module)

__all__ = modules
