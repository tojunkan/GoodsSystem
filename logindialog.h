#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QString>

class QLineEdit;

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    enum Role { Manager, Employee };

    explicit LoginDialog(Role role, QWidget *parent = nullptr);

    int employeeId() const;
    QString employeeNo() const;

private slots:
    void doLogin();

private:
    Role role;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    int empId = -1;
    QString empNo;
};

#endif
