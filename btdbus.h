#ifndef BTDBUS_H
#define BTDBUS_H

#include <QObject>
#include <QDBusConnection>
#include <QApplication>
#include <QDBusObjectPath>
#include <QDebug>
#include <BluezQt/Agent>
#include <BluezQt/Device>
#include <BluezQt/Request>
#include <BluezQt/Manager>
#include <BluezQt/Job>
#include <BluezQt/InitManagerJob>
#include <BluezQt/PendingCall>
#include <BluezQt/Adapter>
#include <BluezQt/MediaPlayer>
#include <BluezQt/Profile>
#include <BluezQt/Services>
#include "mainwindow.h"

using namespace BluezQt;

class BTDBus : public BluezQt::Agent
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.thesuite.tsbt")

    Q_PROPERTY(bool BluetoothEnabled READ BluetoothEnabled WRITE setBluetoothEnabled NOTIFY BluetoothEnabledChanged)
    Q_PROPERTY(bool isConnected READ isConnected)

public:
    explicit BTDBus(QObject *parent = 0);

    QDBusObjectPath objectPath() const;
    void displayPinCode(DevicePtr device, const QString &pinCode);
    void requestAuthorization(DevicePtr device, const Request<> &request);
    void requestPinCode(DevicePtr device, const Request<QString> &request);
    void displayPasskey(DevicePtr device, const QString &passkey, const QString &entered);
    void requestConfirmation(DevicePtr device, const QString &passkey, const Request<> &request);
    void authorizeService(DevicePtr device, const QString &uuid, const Request<> &request);
    void cancel();

    bool BluetoothEnabled();
    void setBluetoothEnabled(bool enabled);

    bool isConnected();

    Manager* btMan;
signals:
    Q_SCRIPTABLE void BluetoothEnabledChanged();

private slots:
    void ActionInvoked(uint id, QString action);

public Q_SLOTS:
    Q_SCRIPTABLE void showSettings();
    Q_SCRIPTABLE void exit();

private:
    Request<> currentRequest;

    uint notificationNumber = 0;

};

#endif // BTDBUS_H
