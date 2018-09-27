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

class SudokuGridWidget;

class SudokuCellWidget final : public QWidget {
    Q_OBJECT

    const QColor FG_DEFAULT = QColor(0, 0, 0);             // black
    const QColor FG_NONCLUE_ENTRY = QColor(110, 110, 110); // grey
    const QColor BG_DEFAULT = QColor(255, 255, 255);     // white
    const QColor BG_FOCUSED = QColor(172, 172, 255);       // light blue
    const QColor BG_HIGHLIGHTED = QColor(255, 153, 153);   // light red

    SudokuGridWidget *m_grid;
    int m_cell_nr;

    bool m_is_entry = false;
    bool m_is_clue = false;
    int m_digit = 0;
    std::bitset<9> m_candidates = {};

    public:
        bool m_is_focused = false;
        bool m_is_highlighted = false;

    private:
        auto fg_color() const -> QColor;
        auto bg_color() const -> QColor;

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

    public slots:
        void set_clue(int digit);
        bool try_set_possibility(int digit, bool is_possible);
        bool try_set_entry(int digit);
        void clear();
};
