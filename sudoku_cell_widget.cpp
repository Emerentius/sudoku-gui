// sudoku_cell_widget
//
// The widget for the fillable cells in a sudoku grid.

#include <QtCore>
#include <QSize>
#include <Qt>
#include <QBrush>
#include <QColor>
#include <QPainter>
#include <QFontDatabase>
#include <algorithm>
#include <assert.h>
#include "sudoku_cell_widget.h"
#include "sudoku_grid_widget.h"

SudokuCellWidget::SudokuCellWidget(int size, int cell_nr, SudokuGridWidget *parent) :
    QWidget(parent),
    m_grid(parent),
    m_cell_nr(cell_nr)
{
    this->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    this->setFixedWidth(size);
    this->setFixedHeight(size);
}

auto SudokuCellWidget::is_clue() const -> bool {
    return m_is_clue;
}

auto SudokuCellWidget::paintEvent(QPaintEvent *event) -> void {
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
auto SudokuCellWidget::sizeHint() const -> QSize {
    return QSize(this->width(), this->height());
}

auto SudokuCellWidget::digit() const -> int {
    if (m_is_entry) {
        return m_digit;
    }
    // TODO:
    // else throw some error
}

auto SudokuCellWidget::candidates() const -> std::bitset<9> {
    if (!m_is_entry) {
        return m_candidates;
    }
    // TODO:
    // else throw some error
}

auto SudokuCellWidget::focusInEvent(QFocusEvent *event) -> void {
    m_bg_color = FOCUSED_BG;
    this->update();
}

auto SudokuCellWidget::focusOutEvent(QFocusEvent *event) -> void {
    m_bg_color = UNFOCUSED_BG;
    this->update();
}

auto SudokuCellWidget::try_set_possibility(int digit, bool is_possible) -> bool {
    if (m_is_entry) {
        return false;
    }
    assert(0 < digit && digit < 10);
    m_candidates[digit-1] = is_possible;
    return true;
}

auto SudokuCellWidget::set_clue(int digit) -> void {
    assert(0 < digit && digit < 10);
    m_is_entry = true;
    m_is_clue = true;
    m_digit = digit;
    m_fg_color = DEFAULT_FG;
}

// non-clue entry
auto SudokuCellWidget::try_set_entry(int digit) -> bool {
    if (m_is_clue || m_is_entry) {
        return false;
    }

    assert(0 < digit && digit < 10);

    m_is_entry = true;
    m_is_clue = false;
    m_digit = digit;
    m_fg_color = NONCLUE_ENTRY_FG;
}

auto SudokuCellWidget::clear() -> void {
    m_is_clue = false;
    m_is_entry = false;
    m_digit = 0;
    m_candidates = std::bitset<9>();
    m_fg_color = DEFAULT_FG;
}

auto SudokuCellWidget::keyPressEvent(QKeyEvent *event) -> void {
    Qt::Key arrow_keys[] = {Qt::Key_Left, Qt::Key_Up, Qt::Key_Right, Qt::Key_Down};
    Direction directions[] = {Direction::Left, Direction::Up, Direction::Right, Direction::Down};

    auto start = std::begin(arrow_keys);
    auto end = std::end(arrow_keys);
    auto position = std::find(start, end, event->key());

    if (position != end) {
        auto idx = std::distance(start, position);
        m_grid->move_focus(m_cell_nr, directions[idx]);
        return;
    }

    if (m_is_clue) {
        return;
    }

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
        //this->try_set_entry(num);
        m_grid->insert_entry(Entry {
            cell: m_cell_nr,
            num: num,
        });
    //} else if (event->key() == Qt::Key_Backspace) {
    //    this->clear();
    } else {
        auto key_ptr = std::find(second_row.begin(), second_row.end(), event->key());

        if (key_ptr != second_row.end()) {
            int pos = std::distance(second_row.begin(), key_ptr);
            auto is_possible = m_candidates[pos];
            m_grid->set_candidate(
                Entry {
                    cell: m_cell_nr,
                    num: pos+1,
                },
                !is_possible
            );
        }
    }
    this->update();
}
