#include "sudoku_cell_widget.h"
#include "sudoku_ffi/sudoku.h"
#include "sudoku_grid_widget.h"
#include "sudoku_helper.h"
#include <QDebug>
#include <QGridLayout>
#include <optional>

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

    this->generate_new_sudoku();
};

auto SudokuGridWidget::cell_state(uint8_t cell) const -> const CellWidgetState& {
    return this->sudoku_state()[cell];
}

auto SudokuGridWidget::reset() -> void {
    m_stack_position = 0;
    m_sudoku.resize(0);

    m_in_hint_mode = false;
    m_hint_candidate = {};
    m_hint_conflicts = {};
    m_highlighted_digit = 0;
}

auto SudokuGridWidget::generate_new_sudoku() -> void {
    this->reset();
    auto sudoku = sudoku_generate_unique();
    auto& grid_state = m_sudoku.emplace_back();

    uint8_t cell = 0;
    for (auto& cell_state : grid_state) {
        auto digit = sudoku._0[cell];
        if (digit != 0) {
            cell_state = Clue { .digit = digit };
        } else {
            cell_state = CellCandidates().set();
        }
        cell++;
    }

    this->recompute_candidates();
}

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
            outer_layout->addLayout(inner_layout, row, col);
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
        inner_layouts[box]->addWidget(cell_widget, minirow, minicol);
    }
}

auto SudokuGridWidget::sudoku_state() -> GridWidgetState& {
    return m_sudoku[m_stack_position];
}

auto SudokuGridWidget::sudoku_state() const -> const GridWidgetState& {
    return m_sudoku[m_stack_position];
}

auto SudokuGridWidget::grid_state() const -> GridState {
    GridState grid_state{};
    const auto& sudoku_state = this->sudoku_state();
    for (int cell = 0; cell < 81; cell++) {
        auto const cell_widget_state = sudoku_state[cell];
        auto &cell_state = grid_state.grid[cell];

        if (std::holds_alternative<CellCandidates>(cell_widget_state)) {
            auto candidates = std::get<CellCandidates>(cell_widget_state);
            cell_state.tag = CellState::Tag::Candidates;
            cell_state.candidates._0 = candidates.to_ulong();
        } else {
            uint8_t digit;
            if (std::holds_alternative<Clue>(cell_widget_state)) {
                digit = std::get<Clue>(cell_widget_state).digit;
            } else {
                digit = std::get<Entry>(cell_widget_state).digit;
            }
            cell_state.tag = CellState::Tag::Digit;
            cell_state.digit._0 = digit;
        }
    }
    return grid_state;
}

auto SudokuGridWidget::recompute_candidates() -> void {
    auto solver = this->strategy_solver();

    auto& sudoku_state = this->sudoku_state();
    for (int cell = 0; cell < 81; cell++) {
        auto& cell_widget_state = sudoku_state[cell];

        // skip clues, the GridState does not distinguish between clue and entry
        if (!std::holds_alternative<CellCandidates>(cell_widget_state)) {
            continue;
        }

        auto& candidates = std::get<CellCandidates>(cell_widget_state);
        auto solver_candidates = strategy_solver_cell_candidates(solver, cell);
        candidates &= CellCandidates(solver_candidates);
    }

    this->update();
}

auto SudokuGridWidget::move_focus(int current_cell, Direction direction) -> void {
    auto row = current_cell / 9;
    auto col = current_cell % 9;

    switch (direction) {
        case Direction::Left:  if (col > 0) { col--; } break;
        case Direction::Right: if (col < 8) { col++; } break;
        case Direction::Up:    if (row > 0) { row--; } break;
        case Direction::Down:  if (row < 8) { row++; } break;
    };

    auto n_cell = row * 9 + col;
    m_cells[n_cell]->setFocus();
}

// Truncate the undo stack to the current position and push a copy of the current state
auto SudokuGridWidget::push_savepoint() -> void {
    // if the stack contains remnants from undo
    // delete them
    m_sudoku.resize(m_stack_position+1);

    auto state = this->sudoku_state();
    m_sudoku.push_back(state);

    m_stack_position++;
}

auto SudokuGridWidget::undo() -> bool {
    if (m_in_hint_mode) {
        return false;
    }

    if (m_stack_position > 0) {
        m_stack_position--;
        this->update();
        return true;
    }
    return false;
}

auto SudokuGridWidget::redo() -> bool {
    if (m_in_hint_mode) {
        return false;
    }
    if (m_stack_position + 1 < m_sudoku.size()) {
        m_stack_position++;
        this->update();
        return true;
    }
    return false;
}


auto SudokuGridWidget::insert_candidate(Candidate candidate) -> void {
    this->push_savepoint();

    auto& grid_state = this->sudoku_state();
    auto& cell_state = grid_state[candidate.cell];
    // cell is already filled, don't do anything
    if (!std::holds_alternative<CellCandidates>(cell_state)) {
        return;
    }

    cell_state = Entry { .digit = candidate.num };
    this->recompute_candidates();
}

// Store savepoint and set candidate
auto SudokuGridWidget::set_candidate(Candidate candidate, bool is_possible) -> void {
    this->push_savepoint();
    this->_set_candidate(candidate, is_possible);
    this->update();
}

