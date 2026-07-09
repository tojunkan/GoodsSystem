#ifndef PRODUCTDIALOG_H
#define PRODUCTDIALOG_H

#include <QDialog>
#include <QDate>
#include <QString>

class QLineEdit;
class QComboBox;
class QDoubleSpinBox;
class QSpinBox;
class QDateEdit;

class ProductDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ProductDialog(QWidget *parent = nullptr);

    void setProduct(const QString &code,
                    const QString &name,
                    const QString &category,
                    double price,
                    const QString &producer,
                    int stock,
                    const QDate &purchaseDate,
                    const QDate &expiryDate);

    QString code() const;
    QString name() const;
    QString category() const;
    double price() const;
    QString producer() const;
    int stock() const;
    QDate purchaseDate() const;
    QDate expiryDate() const;

private slots:
    void validateAndAccept();
    void completeCodeFromCategory();

private:
    QLineEdit *codeEdit;
    QLineEdit *nameEdit;
    QComboBox *categoryBox;
    QDoubleSpinBox *priceSpin;
    QLineEdit *producerEdit;
    QSpinBox *stockSpin;
    QDateEdit *purchaseDateEdit;
    QDateEdit *expiryDateEdit;
};

#endif
