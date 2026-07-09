#ifndef DIGITSTEPDOUBLESPINBOX_H
#define DIGITSTEPDOUBLESPINBOX_H

#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QLocale>
#include <cmath>

// 价格输入框：
// 光标在整数部分时，右侧小箭头按 1 元增减；
// 光标在小数点后一位时，按 0.1 元增减；
// 光标在小数点后两位时，按 0.01 元增减。
class DigitStepDoubleSpinBox : public QDoubleSpinBox
{
public:
    explicit DigitStepDoubleSpinBox(QWidget *parent = nullptr) : QDoubleSpinBox(parent) {}

protected:
    void stepBy(int steps) override
    {
        QLineEdit *edit = lineEdit();
        int cursor = edit ? edit->cursorPosition() : 0;
        QString text = edit ? edit->text() : QString();
        QString point = locale().decimalPoint();

        int dot = text.indexOf(point);
        if (dot < 0) dot = text.indexOf('.');

        double step = 1.0;
        if (dot >= 0 && cursor > dot) {
            int decimalIndex = cursor - dot; // 1 表示小数点后一位，2 表示小数点后两位
            if (decimalIndex <= 1) step = 0.1;
            else step = 0.01;
        }

        setValue(value() + steps * step);
        if (edit) edit->setCursorPosition(cursor);
    }
};

#endif
