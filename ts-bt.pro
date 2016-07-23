#-------------------------------------------------
#
# Project created by QtCreator 2016-07-22T23:03:54
#
#-------------------------------------------------

QT       += core gui dbus BluezQt
CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ts-bt
TEMPLATE = app
INCLUDEPATH += /usr/include/KF5/BluezQt

DBUS_ADAPTORS += mainDBus
mainDBus.files = org.thesuite.tsbt.xml
#mainDBus.header_flags = -l BTDBus -i btdbus.h

SOURCES += main.cpp\
    btdbus.cpp \
    mainwindow.cpp

HEADERS  += mainwindow.h \
    btdbus.h

FORMS    += mainwindow.ui
