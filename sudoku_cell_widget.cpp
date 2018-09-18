// sudoku_cell_widget
//
// The widget for the fillable cells in a sudoku grid.

#include "sudoku_cell_widget.h"
#include <QtCore>
#include <QSize>
#include <Qt>
#include <QBrush>
#include <QColor>
#include <QPainter>
#include <QFontDatabase>
#include <algorithm>

SudokuCellWidget::SudokuCellWidget(int size, QWidget *parent) :
    QWidget(parent)
    /*
    m_bg_color(UNFOCUSED_BG),
    m_fg_color(DEFAULT_FG),
    m_is_clue(false),
    m_is_entry(false),
    m_candidates(),
    m_digit(0)
    */
{
    this->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    this->setFixedWidth(size);
    this->setFixedHeight(size);
}

void SudokuCellWidget::paintEvent(QPaintEvent *event) {
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(m_bg_color));
    painter.drawRect(event->rect());

    painter.setPen(m_fg_color);
    auto font = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    auto const alignment = Qt::AlignCenter;
    QString text;

    if (m_is_entry) {
        font.setPixelSize(80);
        if (m_digit != 0) {
            text = QString::number(m_digit);
        }
    } else {
        font.setPixelSize(24);
        // Non-Breaking Space. Normal spaces are trimmed.
        // This forces Qt to lay out the text as given
        auto nbs = QString::fromUtf8(u8"\u00A0");

        // for each digit, enter the text representation
        // or a space character, if it's not set
        // join each position in a row with `nbs`
        // join rows with linebreaks
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                auto num = 3 * row + col;
                if (m_candidates[num]) {
                    text += QString::number(num+1);
                } else {
                    text += nbs;
                }
                if (col != 2) {
                    text += nbs;
                }
            }
            if (row != 2) {
                text += "\n";
            }
        }
    }

    painter.setFont(font);

    // TODO: The additional alignment flags may be unnecessary
    //       Check it!
    painter.drawText(
        event->rect(),
        alignment | Qt::TextIncludeTrailingSpaces | Qt::TextJustificationForced,
        text
    );
}

// TODO: Is this necessary?
QSize SudokuCellWidget::sizeHint() const {
    return QSize(this->width(), this->height());
}

int SudokuCellWidget::digit() const {
    if (m_is_entry) {
        return m_digit;
    }
    // TODO:
    // else throw some error
}

std::bitset<9> SudokuCellWidget::candidates() const {
    if (!m_is_entry) {
        return m_candidates;
    }
    // TODO:
    // else throw some error
}

// TODO: necessary?
//bool is_clue() const { return m_is_clue; }

void SudokuCellWidget::focusInEvent(QFocusEvent *event) {
    m_bg_color = FOCUSED_BG;
    this->update();
}

void SudokuCellWidget::focusOutEvent(QFocusEvent *event) {
    m_bg_color = UNFOCUSED_BG;
    this->update();
}

void SudokuCellWidget::set_possibility(int digit) {
    if (m_is_entry) {
        // TODO: throw some error
    }
    if (0 < digit && digit < 10) {
        m_candidates[digit-1] = true;
    }
    this->update();
}

void SudokuCellWidget::set_clue(int digit) {
    if (0 < digit && digit < 10) {
        m_is_entry = true;
        m_is_clue = true;
        m_digit = digit;
        m_fg_color = DEFAULT_FG;
    } else {
        // TODO: throw error
    }
    this->update();
}

// non-clue entry
void SudokuCellWidget::set_entry(int digit) {
    if (0 < digit && digit < 10) {
        m_is_entry = true;
        m_is_clue = false;
        m_digit = digit;
        m_fg_color = NONCLUE_ENTRY_FG;
    } else {
        // TODO: throw error
    }
    this->update();
}

void SudokuCellWidget::clear() {
    m_is_clue = false;
    m_is_entry = false;
    m_digit = 0;
    m_candidates = std::bitset<9>();
    m_fg_color = DEFAULT_FG;
    this->update();
}

void SudokuCellWidget::keyPressEvent(QKeyEvent *event) {
    auto one = Qt::Key_1;
    auto nine = Qt::Key_9;

    std::array<Qt::Key, 9> second_row = {
        Qt::Key_F1,
        Qt::Key_F2,
        Qt::Key_F3,
        Qt::Key_F4,
        Qt::Key_F5,
        Qt::Key_F6,
        Qt::Key_F7,
        Qt::Key_F8,
        Qt::Key_F9
    };

    if (one <= event->key() && event->key() <= nine) {
        auto num = event->key() - Qt::Key_0;
        this->set_entry(num);
    } else if (event->key() == Qt::Key_Backspace) {
        this->clear();
    } else {
        auto key_ptr = std::find(second_row.begin(), second_row.end(), event->key());

        if (key_ptr != second_row.end()) {
            int pos = std::distance(second_row.begin(), key_ptr);
            this->set_possibility(pos+1);
        }
    }
}
