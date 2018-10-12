#pragma once

#include "sudoku_ffi/src/sudoku_ffi/sudoku.h"
#include <QFrame>
#include "quadratic_qframe.h"
#include "hint_highlight.h"

class SudokuCellWidget;

enum class Direction {
    Left, Right, Up, Down
};

using Candidates = std::array<uint16_t, 81>;

class SudokuGridWidget final : public QuadraticQFrame {
    Q_OBJECT

    std::array<SudokuCellWidget*, 81> m_cells;
    std::vector<Candidates> m_candidates;
    int m_highlighted_digit = 0; // 1-9, 0 for no highlight
    bool m_in_hint_mode = false;

    // TODO: make private again
    public:
        std::vector<Sudoku> m_sudoku;

    private:
        auto current_sudoku() -> Sudoku&;
        auto current_candidates() -> Candidates&;
        auto grid_state() const -> GridState;
        auto push_savepoint() -> void;
        auto pop_savepoint() -> void;
        auto update_cells() -> void;
        auto update_highlights() -> void;
        auto initialize_cells() -> void;
        auto generate_layout() -> void;
        auto set_clues() -> void;
        auto compute_candidates() -> void;

        auto set_house_highlight(int, HintHighlight) -> void;
        auto set_cell_highlight(int, HintHighlight) -> void;

    public:
        explicit SudokuGridWidget(QWidget *parent = 0);

        auto sudoku() const -> Sudoku;
        auto recompute_candidates() -> void;

        auto move_focus(int current_cell, Direction direction) -> void;

        auto insert_candidate(Candidate candidate) -> void;
        auto set_candidate(Candidate candidate, bool is_possible) -> void;
        auto undo() -> bool;

    public slots:
        void highlight_digit(int digit);
        void hint(std::vector<Strategy> strategies);
};
