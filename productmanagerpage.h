#ifndef PRODUCTMANAGERPAGE_H
#define PRODUCTMANAGERPAGE_H

#include <QWidget>
#include <QMap>

class QLineEdit;
class QComboBox;
class QTabWidget;
class QTableView;
class QSqlTableModel;

class ProductManagerPage : public QWidget
{
    Q_OBJECT
public:
    explicit ProductManagerPage(QWidget *parent = nullptr);

public slots:
    void refreshAll();

private slots:
    void addProduct();
    void downProduct();
    void editProduct();
    void promotionProduct();
    void showProductSalesRecords();
    void searchProduct();

private:
    QString currentCategory() const;
    QString selectedCode();
    void applyProductViewSettings(QTableView *view, QSqlTableModel *model);

    QComboBox *searchTypeBox;
    QLineEdit *searchEdit;
    QTabWidget *categoryTabs;
    QMap<QString, QTableView*> views;
    QMap<QString, QSqlTableModel*> models;
};

#endif
