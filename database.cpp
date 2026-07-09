#include "database.h"

#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>

bool execSql(const QString &sql, const QVariantList &args, QString *err)
{
    QSqlQuery q;
    q.prepare(sql);
    for (int i = 0; i < args.size(); ++i) q.bindValue(i, args[i]);

    if (!q.exec()) {
        if (err) *err = q.lastError().text();
        qDebug() << "SQL error:" << q.lastError().text() << sql << args;
        return false;
    }
    return true;
}

QString money(double v)
{
    return QString::number(v, 'f', 2);
}

void showError(QWidget *parent, const QString &msg)
{
    QMessageBox::warning(parent, "提示", msg);
}

QSqlTableModel* makeModel(QObject *parent, const QString &table, const QString &filter)
{
    auto *model = new QSqlTableModel(parent);
    model->setTable(table);
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    if (!filter.isEmpty()) model->setFilter(filter);
    model->select();
    return model;
}

QString sqlLikeEscape(QString s)
{
    s.replace("'", "''");
    return s;
}


void setEqualColumnWidths(QTableView *view, int width)
{
    if (!view || !view->model()) return;

    view->horizontalHeader()->setStretchLastSection(false);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    for (int c = 0; c < view->model()->columnCount(); ++c) {
        if (!view->isColumnHidden(c)) {
            view->setColumnWidth(c, width);
        }
    }
}

bool columnExists(const QString &table, const QString &column)
{
    QSqlQuery q;
    q.exec(QString("PRAGMA table_info(%1)").arg(table));
    while (q.next()) {
        if (q.value(1).toString().compare(column, Qt::CaseInsensitive) == 0) return true;
    }
    return false;
}

static void ensureColumn(const QString &table, const QString &column, const QString &definition)
{
    if (!columnExists(table, column)) {
        execSql(QString("ALTER TABLE %1 ADD COLUMN %2 %3").arg(table, column, definition));
    }
}

bool initDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dataDir.isEmpty()) {
        dataDir = QCoreApplication::applicationDirPath();
    }

    QDir dir(dataDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString dbPath = dir.filePath("supermarket.db");
    db.setDatabaseName(dbPath);

    qDebug() << "数据库保存位置：" << dbPath;

    if (!db.open()) {
        QMessageBox::critical(nullptr, "数据库错误", "无法打开 SQLite 数据库：" + db.lastError().text());
        return false;
    }

    execSql("CREATE TABLE IF NOT EXISTS managers ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "username TEXT UNIQUE NOT NULL,"
            "password TEXT NOT NULL)");

    execSql("CREATE TABLE IF NOT EXISTS employees ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "username TEXT UNIQUE NOT NULL,"
            "password TEXT NOT NULL,"
            "employee_no TEXT,"
            "phone TEXT,"
            "created_at TEXT NOT NULL)");

    execSql("CREATE TABLE IF NOT EXISTS products ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "code TEXT UNIQUE NOT NULL,"
            "name TEXT NOT NULL,"
            "category TEXT NOT NULL,"
            "price REAL NOT NULL,"
            "producer TEXT,"
            "stock INTEGER NOT NULL,"
            "status INTEGER NOT NULL DEFAULT 1,"
            "promotion_rate REAL NOT NULL DEFAULT 1.0,"
            "sales_count INTEGER NOT NULL DEFAULT 0,"
            "purchase_date TEXT,"
            "expiry_date TEXT,"
            "created_at TEXT NOT NULL)");

    execSql("CREATE TABLE IF NOT EXISTS sales ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "employee_id INTEGER NOT NULL,"
            "employee_no TEXT NOT NULL,"
            "product_code TEXT NOT NULL,"
            "product_name TEXT NOT NULL,"
            "category TEXT NOT NULL,"
            "unit_price REAL NOT NULL,"
            "quantity INTEGER NOT NULL,"
            "amount REAL NOT NULL,"
            "commission REAL NOT NULL,"
            "sold_at TEXT NOT NULL)");

    execSql("DELETE FROM sales");
    execSql("UPDATE products SET sales_count=0");


    ensureColumn("employees", "employee_no", "TEXT");
    ensureColumn("products", "sales_count", "INTEGER NOT NULL DEFAULT 0");
    ensureColumn("products", "purchase_date", "TEXT");
    ensureColumn("products", "expiry_date", "TEXT");
    ensureColumn("sales", "employee_no", "TEXT");

    if (columnExists("employees", "name")) {
        execSql("UPDATE employees SET employee_no=name WHERE employee_no IS NULL OR employee_no=''");
    }
    execSql("UPDATE employees SET employee_no='E' || id WHERE employee_no IS NULL OR employee_no=''");

    if (columnExists("sales", "employee_name")) {
        execSql("UPDATE sales SET employee_no=employee_name WHERE employee_no IS NULL OR employee_no=''");
    }
    execSql("UPDATE sales SET employee_no='E' || employee_id WHERE employee_no IS NULL OR employee_no=''");

    execSql("DELETE FROM products WHERE status=0");
    execSql("INSERT OR IGNORE INTO managers(username, password) VALUES('admin', '123456')");

    QSqlQuery check("SELECT COUNT(*) FROM products");
    int count = 0;
    if (check.next()) count = check.value(0).toInt();

    if (count == 0) {
        QString now = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        QString purchase = QDate::currentDate().toString("yyyy-MM-dd");
        QString longExpiry = QDate::currentDate().addYears(2).toString("yyyy-MM-dd");
        QString shortExpiry = QDate::currentDate().addMonths(6).toString("yyyy-MM-dd");

        QList<QVariantList> demo = {
            QVariantList{QString("L001"), QString("洗衣液"), QString("生活类"), 39.90, QString("蓝月亮"), 120, 1, 1.0, 0, purchase, longExpiry, now},
            QVariantList{QString("L002"), QString("抽纸"), QString("生活类"), 18.50, QString("清风"), 200, 1, 1.0, 0, purchase, longExpiry, now},
            QVariantList{QString("F001"), QString("牛奶"), QString("食品类"), 56.00, QString("伊利"), 90, 1, 1.0, 0, purchase, shortExpiry, now},
            QVariantList{QString("F002"), QString("面包"), QString("食品类"), 12.80, QString("桃李"), 60, 1, 1.0, 0, purchase, QDate::currentDate().addDays(7).toString("yyyy-MM-dd"), now},
            QVariantList{QString("D001"), QString("可乐"), QString("饮料类"), 3.50, QString("可口可乐"), 300, 1, 1.0, 0, purchase, shortExpiry, now},
            QVariantList{QString("S001"), QString("洗发水"), QString("日化类"), 45.00, QString("海飞丝"), 80, 1, 1.0, 0, purchase, longExpiry, now}
        };

        for (auto &row : demo) {
            execSql("INSERT INTO products(code,name,category,price,producer,stock,status,promotion_rate,sales_count,purchase_date,expiry_date,created_at) "
                    "VALUES(?,?,?,?,?,?,?,?,?,?,?,?)", row);
        }
    }

    return true;
}
