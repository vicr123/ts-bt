#include "mainwindow.h"
#include "btdbus.h"
#include <QApplication>
#include <QSharedMemory>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

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

    bool alreadyRunning = false;
    if (QDBusConnection::sessionBus().interface()->registeredServiceNames().value().contains("org.thesuite.tsbt")) {
        alreadyRunning = true;
    }

    if (alreadyRunning) {
        QDBusMessage message = QDBusMessage::createMethodCall("org.thesuite.tsbt", "/org/thesuite/tsbt", "org.thesuite.tsbt", "showSettings");
        QDBusConnection::sessionBus().call(message);

        qDebug() << "ts-bt is already running. Showing settings window in already running session.";
        return 0;
    } else {
        dbusHandler = new BTDBus;
        MainWin = new MainWindow();
        if (showWindow) {
            MainWin->show();
        }
    }

    return a.exec();
}
