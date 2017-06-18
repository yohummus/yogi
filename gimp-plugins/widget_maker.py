#!/usr/bin/env python
from gimpfu import *
import traceback


def plugin_main(image, layer):
    try:
        from widget_maker.Plugin import Plugin as WidgetMakerPlugin
        plugin = WidgetMakerPlugin(image)
        plugin.run()
    except Exception:
        gimp.message('ERROR: {}'.format(traceback.format_exc()))

register(
    "yogi-widget-maker",
    "Export image as a YOGI Widget",
    "Exports the image as an interactive Angular 2 component that can be used with the YOGI framework.",
    "Johannes Bergmann",
    "GPLv3",
    "2017",
    "<Image>/File/Export/Export as YOGI Widget",
    "*",
    [],
    [],
    plugin_main
)

main()
