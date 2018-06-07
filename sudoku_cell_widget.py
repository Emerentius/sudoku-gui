#!/usr/bin/env python3

"""
sudoku_cell_widget.py

The widget for the fillable cells in a sudoku grid.
"""
from typing import Union, List, Optional

from PyQt5.QtCore import pyqtProperty, pyqtSlot, QSize, Qt
from PyQt5.QtGui import QBrush, QColor, QPainter, QFontDatabase
from PyQt5.QtWidgets import QApplication, QWidget

class SudokuCellWidget(QWidget):
    """SudokuCellWidget(QWidget)

    Custom widget for the 81 fillable cells in a sudoku.
    """

    def __init__(self, size, parent=None):
        super().__init__(parent)
        # _number is either nothing (0), a digit (1-9) or the possibility set of digits
        self._number: Union[int, List[bool]] = 0
        self._fixed = True
        self._bg_color = QColor(255, 255, 255)
        self._fg_color = QColor(0, 0, 0)
        self.setFocusPolicy(Qt.FocusPolicy(2)) # 2 = ClickPolicy
        self.setFixedWidth(size)
        self.setFixedHeight(size)

    def paintEvent(self, event):
        painter = QPainter()
        painter.begin(self)
        painter.setRenderHint(QPainter.Antialiasing)
        painter.setBrush(QBrush(self._bg_color))
        painter.drawRect(event.rect())

        #painter.translate(self.width()/2.0, self.height()/2.0)
        painter.setPen(self.fg_color)
        #font = painter.font()
        font = QFontDatabase.systemFont(QFontDatabase.FixedFont)

        if isinstance(self._number, list):
            #text = ''
            possible_nums = self._number

            def row_to_str(row, possible_nums):
                num_poss_iter = ((row*3+idx+1, possible) for idx, possible in enumerate(possible_nums[3*row:3*row+3]))
                # Non-Breaking Space. Forces Qt to lay out the text as given
                # and not stretch or shuffle whitespace around
                nbs = '\u00A0'
                return nbs.join(str(num) if poss else nbs for num, poss in num_poss_iter)

            text = '\n'.join(row_to_str(row, possible_nums) for row in range(3))

            #num_rows = ((range(i, i+3), self._number[i:i+3]) for i in range(3))
            #str_rows = (' '.join(str(num) if possible else ' ' for num, possible in zip(nums, nums_possible)) for nums, nums_possible in num_rows)
            #text = '\n'.join(str_rows)
            font.setPixelSize(24)
            alignment = Qt.AlignCenter
        else:
            text = str(self._number) if self._number is not 0 else ''
            font.setPixelSize(80)
            alignment = Qt.AlignCenter

        painter.setFont(font)
        painter.drawText(event.rect(), alignment | Qt.TextIncludeTrailingSpaces | Qt.TextJustificationForced, text)
        painter.end()

    def sizeHint(self):
        return QSize(self.width(), self.height())

    def getNumber(self):
        return self._number

    @pyqtSlot(int)
    def setNumber(self, number):
        if 0 <= number <= 9:
            self._number = number
        self.update()

    number = pyqtProperty(int, getNumber, setNumber)

    def getNumberPossibilities(self) -> Optional[List[bool]]:
        if isinstance(self._number, list):
            return self._number
        return None

    def setNumberPossibility(self, number):
        poss = self.getNumberPossibilities() or [False] * 9
        poss[number-1] = True
        self._number = poss
        self.update()

    def getFixed(self):
        print(self._fixed)
        print('getFixed')
        return self._fixed

    @pyqtSlot(bool)
    def setFixed(self, fixed):
        print('setFixed')
        self._fixed = fixed
        print(self._fixed)
        if fixed:
            self._fg_color = QColor(0, 0, 0)
        else:
            self._fg_color = QColor(110, 110, 110)
        self.update()
        self.update()

    fixed = pyqtProperty(bool, getFixed, setFixed)

    def getBgColor(self):
        return self._bg_color

    def setBgColor(self, color):
        self._bg_color = color
        self.update()

    bg_color = pyqtProperty(QColor, getBgColor, setBgColor)

    def getFgColor(self):
        return self._fg_color

    def setFgColor(self, color):
        self._fg_color = color
        self.update()

    fg_color = pyqtProperty(QColor, getFgColor, setFgColor)

    # Handle keyboard input and focus events

    def keyPressEvent(self, key_event):
        one  = Qt.Key_1
        nine = Qt.Key_9

        # TODO: Debugging list
        #       Set cell possibilities (possibly unsetting a fixed entry)
        # NEO
        #second_row = [Qt.Key_X, Qt.Key_V, Qt.Key_L, Qt.Key_C, Qt.Key_W, Qt.Key_K, Qt.Key_H, Qt.Key_G, Qt.Key_F]
        # F keys
        second_row = [Qt.Key_F1, Qt.Key_F2, Qt.Key_F3, Qt.Key_F4, Qt.Key_F5, Qt.Key_F6, Qt.Key_F7, Qt.Key_F8, Qt.Key_F9]

        # check probably irrelevant
        # set checks, too
        # don't know range of key though
        if one <= key_event.key() <= nine:
            num = key_event.key() - Qt.Key_0
            self.setNumber(num)
        elif key_event.key() == Qt.Key_Backspace:
            self.setNumber(0)
        ## TODO: Debugging branch
        #        Switch clue between hard set (given) and user set
        #        Not handled: Possibilities don't need the distinction
        elif key_event.key() == Qt.Key_Space:
            self.setFixed(not self.getFixed())
        elif key_event.key() in second_row:
            num = next(i for i in range(9) if second_row[i] == key_event.key()) + 1
            self.setNumberPossibility(num)
        else:
            super().keyPressEvent(key_event)

    def focusInEvent(self, _event):
        self.setBgColor(QColor(172, 172, 255))
        self.update()

    def focusOutEvent(self, _event):
        self.setBgColor(QColor(255, 255, 255))
        self.update()

if __name__ == "__main__":

    import sys

    app = QApplication(sys.argv)
    window = SudokuCellWidget()
    window.show()
    sys.exit(app.exec_())
