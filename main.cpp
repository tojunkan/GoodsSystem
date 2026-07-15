// main.cpp
#include <QApplication>
#include "MainWindow.h"   // 这个我们稍后建

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MainWindow w;
    w.show();

    return app.exec();
}