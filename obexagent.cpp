#include "obexagent.h"

extern MainWindow* MainWin;
extern BTDBus* dbusHandler;

OBEXAgent::OBEXAgent(QObject *parent) : ObexAgent(parent)
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.connect("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", "ActionInvoked", this, SLOT(ActionInvoked(uint,QString)));
    ObexMan = new ObexManager;
    dbusHandler->obexMan = ObexMan;

    bool jobComplete = false;
    InitObexManagerJob* job = ObexMan->init();
    connect(job, &InitObexManagerJob::result, [=, &jobComplete]() {
        this->ObexMan->startService()->waitForFinished();
        PendingCall* calls;
        calls = ObexMan->registerAgent(this);
        calls->waitForFinished();
        jobComplete = true;
    });

    job->start();

}

QDBusObjectPath OBEXAgent::objectPath() const {
    return QDBusObjectPath("/org/thesuite/tsbt/obex");
}

void OBEXAgent::authorizePush(ObexTransferPtr transfer, ObexSessionPtr session, const Request<QString> &request) {
    qDebug() << session.data()->objectPath().path();
    qDebug() << transfer.data()->size();
    currentRequest = request;
    currentTransfer = transfer;
    fileType = transfer.data()->type();

    QString source = session.data()->source();
    QString dest = session.data()->destination();

    //Adapter* adp = dbusHandler->btMan->adapterForAddress(session.data()->source()).data();
    //Device* dev = adp->deviceForAddress(session.data()->destination()).data();
    Device* dev = dbusHandler->btMan->deviceForAddress(dest).data();
    currentDevice = dev->name();
    QMetaObject::invokeMethod(this, "startFile", Qt::QueuedConnection);
    connect(transfer.data(), &ObexTransfer::statusChanged, [=](ObexTransfer::Status status) {
        qDebug() << status;
    });
}

void OBEXAgent::startFile() {
    QDBusInterface interface("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
    QVariantMap hints;
    hints.insert("transient", true);

    QStringList actions;
    actions.append("true");
    actions.append("Save As...");
    actions.append("false");
    actions.append("Cancel");

    QList<QVariant> args;
    args << "ts-bt" << notificationNumber << "" << "Incoming Bluetooth File" <<
                            currentDevice + " is sending \"" + currentTransfer.data()->fileName() + ".\"" <<
                            actions << hints << (int) 0;

    QDBusReply<uint> reply = interface.callWithArgumentList(QDBus::Block, "Notify", args);
    this->notificationNumber = reply.value();
}

void OBEXAgent::ActionInvoked(uint id, QString action) {
    if (id == this->notificationNumber) {
        if (action == "true") {
            QFileDialog* dialog = new QFileDialog;
            dialog->setAcceptMode(QFileDialog::AcceptSave);
            dialog->setMimeTypeFilters(QStringList() << fileType);
            if (dialog->exec() == QFileDialog::Accepted) {
                QString fileName = dialog->selectedFiles().first();
                currentRequest.accept("/home/victor/.testfile.png");
                qDebug() << currentTransfer.data()->fileName();
            } else {
                currentRequest.cancel();
            }
        } else {
            currentRequest.cancel();
        }
    }
}
