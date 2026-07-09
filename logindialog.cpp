#include "logindialog.h"
#include "database.h"

#include <QtWidgets>
#include <QtSql>

LoginDialog::LoginDialog(Role role, QWidget *parent) : QDialog(parent), role(role)
{
    setWindowTitle(role == Manager ? "经理登录" : "销售员登录");
    resize(360, 180);

    auto *form = new QFormLayout;
    usernameEdit = new QLineEdit;
    passwordEdit = new QLineEdit;
    passwordEdit->setEchoMode(QLineEdit::Password);

    form->addRow("用户名：", usernameEdit);
    form->addRow("密码：", passwordEdit);

    auto *btnLogin = new QPushButton("登录");
    auto *btnCancel = new QPushButton("取消");

    auto *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(btnLogin);
    btnLayout->addWidget(btnCancel);

    auto *main = new QVBoxLayout(this);
    main->addLayout(form);
    main->addLayout(btnLayout);

    connect(btnLogin, &QPushButton::clicked, this, &LoginDialog::doLogin);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

int LoginDialog::employeeId() const
{
    return empId;
}

QString LoginDialog::employeeNo() const
{
    return empNo;
}

void LoginDialog::doLogin()
{
    QString u = usernameEdit->text().trimmed();
    QString p = passwordEdit->text();

    if (u.isEmpty() || p.isEmpty()) {
        showError(this, "请输入用户名和密码。");
        return;
    }

    QSqlQuery q;
    if (role == Manager) {
        q.prepare("SELECT id FROM managers WHERE username=? AND password=?");
        q.addBindValue(u);
        q.addBindValue(p);
        q.exec();

        if (q.next()) {
            accept();
        } else {
            showError(this, "经理账号或密码错误。");
        }
    } else {
        q.prepare("SELECT id,employee_no FROM employees WHERE username=? AND password=?");
        q.addBindValue(u);
        q.addBindValue(p);
        q.exec();

        if (q.next()) {
            empId = q.value(0).toInt();
            empNo = q.value(1).toString();
            accept();
        } else {
            showError(this, "销售员账号或密码错误。");
        }
    }
}
