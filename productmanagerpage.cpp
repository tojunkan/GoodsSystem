#include "productmanagerpage.h"
#include "database.h"
#include "productdialog.h"
#include "toggletableview.h"

#include <QtWidgets>
#include <QtSql>

ProductManagerPage::ProductManagerPage(QWidget *parent) : QWidget(parent)
{
    searchTypeBox = new QComboBox;
    searchTypeBox->addItems({"商品编号", "商品名称", "商品分类", "生产商", "全部字段"});

    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("输入查询内容");

    categoryTabs = new QTabWidget;
    categoryTabs->setTabsClosable(false);

    QStringList cats = {"全部", "生活类", "食品类", "饮料类", "日化类", "家电类", "文具类", "其他"};
    for (const QString &cat : cats) {
        auto *view = new ToggleTableView;
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->setSelectionMode(QAbstractItemView::SingleSelection);
        view->setAlternatingRowColors(true);
        view->setEditTriggers(QAbstractItemView::NoEditTriggers);
        view->horizontalHeader()->setStretchLastSection(false);
        categoryTabs->addTab(view, cat);
        views[cat] = view;
    }

    auto *btnAdd = new QPushButton("上架商品");
    auto *btnDown = new QPushButton("下架商品");
    auto *btnEdit = new QPushButton("编辑商品");
    auto *btnPromo = new QPushButton("促销商品");
    auto *btnRecord = new QPushButton("销售记录");
    auto *btnSearch = new QPushButton("查询");
    auto *btnRefresh = new QPushButton("更新");

    auto *top = new QHBoxLayout;
    top->addWidget(new QLabel("查询依据："));
    top->addWidget(searchTypeBox);
    top->addWidget(searchEdit);
    top->addWidget(btnSearch);
    top->addWidget(btnRefresh);

    auto *ops = new QHBoxLayout;
    ops->addWidget(btnAdd);
    ops->addWidget(btnDown);
    ops->addWidget(btnEdit);
    ops->addWidget(btnPromo);
    ops->addWidget(btnRecord);
    ops->addStretch();

    auto *main = new QVBoxLayout(this);
    main->addLayout(top);
    main->addLayout(ops);
    main->addWidget(categoryTabs);

    connect(btnAdd, &QPushButton::clicked, this, &ProductManagerPage::addProduct);
    connect(btnDown, &QPushButton::clicked, this, &ProductManagerPage::downProduct);
    connect(btnEdit, &QPushButton::clicked, this, &ProductManagerPage::editProduct);
    connect(btnPromo, &QPushButton::clicked, this, &ProductManagerPage::promotionProduct);
    connect(btnRecord, &QPushButton::clicked, this, &ProductManagerPage::showProductSalesRecords);
    connect(btnSearch, &QPushButton::clicked, this, &ProductManagerPage::searchProduct);
    connect(btnRefresh, &QPushButton::clicked, this, &ProductManagerPage::refreshAll);

    refreshAll();
}

void ProductManagerPage::refreshAll()
{
    QString current = currentCategory();

    for (auto m : models) delete m;
    models.clear();

    for (auto it = views.begin(); it != views.end(); ++it) {
        QString cat = it.key();
        QString filter = "status=1";

        if (cat != "全部") {
            QString safeCat = cat;
            safeCat.replace("'", "''");
            filter += QString(" AND category='%1'").arg(safeCat);
        }

        auto *model = makeModel(this, "products", filter);
        applyProductViewSettings(it.value(), model);
        models[it.key()] = model;
    }

    searchEdit->clear();

    int idx = categoryTabs->indexOf(views.value(current));
    if (idx >= 0) categoryTabs->setCurrentIndex(idx);
}

void ProductManagerPage::addProduct()
{
    ProductDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;

    QString now = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString err;

    bool ok = execSql("INSERT INTO products(code,name,category,price,producer,stock,status,promotion_rate,sales_count,purchase_date,expiry_date,created_at) "
                      "VALUES(?,?,?,?,?,?,?,?,?,?,?,?)",
                      {dlg.code(), dlg.name(), dlg.category(), dlg.price(), dlg.producer(), dlg.stock(),
                       1, 1.0, 0, dlg.purchaseDate().toString("yyyy-MM-dd"), dlg.expiryDate().toString("yyyy-MM-dd"), now},
                      &err);

    if (!ok) {
        showError(this, "上架失败，商品编号可能已存在。\n" + err);
        return;
    }

    QMessageBox::information(this, "成功", "商品上架成功。");
    refreshAll();
}

