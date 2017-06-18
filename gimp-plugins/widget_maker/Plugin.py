from LayerNode import *

from gimpfu import *
import os


class Plugin(object):
    """
    Main controller
    """
    def __init__(self, image):
        """
        Creates the plugin for the given image.

        Args:
            image (gimp.Image): Image to instantiate the plugin for.
        """
        self._image = image
        self._output_folder = os.path.dirname(image.filename)
        self._root_node = LayerNode.make_root_node(image)
        self._filename_prefix = self._root_node.name

    def run(self):
        """
        Runs the plugin.
        """
        self.export_layers()

    def export_layers(self):
        """
        Exports all layers to PNG files.
        """
        def fn(node):
            if node.is_layer:
                basename = self._output_folder + '/img' + node.path

                directory = os.path.dirname(basename)
                if not os.path.exists(directory):
                    os.makedirs(directory)

                node.save_to_file(basename)

        self._root_node.recurse_children(fn)
