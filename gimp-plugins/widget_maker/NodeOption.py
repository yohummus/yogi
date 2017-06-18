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
        for tup in cls._make_options_as_tuples(node):
            option_class = cls._make_option_class_from_name(node, tup[1])
            args = cls._parse_args(node, tup[1], tup[3])
            options.append(option_class(node, args))

        return options

    @abstractmethod
    def __init__(self, node, args):
        pass

    @staticmethod
    def _import_all_node_options():
        # imports all options due to node_options/__init__.py
        from node_options import *

    @staticmethod
    def _make_options_as_tuples(node):
        string = node.gimp_obj.name[len(node.name):].strip()

        if not re.match(r'^( *[a-zA-Z]+ *(\(.*\))?)*$', string):
            raise Exception('Invalid options syntax in layer "{}": "{}"'.format(node.name, string))

        tuples = re.findall(r'(([a-zA-Z]+) *(\((.*)\))?)', string)
        # [0] is whole option string, e.g. 'MOVABLE(-10, 20)'
        # [1] is option name, e.g. 'MOVABLE'
        # [2] is option parameters with brackets, e.g. '(-10, 20)'
        # [3] is option parameters without brackets, e.g. '-10, 20'
        return tuples

    @classmethod
    def _make_option_class_from_name(cls, node, option_name):
        for option_class in cls.__subclasses__():
            if option_class.__name__.lower() == option_name.lower():
                return option_class

        raise Exception('Unknown Option "{}" in layer "{}"'.format(option_name, node.name))

    @staticmethod
    def _parse_args(node, option_name, string):
        args = []
        if string:
            parts = [x.strip() for x in string.split(',')]
            for part in parts:
                try:
                    if part.startswith('"') and part.endswith('"'):
                        args.append(part[1:-1])
                    elif '.' in part:
                        args.append(float(part))
                    else:
                        args.append(int(part))
                except Exception as e:
                    raise Exception('Could not parse arguments for node option "{}" in layer "{}": {}'
                                    .format(option_name, node.name, e))

        return args
