#include "MainWindow.h"
#include "WelcomePage.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setupUi();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUi()
{
    m_stackedWidget = new QStackedWidget(this);

    m_welcomePage = new WelcomePage(this);
    m_stackedWidget->addWidget(m_welcomePage);

    // 暂时只加 WelcomePage，后面再加 Dashboard 和 Management

    setCentralWidget(m_stackedWidget);
    resize(800, 600);
}