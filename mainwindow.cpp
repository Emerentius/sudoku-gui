#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sudoku_grid_widget.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->resize(1000, 1000);
    new SudokuGridWidget(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