// set candidate in storage and cell, don't create a savepoint
auto SudokuGridWidget::_set_candidate(Candidate candidate, bool is_possible) -> void {
    auto& cell_state = this->sudoku_state()[candidate.cell];
    if (!std::holds_alternative<CellCandidates>(cell_state)) {
        return;
    }
    auto& cell_cands = std::get<CellCandidates>(cell_state);
    cell_cands &= ~( 1u << (candidate.num - 1));
    cell_cands |= (uint16_t) is_possible << (candidate.num - 1);
}

auto SudokuGridWidget::highlight_digit(int digit) -> void {
    assert(digit >= 0 && digit < 10);
    m_highlighted_digit = digit;
    this->update();
}

auto SudokuGridWidget::hint(std::vector<Strategy> strategies) -> void {
    if (m_in_hint_mode) {
        // insert the results of the hint
        if (m_hint_candidate.has_value()) {
            this->insert_candidate(*m_hint_candidate);
            m_hint_candidate = {};
        }
        if (m_hint_conflicts.has_value()) {
            this->push_savepoint();

            auto len = conflicts_len(*m_hint_conflicts);
            for (int i = 0; i < len; i++) {
                auto conflict = conflicts_get(*m_hint_conflicts, i);
                this->_set_candidate(conflict, false);
            }
            m_hint_conflicts = {};
            this->recompute_candidates();
        }

        // reset out of hint mode
        m_in_hint_mode = false;
        for (auto *cell : m_cells) {
            cell->reset_hint_highlights();
            cell->update();
        }
        return;
    }

    auto solver = this->strategy_solver();
    auto results = strategy_solver_solve(solver, strategies.data(), strategies.size());
    auto deductions = results.deductions;
    auto n_deductions = deductions_len(deductions);

    qDebug() << n_deductions;
    if (n_deductions == 0) {
        return; // nothing found, don't change anything
    }

    // find and mark cell
    // also give a lighter highlight to all cells in the same line or col
    // to guide the eyes
    auto deduction = deductions_get(deductions, 0);

    qDebug() << (int) deduction.tag << "\n";

    switch (deduction.tag) {
        case DeductionTag::NakedSingles: {
            auto candidate = deduction.data.naked_singles.candidate;
            auto cell = candidate.cell;
            auto row = cell / 9;
            auto col = cell % 9;
            this->set_house_highlight(row, HintHighlight::Weak);
            this->set_house_highlight(col+9, HintHighlight::Weak);
            this->set_cell_highlight(cell, HintHighlight::Strong);

            m_hint_candidate = candidate;
            break;
        }
        case DeductionTag::HiddenSingles: {
            auto data = deduction.data.hidden_singles;
            auto candidate = data.candidate;
            auto house = house_of_cell(candidate.cell, data.house_type);
            this->set_house_highlight(house, HintHighlight::Weak);
            this->set_cell_highlight(candidate.cell, HintHighlight::Strong);

            m_hint_candidate = candidate;
            break;
        }
        case DeductionTag::LockedCandidates: {
            auto data = deduction.data.locked_candidates;
            auto digit = data.digit;
            auto miniline = data.miniline;

            auto block = block_of_miniline(miniline);
            auto line = line_of_miniline(miniline);

            this->set_house_highlight(block+18, HintHighlight::Weak);
            this->set_house_highlight(line, HintHighlight::Weak);

            auto set_digit_highlights = [&](int cell) {
                    m_cells[cell]->set_digit_highlight(digit-1, false);
                };
            if (data.is_pointing) {
                foreach_cell_in_block(block, set_digit_highlights);
            } else {
                foreach_cell_in_house(line, set_digit_highlights);
            }

            m_hint_conflicts = data.conflicts;
            break;
        }
        case DeductionTag::Subsets: {
            auto data = deduction.data.subsets;
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
        case DeductionTag::Wing: {
            auto data = deduction.data.wing;
            auto digits = std::bitset<9>(data.hinge_digits);
            auto pincer_cells_lower = std::bitset<81>{data.pincers[0]};
            auto pincer_cells_upper = std::bitset<81>{data.pincers[1]};
            auto pincer_cells = pincer_cells_upper << 64 | pincer_cells_lower;
            m_hint_conflicts = data.conflicts;

            auto affected_cells = std::vector { data.hinge };

            for (size_t cell = 0; cell < 81; cell++) {
                if (!pincer_cells[cell]) {
                    continue;
                }

                affected_cells.push_back(cell);
            }

            for (auto pattern_cell : affected_cells) {
                set_cell_highlight(pattern_cell, HintHighlight::Strong);
                for (size_t digit = 0; digit < 9; digit++) {
                    if (!digits[digit]) {
                        continue;
                    }
                    m_cells[pattern_cell]->set_digit_highlight(digit, false);
                }
            }
            break;
        }
        /*
        case DeductionTag::Fish: {
            auto data = deduction.data.fish;
            break;
        }
        case DeductionTag::AvoidableRectangle: {
            auto data = deduction.data.avoidable_rectangle;
            break;
        }
        */
        default:
            break;
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
        cell->update();
    }

}

auto SudokuGridWidget::in_hint_mode() const -> bool {
    return m_in_hint_mode;
}

auto SudokuGridWidget::strategy_solver() const -> StrategySolver {
    return strategy_solver_from_grid_state(this->grid_state());
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
