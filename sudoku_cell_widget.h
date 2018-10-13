#pragma once
// sudoku_cell_widget
//
// The widget for the fillable cells in a sudoku grid.

#include <QSize>
#include <QColor>
#include <QWidget>
#include <bitset>
#include <QPaintEvent>
#include <QFocusEvent>
#include <QKeyEvent>
#include "hint_highlight.h"

class SudokuGridWidget;

class SudokuCellWidget final : public QWidget {
    Q_OBJECT

    const QColor FG_DEFAULT = QColor(0, 0, 0);             // black
    const QColor FG_NONCLUE_ENTRY = QColor(110, 110, 110); // grey
    const QColor BG_DEFAULT = QColor(255, 255, 255);     // white
    const QColor BG_FOCUSED = QColor(172, 172, 255);       // light blue
    const QColor BG_HIGHLIGHTED = QColor(255, 153, 153);   // light red
    const QColor BG_HIGHLIGHTED_HINT_WEAK = QColor(217, 217, 217);   // light grey
    const QColor BG_HIGHLIGHTED_HINT_STRONG = QColor(140, 140, 140);   // strong grey
    const QColor DIGIT_HIGHLIGHTED = QColor(15, 225, 15);   // green
    const QColor DIGIT_HIGHLIGHTED_CONFLICT = QColor(225, 15, 15);   // red

    SudokuGridWidget *m_grid;
    int m_cell_nr;

    bool m_is_entry = false;
    bool m_is_clue = false;
    int m_digit = 0;
    std::bitset<9> m_candidates = {};
    std::bitset<9> m_candidates_highlighted = {};
    std::bitset<9> m_candidates_highlighted_conflict = {};

    public:
        bool m_is_focused = false;
        bool m_is_highlighted = false;
        bool m_in_hint_mode = false;
        HintHighlight m_hint_mode = HintHighlight::None;

    private:
        auto fg_color() const -> QColor;
        auto bg_color() const -> QColor;
        auto bg_color_inner() const -> QColor;

    public:
        explicit SudokuCellWidget(int cell_nr, SudokuGridWidget *parent = 0);
        auto paintEvent(QPaintEvent *event) -> void override;
        // TODO: Is this necessary?
        auto sizeHint() const -> QSize override;
        auto digit() const -> int;
        auto candidates() const -> std::bitset<9>;
        auto focusInEvent(QFocusEvent *event) -> void override;
        auto focusOutEvent(QFocusEvent *event) -> void override;
        auto keyPressEvent(QKeyEvent *event) -> void override;

        auto is_clue() const -> bool;

        auto reset_hint_highlights() -> void;
        auto set_digit_highlight(int digit, bool is_conflict) -> void;

    public slots:
        void set_clue(int digit);
        bool try_set_possibility(int digit, bool is_possible);
        bool try_set_entry(int digit);
        void clear();
};
