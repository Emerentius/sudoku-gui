#pragma once

#include "sudoku_ffi/src/sudoku_ffi/sudoku.h"
#include <QFrame>

class SudokuCellWidget;

enum class Direction {
    Left, Right, Up, Down
};

class SudokuGridWidget final : public QFrame {
    std::vector<SudokuCellWidget*> m_cells;

    // TODO: make private again
    public:
        Sudoku m_sudoku;

    public:
        explicit SudokuGridWidget(QWidget *parent = 0);

        auto sudoku() -> Sudoku;
        auto generate_major_lines() -> void;
        auto generate_minor_lines() -> void;
        auto generate_cells() -> void;
        auto set_clues() -> void;

        auto compute_candidates() -> void;
        auto recompute_candidates() -> void;

        auto move_focus(int current_cell, Direction direction) -> void;
};
