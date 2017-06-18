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

    def run(self):
        """
        Runs the plugin.
        """
        pass
