#include <assert.h>
#include <optional>
#include <QGridLayout>
#include <QDebug>
#include "sudoku_grid_widget.h"
#include "sudoku_cell_widget.h"
#include "sudoku_ffi/src/sudoku_ffi/sudoku.h"
#include "sudoku_helper.h"

const int MAJOR_LINE_SIZE = 6;
const int MINOR_LINE_SIZE = 2;


SudokuGridWidget::SudokuGridWidget(QWidget *parent) : QuadraticQFrame(parent) {
    this->initialize_cells();
    this->generate_layout();

    // set background to black
    // it will cause the appearance of black lines in combination
    // with the free space around the cells from the layout
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::black);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    // generate random sudoku and initialize the undo-stack
    m_sudoku.push_back(sudoku_generate_unique());
    m_candidates.push_back(Candidates());

    this->set_clues();
    this->compute_candidates();
    this->update_cells();
};

auto SudokuGridWidget::initialize_cells() -> void {
    for (int n_cell = 0; n_cell < 81; n_cell++) {
        m_cells[n_cell] = new SudokuCellWidget(n_cell, this);
    }
}

// assumes initialize_cells was already called
auto SudokuGridWidget::generate_layout() -> void {
    auto *outer_layout = new QGridLayout(this);
    outer_layout->setMargin(0);
    outer_layout->setHorizontalSpacing(MAJOR_LINE_SIZE);
    outer_layout->setVerticalSpacing(MAJOR_LINE_SIZE);

    std::vector<QGridLayout*> inner_layouts;
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            // TODO: Find out whether these layouts are cleaned up by the outer layout
            auto *inner_layout = new QGridLayout();
            inner_layout->setMargin(0);
            inner_layout->setHorizontalSpacing(MINOR_LINE_SIZE);
            inner_layout->setVerticalSpacing(MINOR_LINE_SIZE);

            inner_layouts.push_back(inner_layout);
            outer_layout->addLayout(inner_layout, row, col, 0); // 4th arg is alignment, 0 means stretch
        }
    }

    for (int cell = 0; cell < 81; cell++) {
        auto row = cell / 9;
        auto col = cell % 9;
        auto band = row / 3;
        auto stack = col / 3;
        auto box = band*3 + stack;
        auto minirow = row % 3;
        auto minicol = col % 3;

        auto *cell_widget = m_cells[cell];
        inner_layouts[box]->addWidget(cell_widget, minirow, minicol, 0);
    }
}

auto SudokuGridWidget::current_sudoku() -> Sudoku& {
    return m_sudoku.back();
}

auto SudokuGridWidget::current_candidates() -> Candidates& {
    return m_candidates.back();
}

auto SudokuGridWidget::sudoku() const -> Sudoku {
    return m_sudoku.back();
}

auto SudokuGridWidget::grid_state() const -> GridState {
    GridState grid_state;
    auto sudoku = this->sudoku();
    for (int cell = 0; cell < 81; cell++) {
        auto digit = sudoku._0[cell];
        auto &cell_state = grid_state.grid[cell];
        if (digit != 0) {
            cell_state.tag = CellState::Tag::Digit;
            cell_state.digit._0 = digit;
        } else {
            cell_state.tag = CellState::Tag::Candidates;
            cell_state.candidates._0 = m_candidates.back()[cell];
        }
    }
    return grid_state;
}

// Only for the initial round!
// It sets all currently known entries as clues
auto SudokuGridWidget::set_clues() -> void {
    auto& sudoku = this->current_sudoku();
    auto cell_ptr = sudoku_as_ptr(&sudoku);
    for (int cell = 0; cell < 81; cell++) {
        //auto& c = m_cells[cell];
        if (*cell_ptr != 0) {
            m_cells[cell]->set_clue(*cell_ptr);
        }
        cell_ptr++;
    }
}

auto SudokuGridWidget::compute_candidates() -> void {
    auto solver = strategy_solver_new(this->sudoku());
    auto& sudoku = this->current_sudoku();
    auto cell_ptr = sudoku_as_ptr(&sudoku);
    auto& candidates = m_candidates.back();
    for (int cell = 0; cell < 81; cell++) {
        if (*cell_ptr == 0) {
            auto solver_candidates = strategy_solver_cell_candidates(solver, cell);
            candidates[cell] = solver_candidates;
        }
        cell_ptr++;
    }
}

auto SudokuGridWidget::recompute_candidates() -> void {
    auto solver = strategy_solver_new(this->sudoku());
    auto cell_ptr = sudoku_as_ptr(&this->current_sudoku());
    auto& candidates = m_candidates.back();
    for (int cell = 0; cell < 81; cell++) {
        if (*cell_ptr == 0) {
            auto& c = m_cells[cell];
            auto solver_candidates = strategy_solver_cell_candidates(solver, cell);
            candidates[cell] &= solver_candidates;
        }
        cell_ptr++;
    }

    this->update_cells();
}

