#pragma once

#include "sudoku_cell_widget.h"
#include <QFrame>

class SudokuGridWidget final : public QFrame {
    public:
        explicit SudokuGridWidget(QWidget *parent = 0);
};
