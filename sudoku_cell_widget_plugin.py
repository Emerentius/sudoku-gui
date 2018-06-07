#!/usr/bin/env python3

"""
sudoku_cell_widget_plugin.py

A custom widget for a cell in a sudoku grid.
"""

from PyQt5.QtGui import QIcon, QPixmap
from PyQt5.QtDesigner import QPyDesignerCustomWidgetPlugin

from sudoku_cell_widget import SudokuCellWidget

class SudokuCellWidgetPlugin(QPyDesignerCustomWidgetPlugin):
    """SudokuCellWidgetPlugin(QPyDesignerCustomWidgetPlugin)

    Provides a Python custom plugin for Qt Designer by implementing the
    QDesignerCustomWidgetPlugin via a PyQt-specific custom plugin class.
    """

    def __init__(self, parent=None):
        super(SudokuCellWidgetPlugin, self).__init__(parent)
        self.initialized = False

    def initialize(self, core):
        if self.initialized:
            return
        self.initialized = True

    def isInitialized(self):
        return self.initialized

    def createWidget(self, parent):
        return SudokuCellWidget(parent)

    def name(self):
        return "SudokuCellWidget"

    # Name of group under which widget should be listed in designer
    def group(self):
        return "Sudoku"

    def icon(self):
        return QIcon(_logo_pixmap)

    def toolTip(self):
        return ""

    def whatsThis(self):
        return ""

    def isContainer(self):
        return False

    # Returns an XML description of a custom widget instance that describes
    # default values for its properties. Each custom widget created by this
    # plugin will be configured using this description.
    def domXml(self):
        return '<widget class="SudokuCellWidget" name="sudokuCellWidget" />\n'

    # Returns the module containing the custom widget class. It may include
    # a module path.
    def includeFile(self):
        return "sudoku_cell_widget"


# Define the image used for the icon.
_logo_16x16_xpm = [
"16 16 46 1",
". c #a5a5dc",
"l c #a69fd6",
"k c #a7a5da",
"h c #a7a6dc",
"a c #a7a7de",
"Q c #a8a5da",
"s c #a9a7d7",
"R c #a9a9e0",
"z c #abaad4",
"E c #afafda",
"M c #afafdb",
"K c #b0a8e2",
"o c #b1afe4",
"p c #b2b2d7",
"# c #b2b2ed",
"i c #b39eb6",
"F c #b3b3e1",
"e c #b4b4ef",
"t c #b58bab",
"d c #b6b6f2",
"n c #b798b8",
"P c #b798b9",
"c c #b8b6f2",
"D c #b8b89c",
"m c #b9648d",
"J c #ba84b0",
"A c #bdbdfb",
"f c #bfbffe",
"g c #c06996",
"b c #c0c0ff",
"B c #cbb889",
"L c #cbb989",
"O c #cfcf87",
"I c #d09585",
"w c #d0cf86",
"x c #dede81",
"G c #e8e87c",
"q c #edde7b",
"N c #f1e07b",
"v c #f2e07b",
"H c #f6e57c",
"j c #fb917e",
"u c #ffb580",
"r c #ffda80",
"C c #fffe80",
"y c #ffff80",
".##############a",
"#bbbbbbbbcdbbbbe",
"#bbbbbbbfghbbbbe",
"#bbbbbbbijkbbbbe",
"#blmnobpqrsbbbbe",
"#bbtuvwxyyzbbbbe",
"#bbABCyyyyDEfbbe",
"#bbbFGyyyyyHIJKe",
"#bbbFGyyyyyHIJKe",
"#bbALCyyyyDMfbbe",
"#bbtuNOxyyzbbbbe",
"#blmPobpqrsbbbbe",
"#bbbbbbbijQbbbbe",
"#bbbbbbbfghbbbbe",
"#bbbbbbbbcdbbbbe",
"aeeeeeeeeeeeeeeR"]

_logo_pixmap = QPixmap(_logo_16x16_xpm)