auto SudokuGridWidget::update_cells() -> void {
    auto sudoku = this->current_sudoku();
    auto cell_ptr = sudoku_as_ptr(&sudoku);
    auto& candidates = m_candidates.back();
    for (int cell = 0; cell < 81; cell++) {
        auto& c = m_cells[cell];
        if ( !(c->is_clue()) ) {
            c->clear();
            if (*cell_ptr == 0) {
                auto cell_candidates = candidates[cell];
                for (int dig = 1; dig < 10; dig++) {
                    c->try_set_possibility(dig, (cell_candidates & 1) == 1);
                    cell_candidates >>= 1;
                }
            } else {
                c->try_set_entry(*cell_ptr);
            }
        }
        cell_ptr++;
        c->update(); // currently superflous because update_highlights does it as well
    }

    this->update_highlights();
}

auto SudokuGridWidget::move_focus(int current_cell, Direction direction) -> void {
    auto row = current_cell / 9;
    auto col = current_cell % 9;

    switch (direction) {
        case Direction::Left:  if (col > 0) { col -= 1; } break;
        case Direction::Right: if (col < 8) { col += 1; } break;
        case Direction::Up:    if (row > 0) { row -= 1; } break;
        case Direction::Down:  if (row < 8) { row += 1; } break;
    };

    auto n_cell = row * 9 + col;
    m_cells[n_cell]->setFocus();
}

auto SudokuGridWidget::push_savepoint() -> void {
    auto sudoku = this->sudoku();
    auto candidates = m_candidates.back();
    m_sudoku.push_back(sudoku);
    m_candidates.push_back(candidates);
}

auto SudokuGridWidget::pop_savepoint() -> void {
    assert(m_candidates.size() > 0 && m_sudoku.size() > 0);
    m_sudoku.pop_back();
    m_candidates.pop_back();

    this->update_cells();
}

auto SudokuGridWidget::undo() -> bool {
    if (m_candidates.size() > 1) {
        this->pop_savepoint();
        return true;
    } else {
        return false;
    }
}

auto SudokuGridWidget::insert_candidate(Candidate candidate) -> void {
    this->push_savepoint();

    auto& sudoku = this->current_sudoku()._0;
    sudoku[candidate.cell] = candidate.num;

    m_cells[candidate.cell]->try_set_entry(candidate.num);
    this->recompute_candidates();
}

auto SudokuGridWidget::set_candidate(Candidate candidate, bool is_possible) -> void {
    this->push_savepoint();

    auto& cands = this->current_candidates();
    auto& cell_cands = cands[candidate.cell];
    cell_cands &= ~( 1 << candidate.num - 1);
    cell_cands |= (uint16_t) is_possible << candidate.num - 1;
    m_cells[candidate.cell]->try_set_possibility(candidate.num, is_possible);
    this->update_highlights();
}

auto SudokuGridWidget::highlight_digit(int digit) -> void {
    assert(digit >= 0 && digit < 10);
    m_highlighted_digit = digit;
    this->update_highlights();
}

auto SudokuGridWidget::update_highlights() -> void {
    auto hl_digit = m_highlighted_digit;
    for (auto& cell : m_cells) {
        auto is_candidate = cell->candidates()[hl_digit - 1];
        cell->m_is_highlighted = is_candidate;
        cell->update();
    }
}

