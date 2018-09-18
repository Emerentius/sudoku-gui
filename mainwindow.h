#pragma once
#include <QMainWindow>
#include <QFrame>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QFrame *m_sudoku_grid;

public:
    explicit MainWindow(QWidget *parent = 0);
    auto setupUi(QWidget *widget);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
};
