#include "btdbus.h"
#include "tsbt_adaptor.h"

extern MainWindow* MainWin;

BTDBus::BTDBus(QObject *parent) : BluezQt::Agent(parent)
{
    new TsbtAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/org/thesuite/tsbt", this);
    dbus.registerService("org.thesuite.tsbt");
    dbus.connect("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", "ActionInvoked", this, SLOT(ActionInvoked(uint,QString)));

    btMan = new Manager;

    bool jobComplete = false;
    InitManagerJob* job = btMan->init();
    connect(job, &InitManagerJob::result, [=, &jobComplete]() {
        qDebug() << btMan->isOperational();
        qDebug() << job->errorText();
        PendingCall* calls;
        calls = btMan->registerAgent(this);
        calls->waitForFinished();
        qDebug() << job->errorText();
        btMan->requestDefaultAgent(this);
        calls->waitForFinished();
        qDebug() << job->errorText();
        jobComplete = true;
    });

    job->start();

    //while (!jobComplete) {};

    connect(btMan, &Manager::bluetoothBlockedChanged, [=]() {
        emit BluetoothEnabledChanged();
    });
}

void BTDBus::showSettings() {
    MainWin->show();
}

void BTDBus::exit() {
    QApplication::exit(0);
}

QDBusObjectPath BTDBus::objectPath() const {
    return QDBusObjectPath("/org/thesuite/tsbt");
}

void BTDBus::displayPinCode(DevicePtr device, const QString &pinCode) {
    qDebug() << device.data()->name() + " " + pinCode;
}

void BTDBus::requestAuthorization(DevicePtr device, const Request<> &request) {
    currentRequest = request;

    QDBusInterface interface("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
    QVariantMap hints;
    hints.insert("transient", true);

    QStringList actions;
    actions.append("true");
    actions.append("Pair");
    actions.append("false");
    actions.append("Cancel");

    QList<QVariant> args;
    args << "ts-bt" << notificationNumber << "" << "Bluetooth Pairing" <<
                            device.data()->name() + " wants to pair with your PC." <<
                            actions << hints << (int) 0;

    QDBusReply<uint> reply = interface.callWithArgumentList(QDBus::Block, "Notify", args);
    this->notificationNumber = reply.value();
}

void BTDBus::requestPinCode(DevicePtr device, const Request<QString> &request) {

}

void BTDBus::displayPasskey(DevicePtr device, const QString &passkey, const QString &entered) {
    QDBusInterface interface("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
    QVariantMap hints;
    hints.insert("transient", true);

    QStringList actions;
    actions.append("true");
    actions.append("Pair");
    actions.append("false");
    actions.append("Cancel");

    QList<QVariant> args;
    args << "ts-bt" << notificationNumber << "" << "Bluetooth Pairing" <<
                            "Enter " + passkey + " on " + device.data()->name() + " to pair with this PC." <<
                            actions << hints << (int) 0;

    QDBusReply<uint> reply = interface.callWithArgumentList(QDBus::Block, "Notify", args);
    this->notificationNumber = reply.value();
}

void BTDBus::requestConfirmation(DevicePtr device, const QString &passkey, const Request<> &request) {
    currentRequest = request;

    QDBusInterface interface("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
    QVariantMap hints;
    hints.insert("transient", true);

    QStringList actions;
    actions.append("true");
    actions.append("OK");
    actions.append("false");
    actions.append("Cancel");

    QList<QVariant> args;
    args << "ts-bt" << notificationNumber << "" << "Bluetooth Pairing" <<
                            device.data()->name() + " wants to pair with your PC. Check that the passcode <b>" + passkey + "</b> matches the one shown on the device." <<
                            actions << hints << (int) 0;

    QDBusReply<uint> reply = interface.callWithArgumentList(QDBus::Block, "Notify", args);
    this->notificationNumber = reply.value();
}

void BTDBus::authorizeService(DevicePtr device, const QString &uuid, const Request<> &request) {
    currentRequest = request;

    QDBusInterface interface("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
    QVariantMap hints;
    hints.insert("transient", true);

    QStringList actions;
    actions.append("true");
    actions.append("OK");
    actions.append("false");
    actions.append("Cancel");

    QList<QVariant> args;
    args << "ts-bt" << notificationNumber << "" << "Bluetooth Connection" <<
                            device.data()->name() + " wants to connect to your PC." <<
                            actions << hints << (int) 0;

    QDBusReply<uint> reply = interface.callWithArgumentList(QDBus::Block, "Notify", args);
    this->notificationNumber = reply.value();
}

void BTDBus::cancel() {

}

bool BTDBus::BluetoothEnabled() {
    return !btMan->isBluetoothBlocked();
}

void BTDBus::setBluetoothEnabled(bool enabled) {
    btMan->setBluetoothBlocked(!enabled);
}

void BTDBus::ActionInvoked(uint id, QString action) {
    if (id == this->notificationNumber) {
        if (action == "true") {
            currentRequest.accept();
        } else {
            currentRequest.reject();
        }
    }
}

bool BTDBus::isConnected() {
    bool connected = false;
    for (DevicePtr device : btMan->devices()) {
        if (device.data()->isConnected()) {
            connected = true;
        }
    }
    return connected;
}
