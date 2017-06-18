from NodeOption import *

from gimpfu import *
import re
import os.path


class LayerNode(object):
    """
    Represents a node in the hierarchy of an image's layers.

    The root LayerNode represents the whole image and contains an arbitrary number of child
    LayerNodes which represent the top-most layers or layer groups in the image. The children of
    those nodes represent the next lower layers or layer groups and so forth.
    """

    @classmethod
    def make_root_node(cls, image):
        """
        Creates the root node of an image.

        Args:
            image (gimp.Image): The image for which the root node will be created.

        Returns:
            LayerNode: Node representing the root of the image.
        """
        root = cls._make_node(None, image)
        cls._recursively_create_children(root)
        return root

    @property
    def parent(self):
        """LayerNode: Parent of this node."""
        return self._parent

    @property
    def is_root(self):
        """bool: True if this node is the root node."""
        return self.parent is None

    @property
    def gimp_obj(self):
        """gimp.Image | gimp.Layer | gimp.LayerGroup: GIMP object that this node represents."""
        return self._gimp_obj

    @property
    def name(self):
        """str: Name of the node without any node options."""
        if isinstance(self.gimp_obj, gimp.Image):
            name = os.path.splitext(self.gimp_obj.name)[0]
        else:
            name = self.gimp_obj.name
            if ' ' in name:
                name = name[:name.find(' ')]
        return name

    @property
    def path(self):
        """str: Hierarchical path of the node with / being the image itself."""
        if self.is_root:
            return '/'
        elif self.parent.is_root:
            return self.parent.path + self.name
        else:
            return self.parent.path + '/' + self.name

    @property
    def children(self):
        """LayerNode[]: Child nodes."""
        return self._children

    @property
    def node_options(self):
        """NodeOption[]: Node options for this nodes."""
        return self._node_options

    @property
    def is_layer_group(self):
        """bool: True if this node represents a group layer."""
        return isinstance(self.gimp_obj, gimp.GroupLayer)

    @property
    def is_layer(self):
        """bool: True if this node represents a single layer."""
        return isinstance(self.gimp_obj, gimp.Layer) and not isinstance(self.gimp_obj, gimp.GroupLayer)

    def save_to_file(self, filename):
        """
        Saves the gimp object that the node represents to a PNG file.

        Args:
            filename (str): Path to write the PNG file to.
        """
        if not filename.endswith('.png'):
            filename += '.png'

        node = self
        while node.parent:
            node = node.parent

        gimp.pdb.file_png_save(node.gimp_obj, self.gimp_obj, filename, 'raw_filename', 0, 9, 0, 0, 0, 0, 0)

    def recurse_children(self, fn):
        """
        Call fn for each child node recursively.

        Args:
            fn: Callback function taking the child node as a single parameter
        """
        for child in self.children:
            fn(child)
            child.recurse_children(fn)

    @classmethod
    def _make_node(cls, parent, gimp_obj):
        cls._check_name(gimp_obj)

        node = cls()
        node._parent = parent
        node._children = []
        node._gimp_obj = gimp_obj
        node._node_options = []
        return node

    @staticmethod
    def _check_name(gimp_obj):
        name = gimp_obj.name
        if isinstance(gimp_obj, gimp.Image):
            name = os.path.splitext(name)[0]

        if not re.match(r'^[a-zA-Z][a-zA-Z0-9_-]*( |$).*$', name):
            raise Exception('Invalid image or layer name "{}". Only alphanumeric characters,' \
                            ' underscores and dashes are allowed. Furthermore, the first character ' \
                            ' must be a letter.'.format(name))

    @classmethod
    def _recursively_create_children(cls, parent):
        for gimp_obj in parent._gimp_obj.layers:
            child = cls._make_node(parent, gimp_obj)
            child._node_options = NodeOption.make_options(child)
            parent._children.append(child)

            if isinstance(gimp_obj, gimp.GroupLayer):
                cls._recursively_create_children(child)
