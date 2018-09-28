#include <QAction>
#include <QActionGroup>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sudoku_grid_widget.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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

    // group actions so they uncheck each other
    auto action_group = new QActionGroup(this);

    for (int digit = 1; digit < 10; digit++) {
        auto button = buttons[digit-1];
        auto action = new QAction(action_group);

        action->setCheckable(true);
        action->setIconText(button->text());

        auto shortcut = QKeySequence(Qt::ALT + Qt::Key_0 + digit);
        action->setShortcut(shortcut);

        button->setDefaultAction(action);

        connect(action, &QAction::triggered,
            [this, digit]() { this->ui->sudoku_grid->highlight_digit(digit); }
        );
    }

    // hook hint button up to hint slot
    connect(ui->hint_button, &QPushButton::clicked,
            ui->sudoku_grid, &SudokuGridWidget::hint);
}

MainWindow::~MainWindow()
{
    delete ui;
}
