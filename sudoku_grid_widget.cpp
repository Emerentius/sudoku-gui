#include "sudoku_grid_widget.h"
#include "sudoku_cell_widget.h"
#include "sudoku_ffi/src/sudoku_ffi/sudoku.h"

const int MAJOR_LINE_SIZE = 6;
const int MINOR_LINE_SIZE = 2;

// quadratic
const int sudoku_size = 900;
const int x_off = 30;
const int y_off = 30;
const int cell_size = 96;

auto line(QWidget *parent, QFrame::Shape shape) -> QFrame* {
    auto frame = new QFrame(parent);
    auto size_policy = QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    size_policy.setHorizontalStretch(0);
    size_policy.setVerticalStretch(0);
    frame->setSizePolicy(size_policy);

    frame->setFrameShape(shape);
    frame->setFrameShadow(QFrame::Plain);
    frame->setLineWidth(MAJOR_LINE_SIZE);
    return frame;
}

auto minor_line(QWidget* parent, QFrame::Shape shape) -> QFrame* {
    auto frame = line(parent, shape);
    frame->setLineWidth(MINOR_LINE_SIZE);
    return frame;
}

SudokuGridWidget::SudokuGridWidget(QWidget *parent) : QFrame(parent) {
    {
        this->setGeometry(QRect(x_off, y_off, sudoku_size, sudoku_size));
        auto size_policy = QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        size_policy.setHorizontalStretch(0);
        size_policy.setVerticalStretch(0);
        size_policy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());

        this->setSizePolicy(size_policy);
        this->setFrameShape(QFrame::Box);
        this->setFrameShadow(QFrame::Plain);
        this->setLineWidth(MAJOR_LINE_SIZE);
    }

    // create grid of lines
    // using QFrame borders for lines
    // may wanna switch to QLine
    this->generate_major_lines();
    this->generate_minor_lines();

    this->generate_cells();

    m_sudoku = sudoku_generate_unique();
    this->set_clues();
    this->compute_candidates();
};


auto SudokuGridWidget::generate_major_lines() -> void {
    for (int i = 1; i < 3; i++) {
        auto offset = i * (MAJOR_LINE_SIZE + 3 * cell_size + 2 * MINOR_LINE_SIZE);
        auto hframe = new QFrame(this);
        auto hline = line(hframe, QFrame::HLine);
        hline->setGeometry(
            QRect(MAJOR_LINE_SIZE, offset, sudoku_size, MAJOR_LINE_SIZE)
        );

        auto vline = line(this, QFrame::VLine);
        vline->setGeometry(
            QRect(offset, MAJOR_LINE_SIZE, MAJOR_LINE_SIZE, sudoku_size)
        );
    }
}

auto SudokuGridWidget::generate_minor_lines() -> void {
    auto pos = 0;
    for (int i = 0; i < 9; i++ ) {
        pos += cell_size + (i % 3 == 0 ? MAJOR_LINE_SIZE : MINOR_LINE_SIZE);
        auto hline = minor_line(this, QFrame::HLine);
        hline->setGeometry(QRect(0, pos, sudoku_size, MINOR_LINE_SIZE));

        auto vline = minor_line(this, QFrame::VLine);
        vline->setGeometry(QRect(pos, 0, MINOR_LINE_SIZE, sudoku_size));
    }
}

auto SudokuGridWidget::generate_cells() -> void {
    std::vector<SudokuCellWidget*> cells;
    auto y_pos = 0;
    for (int y_idx = 0; y_idx < 9; y_idx++) {
        y_pos += y_idx % 3 == 0 ? MAJOR_LINE_SIZE : MINOR_LINE_SIZE;

        auto x_pos = 0;
        for (int x_idx = 0; x_idx < 9; x_idx++) {
            x_pos += x_idx % 3 == 0 ? MAJOR_LINE_SIZE : MINOR_LINE_SIZE;
            auto cell = new SudokuCellWidget(cell_size, y_idx*9 + x_idx, this);
            cell->move(x_pos, y_pos);
            cell->show();
            cells.push_back(cell);

            x_pos += cell_size;
        }

        y_pos += cell_size;
    }

    m_cells = cells;
}

// Only for the initial round!
// It sets all currently known entries as clues
auto SudokuGridWidget::set_clues() -> void {
    auto cell_ptr = sudoku_as_ptr(&m_sudoku);
    for (int cell = 0; cell < 81; cell++) {
        //auto& c = m_cells[cell];
        if (*cell_ptr != 0) {
            m_cells[cell]->set_clue(*cell_ptr);
        }
        cell_ptr++;
    }
}

auto SudokuGridWidget::compute_candidates() -> void {
    auto solver = strategy_solver_new(m_sudoku);
    auto cell_ptr = sudoku_as_ptr(&m_sudoku);
    for (int cell = 0; cell < 81; cell++) {
        if (*cell_ptr == 0) {
            auto& c = m_cells[cell];
            auto candidates = strategy_solver_cell_candidates(solver, cell);
            for (int dig = 1; dig < 10; dig++) {
                if (candidates & 1) {
                    c->set_possibility(dig, true);
                }
                candidates >>= 1;
            }
        }
        cell_ptr++;
    }
}

auto SudokuGridWidget::recompute_candidates() -> void {
    auto solver = strategy_solver_new(m_sudoku);
    auto cell_ptr = sudoku_as_ptr(&m_sudoku);
    for (int cell = 0; cell < 81; cell++) {
        if (*cell_ptr == 0) {
            auto& c = m_cells[cell];
            auto candidates = strategy_solver_cell_candidates(solver, cell);
            for (int dig = 1; dig < 10; dig++) {
                if ((candidates & 1) == 0) {
                    c->set_possibility(dig, false);
                }
                candidates >>= 1;
            }
        }
        cell_ptr++;
    }
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
