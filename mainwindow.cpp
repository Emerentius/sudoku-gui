#include <QtCore>
#include <QRect>
#include "sudoku_cell_widget.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <vector>
#include <assert.h>

const int MAJOR_LINE_SIZE = 6;
const int MINOR_LINE_SIZE = 2;

QFrame* line(QWidget* parent, QFrame::Shape shape) {
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


QFrame* minor_line(QWidget* parent, QFrame::Shape shape) {
    auto frame = line(parent, shape);
    frame->setLineWidth(MINOR_LINE_SIZE);
    return frame;
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->resize(1000, 1000);

    // quadratic
    auto sudoku_size = 900;
    auto x_off = 30;
    auto y_off = 30;
    auto cell_size = 96;

    {
        auto frame = new QFrame(this);
        frame->setGeometry(QRect(x_off, y_off, sudoku_size, sudoku_size));
        auto size_policy = QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        size_policy.setHorizontalStretch(0);
        size_policy.setVerticalStretch(0);
        size_policy.setHeightForWidth(frame->sizePolicy().hasHeightForWidth());

        frame->setSizePolicy(size_policy);
        frame->setFrameShape(QFrame::Box);
        frame->setFrameShadow(QFrame::Plain);
        frame->setLineWidth(MAJOR_LINE_SIZE);

        m_sudoku_grid = frame;
        assert(m_sudoku_grid);
    }

    // create grid of lines
    //auto line_off = sudoku_size / 3;

    // using QFrame borders for lines
    // may wanna switch to QLine
    std::vector<QFrame*> major_hor_lines;
    std::vector<QFrame*> major_ver_lines;

    for (int i = 1; i < 3; i++) {
        auto offset = i * (MAJOR_LINE_SIZE + 3 * cell_size + 2 * MINOR_LINE_SIZE);
        auto hframe = new QFrame(m_sudoku_grid);
        auto hline = line(hframe, QFrame::HLine);
        hline->setGeometry(
            QRect(MAJOR_LINE_SIZE, offset, sudoku_size, MAJOR_LINE_SIZE)
        );
        major_hor_lines.push_back(hline);

        auto vline = line(m_sudoku_grid, QFrame::VLine);
        vline->setGeometry(
            QRect(offset, MAJOR_LINE_SIZE, MAJOR_LINE_SIZE, sudoku_size)
        );
        major_ver_lines.push_back(vline);
    }

    std::vector<QFrame*> minor_hor_lines;
    std::vector<QFrame*> minor_ver_lines;

    auto pos = 0;
    for (int i = 0; i < 9; i++ ) {
        pos += cell_size + (i % 3 == 0 ? MAJOR_LINE_SIZE : MINOR_LINE_SIZE);
        auto hline = minor_line(m_sudoku_grid, QFrame::HLine);
        hline->setGeometry(QRect(0, pos, sudoku_size, MINOR_LINE_SIZE));
        minor_hor_lines.push_back(hline);

        auto vline = minor_line(m_sudoku_grid, QFrame::VLine);
        vline->setGeometry(QRect(pos, 0, MINOR_LINE_SIZE, sudoku_size));
        minor_ver_lines.push_back(vline);
    }

    std::vector<SudokuCellWidget*> cells;
    auto x_pos = 0;

    for (int x_idx = 0; x_idx < 9; x_idx++) {
        x_pos += x_idx % 3 == 0 ? MAJOR_LINE_SIZE : MINOR_LINE_SIZE;

        auto y_pos = 0;
        for (int y_idx = 0; y_idx < 9; y_idx++) {
            y_pos += y_idx % 3 == 0 ? MAJOR_LINE_SIZE : MINOR_LINE_SIZE;

            auto cell = new SudokuCellWidget(cell_size, m_sudoku_grid);
            cell->move(x_pos, y_pos);
            //cell->lower();
            cell->show();
            cells.push_back(cell);

            y_pos += cell_size;
        }

        x_pos += cell_size;
    }
    /*
    for (auto line : major_hor_lines) {
        //line->raise();
    }
    for (auto line : major_ver_lines) {
        //line->raise();
    }
    for (auto line : minor_hor_lines) {
        //line->raise();
    }
    for (auto line : minor_ver_lines) {
        //line->raise();
    }
    //m_sudoku_grid->raise();
    */
    //this->update();

}

MainWindow::~MainWindow()
{
    delete ui;
}
