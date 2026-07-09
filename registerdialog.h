#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>

class QLineEdit;

class RegisterDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RegisterDialog(QWidget *parent = nullptr);

private slots:
    void doRegister();

private:
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QLineEdit *employeeNoEdit;
    QLineEdit *phoneEdit;
};

#endif
