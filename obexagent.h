#ifndef OBEXAGENT_H
#define OBEXAGENT_H

#include <QObject>
#include <BluezQt/ObexManager>
#include <BluezQt/InitObexManagerJob>
#include <BluezQt/ObexAgent>
#include <BluezQt/ObexTransfer>
#include <BluezQt/ObexSession>
#include <QFileDialog>
#include <QDBusInterface>
#include "mainwindow.h"


class OBEXAgent : public ObexAgent
{
    Q_OBJECT
public:
    explicit OBEXAgent(QObject *parent = 0);

    QDBusObjectPath objectPath() const;
    void authorizePush(ObexTransferPtr transfer, ObexSessionPtr session, const Request<QString> &request);

    ObexManager* ObexMan;
signals:

public slots:
    void ActionInvoked(uint id, QString action);

private Q_SLOTS:
    void startFile();

private:
    uint notificationNumber = 0;
    Request<QString> currentRequest;
    ObexTransferPtr currentTransfer;
    QString fileType;
    QString currentDevice;
};

#endif // OBEXAGENT_H
