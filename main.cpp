#include "database.h"
#include "mainmenu.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(R"(
    QWidget {
        font-family: "Microsoft YaHei";
        font-size: 15px;
        color: #1f2937;
        background-color: #f5f7fb;
    }

    QLabel#MainMenuTitle {
        font-size: 30px;
        font-weight: bold;
        color: #10233f;
    }

    QMainWindow {
        background-color: #f5f7fb;
    }

    QDialog {
        background-color: #f5f7fb;
    }

    QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QDateEdit {
        background-color: white;
        border: 1px solid #d0d7de;
        border-radius: 6px;
        padding: 5px 8px;
        min-height: 26px;
    }

    QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus, QDateEdit:focus {
        border: 1px solid #3b82f6;
    }

    QPushButton {
        background-color: white;
        border: 1px solid #cfd8e3;
        border-radius: 7px;
        padding: 6px 16px;
        min-height: 26px;
    }

    QPushButton:hover {
        background-color: #eaf2ff;
        border: 1px solid #60a5fa;
        color: #1d4ed8;
    }

    QPushButton:pressed {
        background-color: #dbeafe;
    }

    QTabWidget::pane {
        border: 1px solid #d7dee8;
        background-color: white;
        border-radius: 6px;
        top: -1px;
    }

    QTabBar::tab {
        background-color: #eef2f7;
        border: 1px solid #d7dee8;
        border-bottom: none;
        padding: 8px 18px;
        border-top-left-radius: 7px;
        border-top-right-radius: 7px;
        margin-right: 2px;
    }

    QTabBar::tab:selected {
        background-color: white;
        color: #2563eb;
        font-weight: bold;
    }

    QTableView {
        background-color: white;
        alternate-background-color: #f8fafc;
        border: 1px solid #d7dee8;
        border-radius: 6px;
        gridline-color: #e5e7eb;
        selection-background-color: #dbeafe;
        selection-color: #111827;
    }

    QHeaderView::section {
        background-color: #eff6ff;
        color: #1e3a8a;
        font-weight: bold;
        border: none;
        border-right: 1px solid #d7dee8;
        border-bottom: 1px solid #d7dee8;
        padding: 7px;
        min-height: 28px;
    }

    QToolBar {
        background-color: #ffffff;
        border-bottom: 1px solid #d7dee8;
        spacing: 8px;
        padding: 6px;
    }

    QToolButton {
        background-color: transparent;
        border: none;
        padding: 6px 12px;
        border-radius: 6px;
    }

    QToolButton:hover {
        background-color: #eaf2ff;
        color: #1d4ed8;
    }

    QLabel {
        background-color: transparent;
    }
)");

    if (!initDatabase()) {
        return 1;
    }

    MainMenu w;
    w.show();

    return app.exec();
}
