#include "WelcomePage.h"

WelcomePage::WelcomePage(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::WelcomePageClass())
{
	ui->setupUi(this);
}

WelcomePage::~WelcomePage()
{
	delete ui;
}

