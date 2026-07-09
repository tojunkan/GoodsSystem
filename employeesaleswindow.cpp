#include "employeesaleswindow.h"
#include "database.h"
#include "toggletableview.h"

#include <QtWidgets>
#include <QtSql>

EmployeeSalesWindow::EmployeeSalesWindow(int employeeId, const QString &employeeNo, QWidget *parent)
    : QMainWindow(parent), empId(employeeId), empNo(employeeNo)
{
    setWindowTitle("销售员工作台 - 编号：" + empNo);
    resize(1000, 650);

    auto *toolbar = addToolBar("操作");
    toolbar->setMovable(false);
    auto *logoutAction = toolbar->addAction("返回主菜单");
    connect(logoutAction, &QAction::triggered, this, &QWidget::close);

    auto *central = new QWidget;
    setCentralWidget(central);

    codeEdit = new QLineEdit;
    codeEdit->setPlaceholderText("输入商品编号，例如 F001");

    quantitySpin = new QSpinBox;
    quantitySpin->setRange(1, 999999);

    productInfo = new QLabel("商品信息：未查询");
    amountInfo = new QLabel("应付金额：￥0.00；本单提成：￥0.00");

    auto *btnFind = new QPushButton("查询商品");
    auto *btnReset = new QPushButton("更新状态");
    auto *btnSale = new QPushButton("确认销售");

    auto *form = new QGridLayout;
    form->addWidget(new QLabel("商品编号："), 0, 0);
    form->addWidget(codeEdit, 0, 1);
    form->addWidget(new QLabel("购买数量："), 0, 2);
    form->addWidget(quantitySpin, 0, 3);
    form->addWidget(btnFind, 0, 4);
    form->addWidget(btnReset, 0, 5);
    form->addWidget(btnSale, 0, 6);
    form->addWidget(productInfo, 1, 0, 1, 7);
    form->addWidget(amountInfo, 2, 0, 1, 7);

    productsView = new ToggleTableView;
    productsView->setSelectionBehavior(QAbstractItemView::SelectRows);
    productsView->setAlternatingRowColors(true);
    productsView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    productsView->horizontalHeader()->setStretchLastSection(false);

    mySalesView = new ToggleTableView;
    mySalesView->setSelectionBehavior(QAbstractItemView::SelectRows);
    mySalesView->setAlternatingRowColors(true);
    mySalesView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mySalesView->horizontalHeader()->setStretchLastSection(false);

    auto *tabs = new QTabWidget;
    tabs->addTab(productsView, "可售商品");
    tabs->addTab(mySalesView, "我的销售记录");

    auto *main = new QVBoxLayout(central);
    main->addLayout(form);
    main->addWidget(tabs);

    connect(btnFind, &QPushButton::clicked, this, &EmployeeSalesWindow::findProduct);
    connect(btnReset, &QPushButton::clicked, this, &EmployeeSalesWindow::resetLookup);
    connect(btnSale, &QPushButton::clicked, this, &EmployeeSalesWindow::sellProduct);
    connect(quantitySpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &EmployeeSalesWindow::updateAmountPreview);

    refreshProducts();
    refreshMySales();
}

void EmployeeSalesWindow::refreshProducts()
{
    if (productsModel) delete productsModel;

    productsModel = makeModel(this, "products", "status=1 AND stock>0");
    applyProductViewSettings(productsView, productsModel);
}

void EmployeeSalesWindow::refreshMySales()
{
    if (salesModel) delete salesModel;

    QString filter = QString("employee_id=%1").arg(empId);
    salesModel = makeModel(this, "sales", filter);
    applySalesViewSettings(mySalesView, salesModel);
}

