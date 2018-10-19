// sudoku_cell_widget
//
// The widget for the fillable cells in a sudoku grid.

#include "sudoku_cell_widget.h"
#include "sudoku_grid_widget.h"
#include <QBrush>
#include <QColor>
#include <QFontDatabase>
#include <QPainter>
#include <QSize>
#include <Qt>
#include <QtCore>
#include <algorithm>
#include <cassert>

SudokuCellWidget::SudokuCellWidget(int cell_nr, SudokuGridWidget *parent) :
    QWidget(parent),
    m_grid(parent),
    m_cell_nr(cell_nr)
{
    this->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
}

auto SudokuCellWidget::is_clue() const -> bool {
    return std::holds_alternative<Clue>(m_state);
}

auto SudokuCellWidget::is_entry() const -> bool {
    return std::holds_alternative<Entry>(m_state);
}

auto SudokuCellWidget::is_candidates() const -> bool {
    return std::holds_alternative<CellCandidates>(m_state);
}

auto SudokuCellWidget::fg_color() const -> QColor {
    if (this->is_entry()) {
        return FG_NONCLUE_ENTRY;
    }
    return FG_DEFAULT;
}

auto SudokuCellWidget::bg_color() const -> QColor {
    if (this->in_hint_mode()) {
        switch (m_hint_mode) {
            case HintHighlight::Strong: return BG_HIGHLIGHTED_HINT_STRONG;
            case HintHighlight::Weak:   return BG_HIGHLIGHTED_HINT_WEAK;
            case HintHighlight::None:   return BG_DEFAULT;
        }
    }

    if (this->hasFocus()) {
        return BG_FOCUSED;
    }

    if (this->contains_highlighted_digit()) {
        return BG_HIGHLIGHTED;
    }
    return BG_DEFAULT;
}

auto SudokuCellWidget::bg_color_inner() const -> QColor {
    if (this->in_hint_mode()) {
        return bg_color();
    }

    if (this->contains_highlighted_digit()) {
        return BG_HIGHLIGHTED;
    }
    if (this->hasFocus()) {
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

    auto digit = this->digit();
    if (digit) {
        font.setPixelSize( this->width() * 5 / 6 );
        text = QString::number(digit.value());
    } else {
        auto candidates = this->candidates().value();
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
                if (candidates[num]) {
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

                // strategy results don't always contain the full list of candidates
                // only 2 sets of some position and some digits
                // we check here so we don't highlight empty places
                if (candidates[digit]) {
                    auto highlight = m_candidates_highlights[digit];
                    if (highlight) {
                        switch (*highlight) {
                            case DigitHighlight::Regular: {
                                painter.setBrush(QBrush(DIGIT_HIGHLIGHTED));
                                break;
                            }
                            case DigitHighlight::Conflict: {
                                painter.setBrush(QBrush(DIGIT_HIGHLIGHTED_CONFLICT));
                                break;
                            }
                        }
                        painter.drawEllipse(digit_pos, radius, radius);
                    }
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

auto SudokuCellWidget::digit() const -> std::optional<int> {
    if (this->is_clue()) {
        return std::get<Clue>(m_state).digit;
    } else if (this->is_entry()) {
        return std::get<Entry>(m_state).digit;
    }
    return std::optional<int>();
}

auto SudokuCellWidget::candidates() const -> std::optional<CellCandidates> {
    if ( std::holds_alternative<CellCandidates>(m_state)) {
        return std::get<CellCandidates>(m_state);
    }
    return std::optional<CellCandidates>();
}

auto SudokuCellWidget::try_set_possibility(int digit, bool is_possible) -> bool {
    if (!this->is_candidates()) {
        return false;
    }
    assert(0 < digit && digit < 10);
    auto &candidates = std::get<CellCandidates>(m_state);
    candidates[digit-1] = is_possible;
    return true;
}

auto SudokuCellWidget::set_clue(int digit) -> void {
    assert(0 < digit && digit < 10);
    m_state = Clue { .digit = digit };
}

// non-clue entry
auto SudokuCellWidget::try_set_entry(int digit) -> bool {
    if (!this->is_candidates()) {
        return false;
    }

    assert(0 < digit && digit < 10);
    m_state = Entry { .digit = digit };
}

auto SudokuCellWidget::clear() -> void {
    m_state = std::bitset<9>();
}

auto SudokuCellWidget::keyPressEvent(QKeyEvent *event) -> void {
    if (this->in_hint_mode()) {
        return;
    }

    // move with arrow keys
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

    // entries and clues are unalterable except by undoing
    if (!this->candidates()) {
        return;
    }
    auto candidates = this->candidates().value();

    // highest and lowest key to enter digits
    auto one = Qt::Key_1;
    auto nine = Qt::Key_9;

    // keys for toggling pencilmarks
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

    // <Space> and <Return> are dependent on the current highlighted digit
    // <Space> toggles pencilmark, <Return> enters digit
    auto highlighted_digit = m_grid->m_highlighted_digit;
    if (highlighted_digit != 0) {
        auto candidate = Candidate {
            .cell = m_cell_nr,
            .num = highlighted_digit,
        };
        if (event->key() == Qt::Key_Return) {
            m_grid->insert_candidate(candidate);
        } else if (event->key() == Qt::Key_Space) {
            auto is_possible = candidates[highlighted_digit-1];
            m_grid->set_candidate(candidate, !is_possible);
        }
    }

    if (one <= event->key() && event->key() <= nine) {
        uint8_t num = event->key() - Qt::Key_0;
        m_grid->insert_candidate(Candidate {
            .cell = m_cell_nr,
            .num = num,
        });
    } else {
        auto key_ptr = std::find(second_row.begin(), second_row.end(), event->key());

        if (key_ptr != second_row.end()) {
            int pos = std::distance(second_row.begin(), key_ptr);
            auto is_possible = candidates[pos];
            m_grid->set_candidate(
                Candidate {
                    .cell = m_cell_nr,
                    .num = static_cast<uint8_t>(pos+1),
                },
                !is_possible
            );
        }
    }
    this->update();
}

auto SudokuCellWidget::reset_hint_highlights() -> void {
    m_candidates_highlights = {};
    m_hint_mode = HintHighlight::None;
}

auto SudokuCellWidget::set_digit_highlight(int digit, bool is_conflict) -> void {
    auto& highlight = m_candidates_highlights[digit];
    if (is_conflict) {
       highlight = DigitHighlight::Conflict;
    } else {
        highlight = DigitHighlight::Regular;
    }
}

auto SudokuCellWidget::in_hint_mode() const -> bool {
    m_grid->in_hint_mode();
}


auto SudokuCellWidget::contains_highlighted_digit() const -> bool {
    auto maybe_candidates = this->candidates();
    if (!maybe_candidates) {
        return false;
    }
    auto candidates = *maybe_candidates;
    return candidates[m_grid->m_highlighted_digit - 1];
}
