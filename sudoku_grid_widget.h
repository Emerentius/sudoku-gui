#pragma once

#include "sudoku_cell_widget.h"
#include "sudoku_ffi/src/sudoku_ffi/sudoku.h"
#include <QFrame>

class SudokuGridWidget final : public QFrame {
    std::vector<SudokuCellWidget*> m_cells;
    Sudoku m_sudoku;

    public:
        explicit SudokuGridWidget(QWidget *parent = 0);

        auto sudoku() -> Sudoku;
        auto generate_major_lines() -> void;
        auto generate_minor_lines() -> void;
        auto generate_cells() -> void;
        auto set_clues() -> void;

        auto compute_candidates() -> void;
};
