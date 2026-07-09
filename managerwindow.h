#ifndef MANAGERWINDOW_H
#define MANAGERWINDOW_H

#include <QMainWindow>

class ProductManagerPage;
class EmployeeManagerPage;
class SalesRecordPage;

class ManagerWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit ManagerWindow(QWidget *parent = nullptr);

private:
    ProductManagerPage *productPage;
    EmployeeManagerPage *employeePage;
    SalesRecordPage *salesPage;
};

#endif