auto SudokuGridWidget::hint(std::vector<Strategy> strategies) -> void {
    if (m_in_hint_mode) {
        // insert the results of the hint
        if (m_hint_candidate.has_value()) {
            this->insert_candidate(*m_hint_candidate);
            m_hint_candidate = std::nullopt;
        }
        if (m_hint_conflicts.has_value()) {
            auto len = conflicts_len(*m_hint_conflicts);

            for (int i = 0; i < len; i++) {
                auto conflict = conflicts_get(*m_hint_conflicts, i);
                this->set_candidate(conflict, false);
            }
            m_hint_conflicts = std::nullopt;
        }

        // reset out of hint mode
        m_in_hint_mode = false;
        for (auto *cell : m_cells) {
            cell->m_in_hint_mode = false;
            cell->reset_highlights();
            cell->update();
        }
        return;
    }

    auto solver = strategy_solver_from_grid_state(this->grid_state());
    auto results = strategy_solver_solve(solver, strategies.data(), strategies.size());
    auto deductions = results.deductions;
    auto n_deductions = deductions_len(deductions);

    qDebug() << n_deductions << "\n";
    if (n_deductions == 0) {
        return; // nothing found, don't change anything
    }

    // find and mark cell
    // also give a lighter highlight to all cells in the same line or col
    // to guide the eyes
    auto deduction = deductions_get(deductions, 0);

    qDebug() << (int) deduction.tag << "\n";

    switch (deduction.tag) {
        case DeductionTag::NakedSingle: {
            auto candidate = deduction.data.naked_single.candidate;
            auto cell = candidate.cell;
            auto row = cell / 9;
            auto col = cell % 9;
            this->set_house_highlight(row, HintHighlight::Weak);
            this->set_house_highlight(col+9, HintHighlight::Weak);
            this->set_cell_highlight(cell, HintHighlight::Strong);

            m_hint_candidate = candidate;
            break;
        }
        case DeductionTag::HiddenSingle: {
            auto data = deduction.data.hidden_single;
            auto candidate = data.candidate;
            auto house = house_of_cell(candidate.cell, data.house_type);
            this->set_house_highlight(house, HintHighlight::Weak);
            this->set_cell_highlight(candidate.cell, HintHighlight::Strong);

            m_hint_candidate = candidate;
            break;
        }
        case DeductionTag::NakedSubset: {
            auto data = deduction.data.naked_subsets;
            this->set_house_highlight(data.house, HintHighlight::Weak);
            auto digits = std::bitset<9>(data.digits);
            auto positions = std::bitset<9>(data.positions);
            for (int pos = 0; pos < 9; pos++) {
                if (!positions[pos]) {
                    continue;
                }
                auto cell_nr = cell_at_position(data.house, pos);
                this->set_cell_highlight(cell_nr, HintHighlight::Strong);

                auto *cell = m_cells[cell_nr];
                for (int digit = 0; digit < 9; digit++) {
                    if (!digits[digit]) {
                        continue;
                    }
                    cell->set_digit_highlight(digit, false);
                }
            }

            m_hint_conflicts = data.conflicts;
            break;
        }
        // straight copy from naked subsets => deduplicate
        case DeductionTag::HiddenSubset: {
            auto data = deduction.data.hidden_subsets;
            this->set_house_highlight(data.house, HintHighlight::Weak);
            auto digits = std::bitset<9>(data.digits);
            auto positions = std::bitset<9>(data.positions);
            for (int pos = 0; pos < 9; pos++) {
                if (!positions[pos]) {
                    continue;
                }
                auto cell_nr = cell_at_position(data.house, pos);
                this->set_cell_highlight(cell_nr, HintHighlight::Strong);

                auto *cell = m_cells[cell_nr];
                for (int digit = 0; digit < 9; digit++) {
                    if (!digits[digit]) {
                        continue;
                    }
                    cell->set_digit_highlight(digit, false);
                }
            }

            m_hint_conflicts = data.conflicts;
            break;
        }
        case DeductionTag::BasicFish: {
            auto data = deduction.data.basic_fish;
            auto lines = std::bitset<18>(data.lines);
            auto positions = std::bitset<9>(data.positions);
            auto digit = data.digit;

            for (int line = 0; line < 18; line++) {
                if (!lines[line]) {
                    continue;
                }
                this->set_house_highlight(line, HintHighlight::Weak);
                for (int pos = 0; pos < 9; pos++) {
                    if (!positions[pos]) {
                        continue;
                    }
                    auto cell_nr = cell_at_position(line, pos);
                    this->set_cell_highlight(cell_nr, HintHighlight::Strong);
                    auto *cell = m_cells[cell_nr];
                    cell->set_digit_highlight(digit-1, false);
                }
            }

            m_hint_conflicts = data.conflicts;
            break;
        }
    }

    if (m_hint_candidate.has_value()) {
        auto candidate = *m_hint_candidate;
        m_cells[candidate.cell]->set_digit_highlight(candidate.num-1, false);
    }

    if (m_hint_conflicts.has_value()) {
        auto len = conflicts_len(*m_hint_conflicts);

        for (int i = 0; i < len; i++) {
            auto conflict = conflicts_get(*m_hint_conflicts, i);
            m_cells[conflict.cell]->set_digit_highlight(conflict.num - 1, true);
        }
    }

    // update the cells or nothing happens
    m_in_hint_mode = true;
    for (auto *cell : m_cells) {
        cell->m_in_hint_mode = true;
        cell->update();
    }

}

auto SudokuGridWidget::set_house_highlight(int house, HintHighlight highlight) -> void {
    assert(house < 27);
    if (house < 9) {
        // row
        auto limit = 9 * house + 9;
        for (int cell = house * 9; cell < limit; cell++) {
            set_cell_highlight(cell, highlight);
        }
    } else if (house < 18) {
        // col
        for (int cell = house - 9; cell < 81; cell += 9) {
            set_cell_highlight(cell, highlight);
        }
    } else {
        // block
        auto block = house - 18;
        auto band = block / 3;
        auto stack = block % 3;
        auto first_row = band * 3;
        auto first_col = stack * 3;
        for (int row = first_row; row < first_row + 3; row++) {
            for (int col = first_col; col < first_col + 3; col++) {
                auto cell = row*9 + col;
                set_cell_highlight(cell, highlight);
            }
        }
    }

}
auto SudokuGridWidget::set_cell_highlight(int cell, HintHighlight highlight) -> void {
    assert(cell < 81);
    m_cells[cell]->m_hint_mode = highlight;
}
