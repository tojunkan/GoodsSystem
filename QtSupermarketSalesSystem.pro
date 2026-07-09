QT += widgets sql
CONFIG += c++17
TARGET = QtSupermarketSalesSystem

SOURCES += \
    main.cpp \
    database.cpp \
    logindialog.cpp \
    registerdialog.cpp \
    productdialog.cpp \
    mainmenu.cpp \
    managerwindow.cpp \
    productmanagerpage.cpp \
    salesrecordpage.cpp \
    employeemanagerpage.cpp \
    employeesaleswindow.cpp

HEADERS += \
    database.h \
    toggletableview.h \
    digitstepdoublespinbox.h \
    logindialog.h \
    registerdialog.h \
    productdialog.h \
    mainmenu.h \
    managerwindow.h \
    productmanagerpage.h \
    salesrecordpage.h \
    employeemanagerpage.h \
    employeesaleswindow.h
