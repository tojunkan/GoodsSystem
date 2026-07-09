#ifndef SALESRECORDPAGE_H
#define SALESRECORDPAGE_H

#include <QWidget>

class QComboBox;
class QSpinBox;
class QLabel;
class QTableView;
class QSqlTableModel;

class SalesRecordPage : public QWidget
{
    Q_OBJECT
public:
    explicit SalesRecordPage(QWidget *parent = nullptr);

public slots:
    void loadEmployees();
    void queryRecords();

private:
    void setupHeaders(QSqlTableModel *model);

    QComboBox *employeeBox;
    QSpinBox *yearSpin;
    QSpinBox *monthSpin;
    QLabel *totalLabel;
    QTableView *view;
    QSqlTableModel *model = nullptr;
};

#endif
