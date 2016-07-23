#include "mainwindow.h"
#include "btdbus.h"
#include <QApplication>
#include <QSharedMemory>
#include <QDebug>

MainWindow* MainWin;
BTDBus* dbusHandler;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);

    bool showWindow;

    for (QString arg : a.arguments()) {
        if (arg == "--help" || arg == "-h") {
            qDebug() << "theShell Bluetooth Manager";
            qDebug() << "Usage: ts-bt [OPTIONS]";
            qDebug() << "  -s, --settings               Show the Settings UI";
            return 0;
        } else if (arg == "--settings" || arg == "-s") {
            showWindow = true;
        }
    }

    dbusHandler = new BTDBus;

    MainWin = new MainWindow();
    if (showWindow) {
        MainWin->show();
    }

    return a.exec();
}
