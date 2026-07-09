#include "employeemanagerpage.h"
#include "database.h"
#include "registerdialog.h"
#include "toggletableview.h"

#include <QtWidgets>
#include <QtSql>

EmployeeManagerPage::EmployeeManagerPage(QWidget *parent) : QWidget(parent)
{
    view = new ToggleTableView;
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setAlternatingRowColors(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->horizontalHeader()->setStretchLastSection(false);

    auto *btnAdd = new QPushButton("添加员工信息");
    auto *btnDelete = new QPushButton("删除员工信息");
    auto *btnEdit = new QPushButton("修改员工信息");

    auto *ops = new QHBoxLayout;
    ops->addWidget(btnAdd);
    ops->addWidget(btnDelete);
    ops->addWidget(btnEdit);
    ops->addStretch();

    auto *main = new QVBoxLayout(this);
    main->addLayout(ops);
    main->addWidget(view);

    connect(btnAdd, &QPushButton::clicked, this, &EmployeeManagerPage::addEmployee);
    connect(btnDelete, &QPushButton::clicked, this, &EmployeeManagerPage::deleteEmployee);
    connect(btnEdit, &QPushButton::clicked, this, &EmployeeManagerPage::editEmployee);

    refresh();
}

void EmployeeManagerPage::refresh()
{
    if (model) delete model;

    model = makeModel(this, "employees");
    view->setModel(model);
    applyEmployeeViewSettings();
}

void EmployeeManagerPage::addEmployee()
{
    RegisterDialog dlg(this);
    dlg.exec();
    refresh();
}

void EmployeeManagerPage::deleteEmployee()
{
    if (!view->selectionModel() || !model) return;

    auto rows = view->selectionModel()->selectedRows();
    if (rows.isEmpty()) {
        showError(this, "请先选择一个员工。");
        return;
    }

    int idCol = model->fieldIndex("id");
    int noCol = model->fieldIndex("employee_no");
    int row = rows.first().row();

    int id = model->data(model->index(row, idCol)).toInt();
    QString no = model->data(model->index(row, noCol)).toString();

    if (QMessageBox::question(this, "确认删除",
                              "确定删除销售员编号：" + no + " 吗？\n历史销售记录会保留。")
        != QMessageBox::Yes) {
        return;
    }

    execSql("DELETE FROM employees WHERE id=?", {id});
    refresh();
}

void EmployeeManagerPage::editEmployee()
{
    if (!view->selectionModel() || !model) return;

    auto rows = view->selectionModel()->selectedRows();
    if (rows.isEmpty()) {
        showError(this, "请先选择一个员工。");
        return;
    }

    int row = rows.first().row();
    int idCol = model->fieldIndex("id");
    int userCol = model->fieldIndex("username");
    int passCol = model->fieldIndex("password");
    int noCol = model->fieldIndex("employee_no");
    int phoneCol = model->fieldIndex("phone");

    int id = model->data(model->index(row, idCol)).toInt();
    QString oldUsername = model->data(model->index(row, userCol)).toString();
    QString oldPassword = model->data(model->index(row, passCol)).toString();
    QString oldNo = model->data(model->index(row, noCol)).toString();
    QString oldPhone = phoneCol >= 0 ? model->data(model->index(row, phoneCol)).toString() : QString();

    QDialog dlg(this);
    dlg.setWindowTitle("修改员工信息");
    dlg.resize(420, 260);

    auto *usernameEdit = new QLineEdit(oldUsername, &dlg);
    auto *passwordEdit = new QLineEdit(oldPassword, &dlg);
    auto *employeeNoEdit = new QLineEdit(oldNo, &dlg);
    auto *phoneEdit = new QLineEdit(oldPhone, &dlg);

    auto *form = new QFormLayout;
    form->addRow("用户名：", usernameEdit);
    form->addRow("密码：", passwordEdit);
    form->addRow("销售员编号：", employeeNoEdit);
    form->addRow("电话：", phoneEdit);

    auto *btnOk = new QPushButton("保存修改", &dlg);
    auto *btnCancel = new QPushButton("取消", &dlg);

    auto *btns = new QHBoxLayout;
    btns->addStretch();
    btns->addWidget(btnOk);
    btns->addWidget(btnCancel);

    auto *layout = new QVBoxLayout(&dlg);
    layout->addLayout(form);
    layout->addLayout(btns);

    connect(btnCancel, &QPushButton::clicked, &dlg, &QDialog::reject);
    connect(btnOk, &QPushButton::clicked, &dlg, [&](){
        QString username = usernameEdit->text().trimmed();
        QString password = passwordEdit->text();
        QString employeeNo = employeeNoEdit->text().trimmed();
        QString phone = phoneEdit->text().trimmed();

        if (username.isEmpty() || password.isEmpty() || employeeNo.isEmpty()) {
            showError(&dlg, "用户名、密码、销售员编号不能为空。");
            return;
        }

        QSqlQuery check;
        check.prepare("SELECT id FROM employees WHERE (username=? OR employee_no=?) AND id<>?");
        check.addBindValue(username);
        check.addBindValue(employeeNo);
        check.addBindValue(id);
        check.exec();

        if (check.next()) {
            showError(&dlg, "修改失败：用户名或销售员编号已经存在。");
            return;
        }

        QString err;
        bool ok = false;

        // 兼容旧版数据库：旧 employees 表可能还保留 name 字段。
        if (columnExists("employees", "name")) {
            ok = execSql("UPDATE employees SET username=?,password=?,name=?,employee_no=?,phone=? WHERE id=?",
                         {username, password, employeeNo, employeeNo, phone, id}, &err);
        } else {
            ok = execSql("UPDATE employees SET username=?,password=?,employee_no=?,phone=? WHERE id=?",
                         {username, password, employeeNo, phone, id}, &err);
        }

        if (!ok) {
            showError(&dlg, "修改失败。\n" + err);
            return;
        }

        QMessageBox::information(&dlg, "成功", "员工信息已修改。");
        dlg.accept();
    });

    if (dlg.exec() == QDialog::Accepted) {
        refresh();
    }
}

void EmployeeManagerPage::applyEmployeeViewSettings()
{
    if (!model) return;

    const QStringList hidden = {"id", "password", "name"};
    for (const QString &field : hidden) {
        int col = model->fieldIndex(field);
        if (col >= 0) view->hideColumn(col);
    }

    model->setHeaderData(model->fieldIndex("username"), Qt::Horizontal, "用户名");
    model->setHeaderData(model->fieldIndex("employee_no"), Qt::Horizontal, "销售员编号");
    model->setHeaderData(model->fieldIndex("phone"), Qt::Horizontal, "电话");
    model->setHeaderData(model->fieldIndex("created_at"), Qt::Horizontal, "注册时间");

    setEqualColumnWidths(view, 220);
}
