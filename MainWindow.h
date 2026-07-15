#pragma once
#pragma once
#include <QMainWindow>
#include <QStackedWidget>

class WelcomePage;
class DashboardPage;
class ManagementPage;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    void setupUi();

    QStackedWidget* m_stackedWidget;
    WelcomePage* m_welcomePage;
    // DashboardPage *m_dashboardPage;   // 后面再加
    // ManagementPage *m_managementPage; // 后面再加
};