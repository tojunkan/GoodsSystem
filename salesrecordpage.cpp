#include "salesrecordpage.h"
#include "database.h"
#include "toggletableview.h"

#include <QtWidgets>
#include <QtSql>

SalesRecordPage::SalesRecordPage(QWidget *parent) : QWidget(parent)
{
    employeeBox = new QComboBox;
    employeeBox->addItem("全部销售员", -1);

    yearSpin = new QSpinBox;
    yearSpin->setRange(2000, 2100);
    yearSpin->setValue(QDate::currentDate().year());

    monthSpin = new QSpinBox;
    monthSpin->setRange(1, 12);
    monthSpin->setValue(QDate::currentDate().month());

    auto *btnQuery = new QPushButton("查询");
    totalLabel = new QLabel("总销售额：￥0.00，合计提成：￥0.00");

    view = new ToggleTableView;
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setAlternatingRowColors(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->horizontalHeader()->setStretchLastSection(false);

    auto *top = new QHBoxLayout;
    top->addWidget(new QLabel("销售员编号："));
    top->addWidget(employeeBox);
    top->addWidget(new QLabel("年份："));
    top->addWidget(yearSpin);
    top->addWidget(new QLabel("月份："));
    top->addWidget(monthSpin);
    top->addWidget(btnQuery);
    top->addStretch();

    auto *main = new QVBoxLayout(this);
    main->addLayout(top);
    main->addWidget(totalLabel);
    main->addWidget(view);

    connect(btnQuery, &QPushButton::clicked, this, &SalesRecordPage::queryRecords);

    loadEmployees();
    queryRecords();
}

void SalesRecordPage::loadEmployees()
{
    int current = employeeBox->currentData().toInt();

    employeeBox->clear();
    employeeBox->addItem("全部销售员", -1);

    QSqlQuery q("SELECT id,employee_no FROM employees ORDER BY id");
    while (q.next()) {
        employeeBox->addItem(q.value(1).toString(), q.value(0).toInt());
    }

    int idx = employeeBox->findData(current);
    if (idx >= 0) employeeBox->setCurrentIndex(idx);
}

void SalesRecordPage::queryRecords()
{
    int year = yearSpin->value();
    int month = monthSpin->value();
    QDate d(year, month, 1);
    QString begin = d.toString("yyyy-MM-dd 00:00:00");
    QString end = QDate(year, month, d.daysInMonth()).toString("yyyy-MM-dd 23:59:59");
    int empId = employeeBox->currentData().toInt();

    QString filter = QString("sold_at >= '%1' AND sold_at <= '%2'").arg(begin, end);
    if (empId != -1) filter += QString(" AND employee_id=%1").arg(empId);

    if (model) delete model;
    model = makeModel(this, "sales", filter);
    setupHeaders(model);

    view->setModel(model);

    int idCol = model->fieldIndex("id");
    int empIdCol = model->fieldIndex("employee_id");
    int empNameCol = model->fieldIndex("employee_name");
    if (idCol >= 0) view->hideColumn(idCol);
    if (empIdCol >= 0) view->hideColumn(empIdCol);
    if (empNameCol >= 0) view->hideColumn(empNameCol);

    setEqualColumnWidths(view, 120);

    QSqlQuery q;
    QString sql = "SELECT COALESCE(SUM(amount),0), COALESCE(SUM(commission),0) FROM sales "
                  "WHERE sold_at >= ? AND sold_at <= ?";
    if (empId != -1) sql += " AND employee_id=?";

    q.prepare(sql);
    q.addBindValue(begin);
    q.addBindValue(end);
    if (empId != -1) q.addBindValue(empId);
    q.exec();

    double total = 0, commission = 0;
    if (q.next()) {
        total = q.value(0).toDouble();
        commission = q.value(1).toDouble();
    }

    totalLabel->setText(QString("总销售额：￥%1，合计提成：￥%2")
                        .arg(money(total), money(commission)));
}

void SalesRecordPage::setupHeaders(QSqlTableModel *model)
{
    model->setHeaderData(model->fieldIndex("employee_no"), Qt::Horizontal, "销售员编号");
    model->setHeaderData(model->fieldIndex("product_code"), Qt::Horizontal, "商品编号");
    model->setHeaderData(model->fieldIndex("product_name"), Qt::Horizontal, "商品名称");
    model->setHeaderData(model->fieldIndex("category"), Qt::Horizontal, "商品分类");
    model->setHeaderData(model->fieldIndex("unit_price"), Qt::Horizontal, "成交单价");
    model->setHeaderData(model->fieldIndex("quantity"), Qt::Horizontal, "数量");
    model->setHeaderData(model->fieldIndex("amount"), Qt::Horizontal, "金额");
    model->setHeaderData(model->fieldIndex("commission"), Qt::Horizontal, "提成");
    model->setHeaderData(model->fieldIndex("sold_at"), Qt::Horizontal, "销售时间");
}
