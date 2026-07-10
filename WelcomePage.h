#pragma once

#include <QMainWindow>
#include "ui_WelcomePage.h"

QT_BEGIN_NAMESPACE
namespace Ui { class WelcomePageClass; };
QT_END_NAMESPACE

class WelcomePage : public QMainWindow
{
	Q_OBJECT

public:
	WelcomePage(QWidget *parent = nullptr);
	~WelcomePage();

private:
	Ui::WelcomePageClass *ui;
};

