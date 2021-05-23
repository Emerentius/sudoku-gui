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
#include "cell_state.h"

enum class DigitHighlight {
    Regular,
    Conflict, // removable candidates
};

class SudokuGridWidget;

class SudokuCellWidget final : public QWidget {
    Q_OBJECT

    const QColor FG_DEFAULT = QColor(0, 0, 0);                       // black
    const QColor FG_NONCLUE_ENTRY = QColor(110, 110, 110);           // grey
    const QColor BG_DEFAULT = QColor(255, 255, 255);                 // white
    const QColor BG_FOCUSED = QColor(172, 172, 255);                 // light blue
    const QColor BG_HIGHLIGHTED = QColor(255, 153, 153);             // light red
    const QColor BG_HIGHLIGHTED_HINT_WEAK = QColor(217, 217, 217);   // light grey
    const QColor BG_HIGHLIGHTED_HINT_STRONG = QColor(140, 140, 140); // strong grey
    const QColor DIGIT_HIGHLIGHTED = QColor(15, 225, 15);            // green
    const QColor DIGIT_HIGHLIGHTED_CONFLICT = QColor(225, 15, 15);   // red

    SudokuGridWidget* const m_grid;
    const uint8_t m_cell_nr;

    std::array<std::optional<DigitHighlight>, 9> m_candidates_highlights = {};

public:
    HintHighlight m_hint_mode = HintHighlight::None;

private:
    auto cell_state() const -> const CellWidgetState&;

    auto fg_color() const -> QColor;
    auto bg_color() const -> QColor;
    auto bg_color_inner() const -> QColor;

    auto in_hint_mode() const -> bool;
    auto contains_highlighted_digit() const -> bool;

public:
    explicit SudokuCellWidget(int cell_nr, SudokuGridWidget* parent = 0);
    auto paintEvent(QPaintEvent* event) -> void override;
    auto keyPressEvent(QKeyEvent* event) -> void override;

    auto is_clue() const -> bool;
    auto is_entry() const -> bool;
    auto is_candidates() const -> bool;
    auto digit() const -> std::optional<int>;
    auto candidates() const -> std::optional<std::bitset<9>>;

    auto reset_hint_highlights() -> void;
    auto set_digit_highlight(int digit, bool is_conflict) -> void;
};