void EmployeeSalesWindow::findProduct()
{
    currentProductValid = false;

    QString code = codeEdit->text().trimmed();
    if (code.isEmpty()) {
        showError(this, "请输入商品编号。");
        return;
    }

    QSqlQuery q;
    q.prepare("SELECT code,name,category,price,producer,stock,promotion_rate,sales_count,purchase_date,expiry_date "
              "FROM products WHERE code=? AND status=1");
    q.addBindValue(code);
    q.exec();

    if (!q.next()) {
        productInfo->setText("商品信息：未找到商品");
        amountInfo->setText("应付金额：￥0.00；本单提成：￥0.00");
        return;
    }

    currentCode = q.value(0).toString();
    currentName = q.value(1).toString();
    currentCategory = q.value(2).toString();
    currentPrice = q.value(3).toDouble();
    currentProducer = q.value(4).toString();
    currentStock = q.value(5).toInt();
    currentRate = q.value(6).toDouble();
    currentSalesCount = q.value(7).toInt();
    currentPurchaseDate = q.value(8).toString();
    currentExpiryDate = q.value(9).toString();
    currentProductValid = true;

    double dealPrice = currentPrice * currentRate;
    productInfo->setText(QString("商品信息：编号 %1；名称 %2；分类 %3；原价 ￥%4；折扣 %5；成交单价 ￥%6；库存 %7；销售量 %8；进货日期 %9；到期日期 %10")
                         .arg(currentCode,
                              currentName,
                              currentCategory,
                              money(currentPrice),
                              QString::number(currentRate, 'f', 2),
                              money(dealPrice),
                              QString::number(currentStock),
                              QString::number(currentSalesCount),
                              currentPurchaseDate,
                              currentExpiryDate));

    updateAmountPreview();
}

void EmployeeSalesWindow::resetLookup()
{
    currentProductValid = false;
    currentCode.clear();
    currentName.clear();
    currentCategory.clear();
    currentProducer.clear();
    currentPurchaseDate.clear();
    currentExpiryDate.clear();
    currentPrice = 0;
    currentStock = 0;
    currentRate = 1.0;
    currentSalesCount = 0;

    codeEdit->clear();
    quantitySpin->setValue(1);
    productInfo->setText("商品信息：未查询");
    amountInfo->setText("应付金额：￥0.00；本单提成：￥0.00");
    refreshProducts();
}

void EmployeeSalesWindow::updateAmountPreview()
{
    if (!currentProductValid) return;

    int qty = quantitySpin->value();
    double dealPrice = currentPrice * currentRate;
    double amount = dealPrice * qty;
    double commission = amount * 0.05;

    amountInfo->setText(QString("应付金额：￥%1；本单提成：￥%2")
                        .arg(money(amount), money(commission)));
}

void EmployeeSalesWindow::sellProduct()
{
    if (!currentProductValid || codeEdit->text().trimmed() != currentCode) {
        findProduct();
        if (!currentProductValid) return;
    }

    int qty = quantitySpin->value();

    QSqlQuery q;
    q.prepare("SELECT stock,price,promotion_rate,name,category FROM products WHERE code=? AND status=1");
    q.addBindValue(currentCode);
    q.exec();

    if (!q.next()) {
        showError(this, "商品不存在或已下架。");
        resetLookup();
        return;
    }

    int stock = q.value(0).toInt();
    double price = q.value(1).toDouble();
    double rate = q.value(2).toDouble();
    QString productName = q.value(3).toString();
    QString category = q.value(4).toString();

    if (stock < qty) {
        showError(this, QString("库存不足。当前库存：%1").arg(stock));
        findProduct();
        return;
    }

    double dealPrice = price * rate;
    double amount = dealPrice * qty;
    double commission = amount * 0.05;
    QString now = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    QSqlDatabase::database().transaction();

    QString err;
    bool ok1 = execSql("UPDATE products SET stock=stock-?, sales_count=sales_count+? WHERE code=? AND stock>=?",
                       {qty, qty, currentCode, qty}, &err);

    bool ok2 = false;

    // 兼容旧版数据库：旧表里可能还有 NOT NULL 的 employee_name 字段。
    if (columnExists("sales", "employee_name")) {
        ok2 = execSql("INSERT INTO sales(employee_id,employee_name,employee_no,product_code,product_name,category,unit_price,quantity,amount,commission,sold_at) "
                      "VALUES(?,?,?,?,?,?,?,?,?,?,?)",
                      {empId, empNo, empNo, currentCode, productName, category, dealPrice, qty, amount, commission, now},
                      &err);
    } else {
        ok2 = execSql("INSERT INTO sales(employee_id,employee_no,product_code,product_name,category,unit_price,quantity,amount,commission,sold_at) "
                      "VALUES(?,?,?,?,?,?,?,?,?,?)",
                      {empId, empNo, currentCode, productName, category, dealPrice, qty, amount, commission, now},
                      &err);
    }

    if (ok1 && ok2) {
        QSqlDatabase::database().commit();

        QMessageBox::information(this, "销售成功",
                                 QString("商品：%1\n数量：%2\n顾客应付：￥%3\n本单提成：￥%4")
                                 .arg(productName)
                                 .arg(qty)
                                 .arg(money(amount), money(commission)));

        refreshProducts();
        refreshMySales();
        resetLookup();
    } else {
        QSqlDatabase::database().rollback();
        showError(this, "销售失败。\n" + err);
    }
}

