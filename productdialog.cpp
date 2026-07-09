#include "productdialog.h"
#include "database.h"
#include "digitstepdoublespinbox.h"

#include <QtWidgets>
#include <QRegularExpression>

ProductDialog::ProductDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("商品信息");
    resize(430, 360);

    codeEdit = new QLineEdit;
    nameEdit = new QLineEdit;

    categoryBox = new QComboBox;
    categoryBox->setEditable(true);
    categoryBox->addItems({"生活类", "食品类", "饮料类", "日化类", "家电类", "文具类", "其他"});

    priceSpin = new DigitStepDoubleSpinBox;
    priceSpin->setMaximum(9999999);
    priceSpin->setDecimals(2);
    priceSpin->setPrefix("￥");
    priceSpin->setSingleStep(0.01);

    producerEdit = new QLineEdit;

    stockSpin = new QSpinBox;
    stockSpin->setMaximum(999999999);

    purchaseDateEdit = new QDateEdit(QDate::currentDate());
    purchaseDateEdit->setCalendarPopup(true);
    purchaseDateEdit->setDisplayFormat("yyyy-MM-dd");

    expiryDateEdit = new QDateEdit(QDate::currentDate().addYears(1));
    expiryDateEdit->setCalendarPopup(true);
    expiryDateEdit->setDisplayFormat("yyyy-MM-dd");

    auto *form = new QFormLayout;
    form->addRow("商品编号：", codeEdit);
    form->addRow("商品名称：", nameEdit);
    form->addRow("商品分类：", categoryBox);
    form->addRow("商品单价：", priceSpin);
    form->addRow("生产商：", producerEdit);
    form->addRow("库存量：", stockSpin);
    form->addRow("进货日期：", purchaseDateEdit);
    form->addRow("到期日期：", expiryDateEdit);

    auto *okBtn = new QPushButton("确定");
    auto *cancelBtn = new QPushButton("取消");

    auto *btns = new QHBoxLayout;
    btns->addStretch();
    btns->addWidget(okBtn);
    btns->addWidget(cancelBtn);

    auto *main = new QVBoxLayout(this);
    main->addLayout(form);
    main->addLayout(btns);

    connect(okBtn, &QPushButton::clicked, this, &ProductDialog::validateAndAccept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(codeEdit, &QLineEdit::editingFinished, this, &ProductDialog::completeCodeFromCategory);
    connect(categoryBox, &QComboBox::currentTextChanged, this, &ProductDialog::completeCodeFromCategory);
}

void ProductDialog::setProduct(const QString &code,
                               const QString &name,
                               const QString &category,
                               double price,
                               const QString &producer,
                               int stock,
                               const QDate &purchaseDate,
                               const QDate &expiryDate)
{
    codeEdit->setText(code);
    codeEdit->setEnabled(false);
    nameEdit->setText(name);
    categoryBox->setCurrentText(category);
    priceSpin->setValue(price);
    producerEdit->setText(producer);
    stockSpin->setValue(stock);
    purchaseDateEdit->setDate(purchaseDate.isValid() ? purchaseDate : QDate::currentDate());
    expiryDateEdit->setDate(expiryDate.isValid() ? expiryDate : QDate::currentDate().addYears(1));
}


void ProductDialog::completeCodeFromCategory()
{
    if (!codeEdit->isEnabled()) return;

    QString raw = codeEdit->text().trimmed();
    if (raw.isEmpty()) return;

    QString numberPart = raw;
    QRegularExpression onlyNumber("^\\d+$");
    QRegularExpression knownCode("^([FLDSAWO])(\\d+)$", QRegularExpression::CaseInsensitiveOption);

    auto match = knownCode.match(raw);
    if (match.hasMatch()) {
        numberPart = match.captured(2);
    } else if (!onlyNumber.match(raw).hasMatch()) {
        return;
    }

    QString category = categoryBox->currentText().trimmed();
    QString prefix = "O";

    if (category == "食品类") prefix = "F";
    else if (category == "生活类") prefix = "L";
    else if (category == "饮料类") prefix = "D";
    else if (category == "日化类") prefix = "S";
    else if (category == "家电类") prefix = "A";
    else if (category == "文具类") prefix = "W";
    else if (category == "其他") prefix = "O";
    else prefix = category.left(1).toUpper();

    bool ok = false;
    int number = numberPart.toInt(&ok);
    if (!ok) return;

    QString numberText = QString("%1").arg(number, 3, 10, QChar('0'));
    codeEdit->setText(prefix + numberText);
}

QString ProductDialog::code() const { return codeEdit->text().trimmed(); }
QString ProductDialog::name() const { return nameEdit->text().trimmed(); }
QString ProductDialog::category() const { return categoryBox->currentText().trimmed(); }
double ProductDialog::price() const { return priceSpin->value(); }
QString ProductDialog::producer() const { return producerEdit->text().trimmed(); }
int ProductDialog::stock() const { return stockSpin->value(); }
QDate ProductDialog::purchaseDate() const { return purchaseDateEdit->date(); }
QDate ProductDialog::expiryDate() const { return expiryDateEdit->date(); }

void ProductDialog::validateAndAccept()
{
    completeCodeFromCategory();
    if (code().isEmpty() || name().isEmpty() || category().isEmpty()) {
        showError(this, "商品编号、名称、分类不能为空。");
        return;
    }

    if (price() <= 0) {
        showError(this, "商品单价必须大于 0。");
        return;
    }

    if (expiryDate() < purchaseDate()) {
        showError(this, "到期日期不能早于进货日期。");
        return;
    }

    accept();
}