void ProductManagerPage::downProduct()
{
    QString code = selectedCode();
    if (code.isEmpty()) return;

    if (QMessageBox::question(this, "确认",
                              "确定下架选中的商品吗？\n下架后该商品整行会从商品表中删除，但历史销售记录会保留。")
        != QMessageBox::Yes) {
        return;
    }

    execSql("DELETE FROM products WHERE code=?", {code});
    QMessageBox::information(this, "成功", "商品已下架并从商品列表删除。");
    refreshAll();
}

void ProductManagerPage::editProduct()
{
    QString code = selectedCode();
    if (code.isEmpty()) return;

    QSqlQuery q;
    q.prepare("SELECT code,name,category,price,producer,stock,purchase_date,expiry_date FROM products WHERE code=?");
    q.addBindValue(code);
    q.exec();

    if (!q.next()) {
        showError(this, "未找到商品。");
        return;
    }

    ProductDialog dlg(this);
    dlg.setProduct(q.value(0).toString(),
                   q.value(1).toString(),
                   q.value(2).toString(),
                   q.value(3).toDouble(),
                   q.value(4).toString(),
                   q.value(5).toInt(),
                   QDate::fromString(q.value(6).toString(), "yyyy-MM-dd"),
                   QDate::fromString(q.value(7).toString(), "yyyy-MM-dd"));

    if (dlg.exec() != QDialog::Accepted) return;

    QString err;
    bool ok = execSql("UPDATE products SET name=?,category=?,price=?,producer=?,stock=?,purchase_date=?,expiry_date=? WHERE code=?",
                      {dlg.name(), dlg.category(), dlg.price(), dlg.producer(), dlg.stock(),
                       dlg.purchaseDate().toString("yyyy-MM-dd"), dlg.expiryDate().toString("yyyy-MM-dd"), code},
                      &err);

    if (!ok) {
        showError(this, "更新失败。\n" + err);
        return;
    }

    QMessageBox::information(this, "成功", "商品更新成功，已回到所有商品列表。");
    refreshAll();
}

void ProductManagerPage::promotionProduct()
{
    QString code = selectedCode();
    if (code.isEmpty()) return;

    bool ok = false;
    double rate = QInputDialog::getDouble(this, "设置促销折扣",
                                          "请输入折扣，例如 0.8 表示八折，1 表示取消促销：",
                                          0.8, 0.01, 1.0, 2, &ok);
    if (!ok) return;

    execSql("UPDATE products SET promotion_rate=? WHERE code=?", {rate, code});
    QMessageBox::information(this, "成功", rate < 1.0 ? "促销设置成功。" : "已取消促销。");
    refreshAll();
}

