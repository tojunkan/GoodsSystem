#include "managerwindow.h"
#include "productmanagerpage.h"
#include "employeemanagerpage.h"
#include "salesrecordpage.h"

#include <QtWidgets>

ManagerWindow::ManagerWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("经理管理后台");
    resize(1050, 680);

    auto *toolbar = addToolBar("操作");
    toolbar->setMovable(false);
    auto *logoutAction = toolbar->addAction("返回主菜单");
    connect(logoutAction, &QAction::triggered, this, &QWidget::close);

    auto *tabs = new QTabWidget;
    setCentralWidget(tabs);

    productPage = new ProductManagerPage;
    employeePage = new EmployeeManagerPage;
    salesPage = new SalesRecordPage;

    tabs->addTab(productPage, "管理商品");
    tabs->addTab(employeePage, "管理员工");
    tabs->addTab(salesPage, "销售情况");

    connect(tabs, &QTabWidget::currentChanged, this, [=](int idx){
        if (tabs->widget(idx) == employeePage) {
            employeePage->refresh();
        }

        if (tabs->widget(idx) == salesPage) {
            salesPage->loadEmployees();
            salesPage->queryRecords();
        }

        if (tabs->widget(idx) == productPage) {
            productPage->refreshAll();
        }
    });
}
