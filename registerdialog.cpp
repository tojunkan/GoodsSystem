#include "registerdialog.h"
#include "database.h"

#include <QtWidgets>
#include <QtSql>

RegisterDialog::RegisterDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("销售员注册");
    resize(400, 230);

    usernameEdit = new QLineEdit;
    passwordEdit = new QLineEdit;
    passwordEdit->setEchoMode(QLineEdit::Password);
    employeeNoEdit = new QLineEdit;
    employeeNoEdit->setPlaceholderText("例如：S001");
    phoneEdit = new QLineEdit;

    auto *form = new QFormLayout;
    form->addRow("用户名：", usernameEdit);
    form->addRow("密码：", passwordEdit);
    form->addRow("销售员编号：", employeeNoEdit);
    form->addRow("电话：", phoneEdit);

    auto *btnRegister = new QPushButton("注册");
    auto *btnCancel = new QPushButton("取消");

    auto *btns = new QHBoxLayout;
    btns->addStretch();
    btns->addWidget(btnRegister);
    btns->addWidget(btnCancel);

    auto *main = new QVBoxLayout(this);
    main->addLayout(form);
    main->addLayout(btns);

    connect(btnRegister, &QPushButton::clicked, this, &RegisterDialog::doRegister);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void RegisterDialog::doRegister()
{
    QString u = usernameEdit->text().trimmed();
    QString p = passwordEdit->text();
    QString no = employeeNoEdit->text().trimmed();
    QString phone = phoneEdit->text().trimmed();

    if (u.isEmpty() || p.isEmpty() || no.isEmpty()) {
        showError(this, "用户名、密码、销售员编号不能为空。");
        return;
    }

    QSqlQuery check;
    check.prepare("SELECT id FROM employees WHERE username=? OR employee_no=?");
    check.addBindValue(u);
    check.addBindValue(no);
    check.exec();

    if (check.next()) {
        showError(this, "注册失败：用户名或销售员编号已经存在。");
        return;
    }

    QString now = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString err;
    bool ok = false;

    // 兼容旧版数据库：旧表里可能还有 NOT NULL 的 name 字段。
    if (columnExists("employees", "name")) {
        ok = execSql("INSERT INTO employees(username,password,name,employee_no,phone,created_at) VALUES(?,?,?,?,?,?)",
                     {u, p, no, no, phone, now}, &err);
    } else {
        ok = execSql("INSERT INTO employees(username,password,employee_no,phone,created_at) VALUES(?,?,?,?,?)",
                     {u, p, no, phone, now}, &err);
    }

    if (!ok) {
        showError(this, "注册失败。\n" + err);
        return;
    }

    QMessageBox::information(this, "成功", "注册成功，现在可以登录。");
    accept();
}
