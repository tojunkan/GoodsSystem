#ifndef EMPLOYEESALESWINDOW_H
#define EMPLOYEESALESWINDOW_H

#include <QMainWindow>
#include <QString>

class QLineEdit;
class QSpinBox;
class QLabel;
class QTableView;
class QSqlTableModel;

class EmployeeSalesWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit EmployeeSalesWindow(int employeeId, const QString &employeeNo, QWidget *parent = nullptr);

private slots:
    void refreshProducts();
    void refreshMySales();
    void findProduct();
    void resetLookup();
    void updateAmountPreview();
    void sellProduct();

private:
    void applyProductViewSettings(QTableView *view, QSqlTableModel *model);
    void applySalesViewSettings(QTableView *view, QSqlTableModel *model);

    int empId;
    QString empNo;

    QLineEdit *codeEdit;
    QSpinBox *quantitySpin;
    QLabel *productInfo;
    QLabel *amountInfo;
    QTableView *productsView;
    QTableView *mySalesView;
    QSqlTableModel *productsModel = nullptr;
    QSqlTableModel *salesModel = nullptr;

    bool currentProductValid = false;
    QString currentCode;
    QString currentName;
    QString currentCategory;
    QString currentProducer;
    QString currentPurchaseDate;
    QString currentExpiryDate;
    double currentPrice = 0;
    int currentStock = 0;
    double currentRate = 1.0;
    int currentSalesCount = 0;
};

#endif
