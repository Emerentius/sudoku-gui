#include "mainwindow.h"
#include "sudoku_grid_widget.h"
#include "ui_mainwindow.h"
#include <QAction>
#include <QActionGroup>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // clang-format off
    QToolButton *buttons[] = {
        ui->digit_button_off,
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
    // clang-format on

    // group actions so they uncheck each other
    auto action_group = new QActionGroup(this);

    for (int digit = 0; digit < 10; digit++) {
        auto button = buttons[digit];
        auto action = new QAction(action_group);

        action->setCheckable(true);
        action->setIconText(button->text());

        auto shortcut = QKeySequence((int) Qt::ALT + (int) Qt::Key_0 + digit);
        action->setShortcut(shortcut);

        button->setDefaultAction(action);

        connect(action, &QAction::triggered, [this, digit]() { this->ui->sudoku_grid->highlight_digit(digit); });
    }

    // associate hint buttons with their respective strategies
    // TODO: store this somewhere, possibly in a new widget for the strategy selector
    //       and clean up on destruction
    auto button_strategies = std::array<std::pair<QToolButton*, Strategy>, 12>({
        std::make_pair(ui->strategy_naked_singles, Strategy::NakedSingles),
        std::make_pair(ui->strategy_hidden_singles, Strategy::HiddenSingles),

        std::make_pair(ui->strategy_locked_candidates, Strategy::LockedCandidates),

        std::make_pair(ui->strategy_naked_pairs, Strategy::NakedPairs),
        std::make_pair(ui->strategy_naked_triples, Strategy::NakedTriples),
        std::make_pair(ui->strategy_naked_quads, Strategy::NakedQuads),

        std::make_pair(ui->strategy_hidden_pairs, Strategy::HiddenPairs),
        std::make_pair(ui->strategy_hidden_triples, Strategy::HiddenTriples),
        std::make_pair(ui->strategy_hidden_quads, Strategy::HiddenQuads),

        std::make_pair(ui->strategy_x_wing, Strategy::XWing),
        std::make_pair(ui->strategy_swordfish, Strategy::Swordfish),
        std::make_pair(ui->strategy_jellyfish, Strategy::Jellyfish),
    });

    auto hint_strategies = [=, this]() {
        std::vector<Strategy> strategies;

        for (auto& pair : button_strategies) {
            auto* button = pair.first;
            auto strategy = pair.second;

            if (button->isChecked()) {
                strategies.push_back(strategy);
            }
        }

        ui->sudoku_grid->hint(strategies);
    };

    // Hook up actions
    // hint
    auto hint_action = ui->action_hint;
    ui->hint_button->setDefaultAction(hint_action);

    connect(hint_action, &QAction::triggered, hint_strategies);


    // new sudoku
    connect(ui->action_new_sudoku, &QAction::triggered, [this]() { ui->sudoku_grid->generate_new_sudoku(); });

    // undo
    connect(ui->action_undo, &QAction::triggered, [this]() { ui->sudoku_grid->undo(); });

    // redo
    connect(ui->action_redo, &QAction::triggered, [this]() { ui->sudoku_grid->redo(); });
}

MainWindow::~MainWindow() {
    delete ui;
}
