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

SudokuCellWidget::SudokuCellWidget(int cell_nr, SudokuGridWidget *parent) :
    QWidget(parent),
    m_grid(parent),
    m_cell_nr(cell_nr)
{
    this->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
}

auto SudokuCellWidget::is_clue() const -> bool {
    return m_is_clue;
}

auto SudokuCellWidget::fg_color() const -> QColor {
    if (m_is_entry && !m_is_clue) {
        return FG_NONCLUE_ENTRY;
    } else {
        return FG_DEFAULT;
    }
}

auto SudokuCellWidget::bg_color() const -> QColor {
    if (m_in_hint_mode) {
        switch (m_hint_mode) {
            case HintHighlight::Strong: return BG_HIGHLIGHTED_HINT_STRONG;
            case HintHighlight::Weak:   return BG_HIGHLIGHTED_HINT_WEAK;
            case HintHighlight::None:   return BG_DEFAULT;
        }
    }

    if (m_is_focused) {
        return BG_FOCUSED;
    }

    if (m_is_highlighted) {
        return BG_HIGHLIGHTED;
    }
    return BG_DEFAULT;
}

auto SudokuCellWidget::bg_color_inner() const -> QColor {
    if (m_in_hint_mode) {
        return bg_color();
    }

    if (m_is_highlighted) {
        return BG_HIGHLIGHTED;
    }
    if (m_is_focused) {
        return BG_FOCUSED;
    }
    return BG_DEFAULT;
}

auto SudokuCellWidget::paintEvent(QPaintEvent *event) -> void {
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // draw background
    auto bg = this->bg_color();
    painter.setBrush(QBrush(bg));
    painter.drawRect(event->rect());

    // draw inner, rounded square over background
    // if multiple highlights exist on a cell
    // do so conditionally because there is a black 1px border
    // drawn around it
    auto bg_inner = this->bg_color_inner();
    if (bg != bg_inner) {
        painter.setBrush(QBrush(bg_inner));
        auto ring_width = this->width() / 12;
        auto low = ring_width;
        auto high = this->width() - 2*low;
        painter.drawRoundedRect(QRect(low, low, high, high), 25, 25, Qt::SizeMode::RelativeSize);
    }

    auto font = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    auto const alignment = Qt::AlignCenter;
    QString text;

    if (m_is_entry) {
        font.setPixelSize( this->width() * 5 / 6 );
        if (m_digit != 0) {
            text = QString::number(m_digit);
        }
    } else {
        font.setPixelSize(this->width() / 4);
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

        // draw a circle in the place of the digits before the text is drawn
        // TODO: clean up, use the layout system to offload the positioning to Qt
        painter.setPen(Qt::NoPen);
        auto size = this->size().width(); // cell is quadratic
        auto center = QPoint(size / 2, size / 2);
        int offset = size * 1.25 / 4;

        int digit = 0;
        for (int row_offset = -1; row_offset <= 1; row_offset++) {
            for (int col_offset = -1; col_offset <= 1; col_offset++) {
                auto digit_offset = QPoint(col_offset * offset, row_offset * offset);
                auto digit_pos = center + digit_offset;
                auto radius = size * 9 / 64; // a bit more than 1/4 / 2, the size of the font

                if (m_candidates_highlighted[digit]) {
                    painter.setBrush(QBrush(DIGIT_HIGHLIGHTED));
                    painter.drawEllipse(digit_pos, radius, radius);
                }

                if (m_candidates_highlighted_conflict[digit]) {
                    painter.setBrush(QBrush(DIGIT_HIGHLIGHTED_CONFLICT));
                    painter.drawEllipse(digit_pos, radius, radius);
                }
                digit++;
            }
        }
    }

    painter.setPen(this->fg_color());
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
    m_is_focused = true;
    this->update();
}

auto SudokuCellWidget::focusOutEvent(QFocusEvent *event) -> void {
    m_is_focused = false;
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
}

auto SudokuCellWidget::clear() -> void {
    m_is_clue = false;
    m_is_entry = false;
    m_digit = 0;
    m_candidates = std::bitset<9>();
}

auto SudokuCellWidget::keyPressEvent(QKeyEvent *event) -> void {
    if (m_in_hint_mode) {
        return;
    }

    if (
        (event->modifiers() & Qt::KeyboardModifier::ControlModifier) != 0
        && event->key() == Qt::Key_Z
    ) {
        // TODO: on shift, redo
        m_grid->undo();
        return;
    }

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
        m_grid->insert_candidate(Candidate {
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
                Candidate {
                    cell: m_cell_nr,
                    num: pos+1,
                },
                !is_possible
            );
        }
    }
    this->update();
}

auto SudokuCellWidget::reset_highlights() -> void {
    m_candidates_highlighted = {};
    m_candidates_highlighted_conflict = {};
    m_is_highlighted = false;
    m_hint_mode = HintHighlight::None;
}

auto SudokuCellWidget::set_digit_highlight(int digit, bool is_conflict) -> void {
    if (is_conflict) {
        m_candidates_highlighted_conflict[digit] = true;
    } else {
        m_candidates_highlighted[digit] = true;
    }
}