void ProductManagerPage::showProductSalesRecords()
{
    QString code = selectedCode();
    if (code.isEmpty()) return;

    QDialog dlg(this);
    dlg.setWindowTitle("商品销售记录 - " + code);
    dlg.resize(900, 520);

    auto *view = new ToggleTableView(&dlg);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setAlternatingRowColors(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->horizontalHeader()->setStretchLastSection(false);

    QString safeCode = code;
    safeCode.replace("'", "''");
    auto *model = makeModel(&dlg, "sales", QString("product_code='%1'").arg(safeCode));
    view->setModel(model);

    int idCol = model->fieldIndex("id");
    int empIdCol = model->fieldIndex("employee_id");
    int empNameCol = model->fieldIndex("employee_name");
    if (idCol >= 0) view->hideColumn(idCol);
    if (empIdCol >= 0) view->hideColumn(empIdCol);
    if (empNameCol >= 0) view->hideColumn(empNameCol);

    model->setHeaderData(model->fieldIndex("employee_no"), Qt::Horizontal, "销售员编号");
    model->setHeaderData(model->fieldIndex("product_code"), Qt::Horizontal, "商品编号");
    model->setHeaderData(model->fieldIndex("product_name"), Qt::Horizontal, "商品名称");
    model->setHeaderData(model->fieldIndex("category"), Qt::Horizontal, "商品分类");
    model->setHeaderData(model->fieldIndex("unit_price"), Qt::Horizontal, "成交单价");
    model->setHeaderData(model->fieldIndex("quantity"), Qt::Horizontal, "数量");
    model->setHeaderData(model->fieldIndex("amount"), Qt::Horizontal, "金额");
    model->setHeaderData(model->fieldIndex("commission"), Qt::Horizontal, "提成");
    model->setHeaderData(model->fieldIndex("sold_at"), Qt::Horizontal, "销售时间");
    setEqualColumnWidths(view, 120);

    QSqlQuery q;
    q.prepare("SELECT COALESCE(SUM(quantity),0), COALESCE(SUM(amount),0) FROM sales WHERE product_code=?");
    q.addBindValue(code);
    q.exec();

    int totalQty = 0;
    double totalAmount = 0;
    if (q.next()) {
        totalQty = q.value(0).toInt();
        totalAmount = q.value(1).toDouble();
    }

    auto *label = new QLabel(QString("商品编号：%1；累计销量：%2；累计销售额：￥%3")
                             .arg(code).arg(totalQty).arg(money(totalAmount)), &dlg);
    auto *closeBtn = new QPushButton("关闭", &dlg);

    auto *layout = new QVBoxLayout(&dlg);
    layout->addWidget(label);
    layout->addWidget(view);
    layout->addWidget(closeBtn, 0, Qt::AlignRight);

    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    dlg.exec();
}

void ProductManagerPage::searchProduct()
{
    QString key = searchEdit->text().trimmed();
    if (key.isEmpty()) {
        refreshAll();
        return;
    }

    QString safeKey = sqlLikeEscape(key);
    QString type = searchTypeBox->currentText();
    QString condition;

    if (type == "商品编号") {
        condition = QString("code LIKE '%%1%'").arg(safeKey);
    } else if (type == "商品名称") {
        condition = QString("name LIKE '%%1%'").arg(safeKey);
    } else if (type == "商品分类") {
        condition = QString("category LIKE '%%1%'").arg(safeKey);
    } else if (type == "生产商") {
        condition = QString("producer LIKE '%%1%'").arg(safeKey);
    } else {
        condition = QString("(code LIKE '%%1%' OR name LIKE '%%1%' OR category LIKE '%%1%' OR producer LIKE '%%1%')").arg(safeKey);
    }

    QString baseFilter = "status=1 AND " + condition;

    for (auto m : models) delete m;
    models.clear();

    for (auto it = views.begin(); it != views.end(); ++it) {
        QString cat = it.key();
        QString filter = baseFilter;

        if (cat != "全部") {
            QString safeCat = cat;
            safeCat.replace("'", "''");
            filter += QString(" AND category='%1'").arg(safeCat);
        }

        auto *model = makeModel(this, "products", filter);
        applyProductViewSettings(it.value(), model);
        models[it.key()] = model;
    }

    if (models.contains("全部") && models["全部"]->rowCount() == 1) {
        categoryTabs->setCurrentWidget(views["全部"]);
        views["全部"]->selectRow(0);
    }
}

QString ProductManagerPage::currentCategory() const
{
    QWidget *w = categoryTabs->currentWidget();
    for (auto it = views.begin(); it != views.end(); ++it) {
        if (it.value() == w) return it.key();
    }
    return "全部";
}

QString ProductManagerPage::selectedCode()
{
    auto *view = qobject_cast<QTableView*>(categoryTabs->currentWidget());
    if (!view || !view->selectionModel() || !view->model()) {
        showError(this, "请先选择一行商品。");
        return QString();
    }

    QModelIndexList rows = view->selectionModel()->selectedRows();
    if (rows.isEmpty()) {
        showError(this, "请先选择一行商品。");
        return QString();
    }

    auto *model = qobject_cast<QSqlTableModel*>(view->model());
    if (!model) return QString();

    int codeCol = model->fieldIndex("code");
    if (codeCol < 0) return QString();

    int row = rows.first().row();
    return model->data(model->index(row, codeCol)).toString();
}

void ProductManagerPage::applyProductViewSettings(QTableView *view, QSqlTableModel *model)
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
    model->setHeaderData(model->fieldIndex("category"), Qt::Horizontal, "商品分类");
    model->setHeaderData(model->fieldIndex("price"), Qt::Horizontal, "商品单价");
    model->setHeaderData(model->fieldIndex("producer"), Qt::Horizontal, "生产商");
    model->setHeaderData(model->fieldIndex("stock"), Qt::Horizontal, "库存量");
    model->setHeaderData(model->fieldIndex("promotion_rate"), Qt::Horizontal, "促销折扣");
    model->setHeaderData(model->fieldIndex("sales_count"), Qt::Horizontal, "销售量");
    model->setHeaderData(model->fieldIndex("purchase_date"), Qt::Horizontal, "进货日期");
    model->setHeaderData(model->fieldIndex("expiry_date"), Qt::Horizontal, "到期日期");

    setEqualColumnWidths(view, 120);
}