void EmployeeSalesWindow::applyProductViewSettings(QTableView *view, QSqlTableModel *model)
{
    int codeColForSort = model->fieldIndex("code");
    if (codeColForSort >= 0) {
        model->setSort(codeColForSort, Qt::AscendingOrder);
        model->select();
    }

    view->setModel(model);

    const QStringList hidden = {"id", "status", "created_at"};
    for (const QString &field : hidden) {
        int col = model->fieldIndex(field);
        if (col >= 0) view->hideColumn(col);
    }

    model->setHeaderData(model->fieldIndex("code"), Qt::Horizontal, "商品编号");
    model->setHeaderData(model->fieldIndex("name"), Qt::Horizontal, "商品名称");
    model->setHeaderData(model->fieldIndex("category"), Qt::Horizontal, "分类");
    model->setHeaderData(model->fieldIndex("price"), Qt::Horizontal, "单价");
    model->setHeaderData(model->fieldIndex("producer"), Qt::Horizontal, "生产商");
    model->setHeaderData(model->fieldIndex("stock"), Qt::Horizontal, "库存量");
    model->setHeaderData(model->fieldIndex("promotion_rate"), Qt::Horizontal, "促销折扣");
    model->setHeaderData(model->fieldIndex("sales_count"), Qt::Horizontal, "销售量");
    model->setHeaderData(model->fieldIndex("purchase_date"), Qt::Horizontal, "进货日期");
    model->setHeaderData(model->fieldIndex("expiry_date"), Qt::Horizontal, "到期日期");

    setEqualColumnWidths(view, 120);
}

void EmployeeSalesWindow::applySalesViewSettings(QTableView *view, QSqlTableModel *model)
{
    view->setModel(model);

    const QStringList hidden = {"id", "employee_id", "employee_name"};
    for (const QString &field : hidden) {
        int col = model->fieldIndex(field);
        if (col >= 0) view->hideColumn(col);
    }

    model->setHeaderData(model->fieldIndex("employee_no"), Qt::Horizontal, "销售员编号");
    model->setHeaderData(model->fieldIndex("product_code"), Qt::Horizontal, "商品编号");
    model->setHeaderData(model->fieldIndex("product_name"), Qt::Horizontal, "商品名称");
    model->setHeaderData(model->fieldIndex("category"), Qt::Horizontal, "分类");
    model->setHeaderData(model->fieldIndex("unit_price"), Qt::Horizontal, "成交单价");
    model->setHeaderData(model->fieldIndex("quantity"), Qt::Horizontal, "数量");
    model->setHeaderData(model->fieldIndex("amount"), Qt::Horizontal, "金额");
    model->setHeaderData(model->fieldIndex("commission"), Qt::Horizontal, "提成");
    model->setHeaderData(model->fieldIndex("sold_at"), Qt::Horizontal, "销售时间");

    setEqualColumnWidths(view, 120);
}
