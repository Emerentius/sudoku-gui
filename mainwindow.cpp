#include <QSignalMapper>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sudoku_grid_widget.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // forward button press signals from the digit selectors to the sudoku grid
    // but map signals to the corresponding digit of the button
    auto mapper = new QSignalMapper(this);

    QToolButton *buttons[] = {
        ui->digit_button_1,
        ui->digit_button_2,
        ui->digit_button_3,
        ui->digit_button_4,
        ui->digit_button_5,
        ui->digit_button_6,
        ui->digit_button_7,
        ui->digit_button_8,
        ui->digit_button_9,
    };

    for (int digit = 1; digit < 10; digit++) {
        auto button = buttons[digit-1];
        mapper->setMapping(button, digit);
        connect(button, SIGNAL(clicked(bool)),
                mapper, SLOT(map()));
    }

    connect(mapper, SIGNAL(mapped(int)),
            ui->sudoku_grid, SLOT(highlight_digit(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}
