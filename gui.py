#!/usr/bin/env python3
from PyQt5 import QtCore, QtWidgets
from sudoku_cell_widget import SudokuCellWidget

MAJOR_LINE_SIZE = 6
MINOR_LINE_SIZE = 2

class Ui_Form(object):
    def setupUi(self, Form):
        Form.setObjectName("Form")

        Form.resize(1000, 1000)

        # quadratic
        sudoku_size = 900
        x_off, y_off = 30, 30
        cell_size = 96 #int(sudoku_size // 9)

        def gen_frame(parent, x_off, y_off):
            frame = QtWidgets.QFrame(parent)
            frame.setGeometry(QtCore.QRect(x_off, y_off, sudoku_size, sudoku_size))
            sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
            sizePolicy.setHorizontalStretch(0)
            sizePolicy.setVerticalStretch(0)
            sizePolicy.setHeightForWidth(frame.sizePolicy().hasHeightForWidth())
            frame.setSizePolicy(sizePolicy)
            frame.setFrameShape(QtWidgets.QFrame.Box)
            frame.setFrameShadow(QtWidgets.QFrame.Plain)
            frame.setLineWidth(MAJOR_LINE_SIZE)
            return frame

        self.frame = gen_frame(Form, x_off, y_off)
        # to draw box over containing cells
        #self.inner_frame = gen_frame(self.frame, 0, 0)


        ## create grid of lines ###################
        #line_off = int(sudoku_size / 3)

        # large lines between blocks
        major_hor_lines = []
        major_ver_lines = []
        #offset = -MAJOR_LINE_SIZE
        for i in range(1, 3):
            offset = i * (MAJOR_LINE_SIZE + 3 * cell_size + 2 * MINOR_LINE_SIZE)
            line_ = line(self.frame, QtWidgets.QFrame.HLine)
            line_.setGeometry(QtCore.QRect(MAJOR_LINE_SIZE, offset, sudoku_size, MAJOR_LINE_SIZE))
            major_hor_lines.append(line_)

            line_ = line(self.frame, QtWidgets.QFrame.VLine)
            line_.setGeometry(QtCore.QRect(offset, MAJOR_LINE_SIZE, MAJOR_LINE_SIZE, sudoku_size))
            major_ver_lines.append(line_)

        # small lines between cells
        # minor_line_off = int(sudoku_size / 9)
        minor_hor_lines = []
        minor_ver_lines = []
        pos = 0
        for i in range(9):
            pos += cell_size + (MAJOR_LINE_SIZE if i % 3 == 0 else MINOR_LINE_SIZE)
            line_ = minor_line(self.frame, QtWidgets.QFrame.HLine)
            line_.setGeometry(QtCore.QRect(0, pos, sudoku_size, MINOR_LINE_SIZE))
            minor_hor_lines.append(line_)

            line_ = minor_line(self.frame, QtWidgets.QFrame.VLine)
            line_.setGeometry(QtCore.QRect(pos, 0, MINOR_LINE_SIZE, sudoku_size))
            minor_ver_lines.append(line_)

        ## create cells ########################
        cells = []
        x_pos = 0
        for x_idx in range(9):
            x_pos += MAJOR_LINE_SIZE if x_idx % 3 == 0 else MINOR_LINE_SIZE

            y_pos = 0
            for y_idx in range(9):
                y_pos += MAJOR_LINE_SIZE if y_idx % 3 == 0 else MINOR_LINE_SIZE

                cell = SudokuCellWidget(cell_size, self.frame)
                cell.move(x_pos, y_pos)
                cell.lower()
                cells.append(cell)

                y_pos += cell_size

            x_pos += cell_size

        for line_ in major_hor_lines + major_ver_lines + minor_hor_lines + minor_ver_lines:
            line_.raise_()
        self.frame.raise_()

        self.retranslateUi(Form)
        QtCore.QMetaObject.connectSlotsByName(Form)

    def retranslateUi(self, Form):
        _translate = QtCore.QCoreApplication.translate
        Form.setWindowTitle(_translate("Form", "Form"))

def line(frame, shape) -> QtWidgets.QFrame:
    line = QtWidgets.QFrame(frame)
    sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Minimum)
    sizePolicy.setHorizontalStretch(0)
    sizePolicy.setVerticalStretch(0)
    line.setSizePolicy(sizePolicy)

    line.setFrameShape(shape)
    line.setFrameShadow(QtWidgets.QFrame.Plain)
    line.setLineWidth(MAJOR_LINE_SIZE)
    return line

def minor_line(frame, shape) -> QtWidgets.QFrame:
    line_ = line(frame, shape)
    line_.setLineWidth(MINOR_LINE_SIZE)
    return line_


if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    form = QtWidgets.QWidget()
    ui = Ui_Form()
    ui.setupUi(form)
    form.show()
    sys.exit(app.exec_())

