from abc import *
from gimpfu import *
from os.path import dirname, basename, isfile
from glob import glob
import re


class NodeOption(object):
    """
    Represents a user-settable option on a node.
    """
    __metaclass__ = ABCMeta

    @classmethod
    def make_options(cls, node):
        """
        Extracts all node options for a node.

        Args:
            node (LayerNode): Node to create the node options for.

        Returns:
            NodeOption[]: Node options set for the given node.
        """
        cls._import_all_node_options()

        options = []
        for option in cls.__subclasses__():
            regex = r'^{}.* {}((\((.*)\))| |$).*$'.format(node.name, option.__name__.upper())
            match = re.match(regex, node.gimp_obj.name)
            if match:
                args = cls._parse_args(match.group(3))
                options.append(option(node, args))

        # gimp.message(node.name + str(options))
        return options

    @abstractmethod
    def __init__(self, node, args):
        pass

    @staticmethod
    def _import_all_node_options():
        # imports all options due to node_options/__init__.py
        from node_options import *

    @staticmethod
    def _parse_args(string):
        args = []
        if string:
            parts = [x.strip() for x in string.split(',')]
            for part in parts:
                if part.startswith('"') and part.endswith('"'):
                    args.append(part[1:-1])
                elif '.' in part:
                    args.append(float(part))
                else:
                    args.append(int(part))

        return args
