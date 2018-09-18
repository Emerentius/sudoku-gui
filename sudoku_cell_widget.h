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

class SudokuCellWidget final : public QWidget {
    Q_OBJECT

    const QColor DEFAULT_FG = QColor(0, 0, 0);             // black
    const QColor NONCLUE_ENTRY_FG = QColor(110, 110, 110); // grey
    const QColor UNFOCUSED_BG = QColor(255, 255, 255);     // white
    const QColor FOCUSED_BG = QColor(172, 172, 255);

    bool m_is_entry = false;
    bool m_is_clue = false;
    int m_digit = 0;
    std::bitset<9> m_candidates = {};

    QColor m_bg_color = UNFOCUSED_BG;
    QColor m_fg_color = DEFAULT_FG;

    public:
        explicit SudokuCellWidget(int size, QWidget *parent = 0);
        void paintEvent(QPaintEvent *event) override;
        // TODO: Is this necessary?
        QSize sizeHint() const override;
        int digit() const;
        std::bitset<9> candidates() const;
        void focusInEvent(QFocusEvent *event) override;
        void focusOutEvent(QFocusEvent *event) override;
        void keyPressEvent(QKeyEvent *event) override;

    public slots:
        void set_possibility(int digit);
        void set_clue(int digit);
        void set_entry(int digit);
        void clear();
};
