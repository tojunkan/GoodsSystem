#include "mainmenu.h"
#include "logindialog.h"
#include "registerdialog.h"
#include "managerwindow.h"
#include "employeesaleswindow.h"

#include <QtWidgets>

MainMenu::MainMenu(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("商品销售管理系统");
    resize(420, 260);

    auto *title = new QLabel("商品销售管理系统");

    title->setAlignment(Qt::AlignCenter);    
    title->setObjectName("MainMenuTitle");

    auto *btnManager = new QPushButton("我是经理");
    auto *btnSales = new QPushButton("我是销售");

    btnManager->setMinimumHeight(45);
    btnSales->setMinimumHeight(45);

    auto *main = new QVBoxLayout(this);
    main->addStretch();
    main->addWidget(title);
    main->addSpacing(20);
    main->addWidget(btnManager);
    main->addWidget(btnSales);
    main->addStretch();

    connect(btnManager, &QPushButton::clicked, this, &MainMenu::managerLogin);
    connect(btnSales, &QPushButton::clicked, this, &MainMenu::salesMenu);
}

void MainMenu::managerLogin()
{
    LoginDialog dlg(LoginDialog::Manager, this);
    if (dlg.exec() == QDialog::Accepted) {
        auto *w = new ManagerWindow;
        w->setAttribute(Qt::WA_DeleteOnClose);
        connect(w, &QObject::destroyed, this, &QWidget::show);
        hide();
        w->show();
    }
}

void MainMenu::salesMenu()
{
    QDialog dlg(this);
    dlg.setWindowTitle("销售员入口");
    dlg.resize(300, 160);

    auto *btnLogin = new QPushButton("销售员登录");
    auto *btnRegister = new QPushButton("销售员注册");
    auto *btnCancel = new QPushButton("取消");

    auto *layout = new QVBoxLayout(&dlg);
    layout->addWidget(btnLogin);
    layout->addWidget(btnRegister);
    layout->addWidget(btnCancel);

    connect(btnCancel, &QPushButton::clicked, &dlg, &QDialog::reject);

    connect(btnRegister, &QPushButton::clicked, [&](){
        RegisterDialog reg(&dlg);
        reg.exec();
    });

    connect(btnLogin, &QPushButton::clicked, [&](){
        LoginDialog login(LoginDialog::Employee, &dlg);
        if (login.exec() == QDialog::Accepted) {
            auto *w = new EmployeeSalesWindow(login.employeeId(), login.employeeNo());
            w->setAttribute(Qt::WA_DeleteOnClose);
            connect(w, &QObject::destroyed, this, &QWidget::show);
            hide();
            w->show();
            dlg.accept();
        }
    });

    dlg.exec();
}
