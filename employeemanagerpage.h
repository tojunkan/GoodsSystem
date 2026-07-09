#ifndef EMPLOYEEMANAGERPAGE_H
#define EMPLOYEEMANAGERPAGE_H

#include <QWidget>

class QTableView;
class QSqlTableModel;

class EmployeeManagerPage : public QWidget
{
    Q_OBJECT
public:
    explicit EmployeeManagerPage(QWidget *parent = nullptr);

public slots:
    void refresh();

private slots:
    void addEmployee();
    void deleteEmployee();
    void editEmployee();

private:
    void applyEmployeeViewSettings();

    QTableView *view;
    QSqlTableModel *model = nullptr;
};

#endif
