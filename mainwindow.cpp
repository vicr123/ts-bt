#include "mainwindow.h"
#include "ui_mainwindow.h"

extern BTDBus* dbusHandler;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->paneSelection->addItem(new QListWidgetItem(QIcon::fromTheme("smartphone"), "Devices"));
    ui->paneSelection->addItem(new QListWidgetItem(QIcon::fromTheme("preferences-system-bluetooth"), "Settings"));

    ui->devControlFrame->setVisible(false);

    connect(dbusHandler->btMan, SIGNAL(deviceAdded(DevicePtr)), this, SLOT(reloadDevices()));
    connect(dbusHandler->btMan, SIGNAL(deviceRemoved(DevicePtr)), this, SLOT(reloadDevices()));
    connect(dbusHandler->btMan, SIGNAL(deviceChanged(DevicePtr)), this, SLOT(reloadDevices()));
    connect(dbusHandler->btMan, SIGNAL(usableAdapterChanged(AdapterPtr)), this, SLOT(reloadAdapter()));
    connect(dbusHandler->btMan, SIGNAL(bluetoothBlockedChanged(bool)), this, SLOT(reloadAdapter()));

    reloadDevices();
    reloadAdapter();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::reloadDevices() {
    int currentRow = ui->devicesList->currentRow();
    devices.clear();
    ui->devicesList->clear();

    for (DevicePtr devPtr : dbusHandler->btMan->devices()) {
        Device* dev = devPtr.data();
        devices.append(dev);

        QListWidgetItem* item = new QListWidgetItem();
        item->setText(dev->name());
        item->setIcon(QIcon::fromTheme(dev->icon()));

        if (dev->isConnected()) {
            item->setBackground(QBrush(QColor(0, 255, 0, 100)));
        }

        ui->devicesList->addItem(item);
    }

    if (currentRow != -1 && currentRow <= ui->devicesList->count()) {
        ui->devicesList->setCurrentRow(currentRow);
    }
}

void MainWindow::reloadAdapter() {
    Adapter* adp = dbusHandler->btMan->usableAdapter().data();
    if (adp == NULL) {
        if (dbusHandler->btMan->isBluetoothBlocked()) {
            ui->visibleLabel->setVisible(true);
            ui->visibleLabel->setText("Bluetooth has been turned off.");
        }
    } else {
        ui->visibilityBox->setChecked(adp->isDiscoverable());
        ui->visibleLabel->setVisible(adp->isDiscoverable());
        ui->visibleLabel->setText("Your device is visible under the name \"" + adp->name() + "\"");
        ui->bluetoothEnabled->setChecked(!dbusHandler->btMan->isBluetoothBlocked());
    }
}

void MainWindow::show() {
    ui->scanButton->click();
    QMainWindow::show();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (dbusHandler->btMan->usableAdapter()) {
        dbusHandler->btMan->usableAdapter().data()->stopDiscovery();
    }
}

void MainWindow::on_paneSelection_currentRowChanged(int currentRow)
{
    ui->panes->setCurrentIndex(currentRow);
}

void MainWindow::on_devicesList_currentRowChanged(int currentRow)
{
    if (currentRow == -1) {
        ui->devControlFrame->setVisible(false);
    } else {
        ui->devControlFrame->setVisible(true);
        Device* dev = devices.at(currentRow);
        if (dev->isPaired()) {
            ui->pairButton->setVisible(false);
            ui->removeButton->setVisible(true);
            ui->connectButton->setVisible(true);

            if (dev->isConnected()) {
                ui->disconnectButton->setVisible(true);
            } else {
                ui->disconnectButton->setVisible(false);
            }
        } else {
            ui->pairButton->setVisible(true);
            ui->removeButton->setVisible(false);
            ui->connectButton->setVisible(false);
        }

        QMenu* menu = new QMenu(this);
        for (QString uuid : dev->uuids()) {
            QAction* action = new QAction();
            action->setData(uuid);
            if (uuid == Services::AudioSource) {
                action->setText("Audio Source");
            } else if (uuid == "0000110B-0000-1000-8000-00805F9B34FB") {
                action->setText("Audio Player");
            } else if (uuid == Services::Panu) {
                action->setText("Internet Connection");
            } else if (uuid == Services::HumanInterfaceDevice) {
                action->setText("Human Interface Device");
            } else if (uuid == Services::SimAccess) {
                action->setText("SIM Access");
            } else if (uuid == Services::PhonebookAccessServer) {
                action->setText("Address Book");
            } else if (uuid == Services::Handsfree || uuid == Services::HandsfreeAudioGateway) {
                action->setText("Handsfree");
            }

            if (action->text() == "") {
                action->deleteLater();
            } else {
                menu->addAction(action);
            }
        }

        ui->connectButton->setMenu(menu);
        connect(menu, &QMenu::triggered, [=](QAction* action) {
            QString uuid = action->data().toString();
            PendingCall* call = devices.at(ui->devicesList->currentRow())->connectProfile(uuid);
            connect(call, &PendingCall::finished, [=]() {
                if (uuid == Services::Panu) {
                    QDBusMessage getMessage = QDBusMessage::createMethodCall("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", "GetDeviceByIpIface");

                    QVariantList getArgs;
                    getArgs.append(dev->address());

                    getMessage.setArguments(getArgs);

                    QDBusReply<QDBusObjectPath> path = QDBusConnection::systemBus().call(getMessage);

                    if (path.isValid()) {
                        QDBusMessage activateMessage = QDBusMessage::createMethodCall("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", "ActivateConnection");

                        QVariantList activateArgs;
                        activateArgs.append(QVariant::fromValue(QDBusObjectPath("/")));
                        activateArgs.append(QVariant::fromValue(path.value()));
                        activateArgs.append(QVariant::fromValue(QDBusObjectPath("/")));

                        activateMessage.setArguments(activateArgs);

                        QDBusConnection::systemBus().call(activateMessage);
                    } else {
                        qDebug() << "Connect failed!";
                    }
                }
               call->deleteLater();
            });
        });

        ui->deviceName->setText(dev->name());
        QString type;
        switch (dev->type()) {
        case Device::Phone:
            type = "Mobile Phone";
            break;
        case Device::Computer:
            type = "Computer";
            break;
        case Device::AudioVideo:
            type = "AV";
            break;
        case Device::Headphones:
        case Device::Headset:
            type = "Speaker";
            break;
        case Device::Camera:
            type = "Camera";
            break;
        case Device::Health:
            type = "Health";
            break;
        case Device::Imaging:
            type = "Imaging";
            break;
        case Device::Keyboard:
            type = "Keyboard";
            break;
        case Device::Mouse:
            type = "Mouse";
            break;
        case Device::Joypad:
            type = "Joypad";
            break;
        case Device::Peripheral:
            type = "Peripheral";
            break;
        case Device::Printer:
            type = "Printer";
            break;
        case Device::Network:
            type = "Network";
            break;
        case Device::Modem:
            type = "Modem";
            break;
        default:
            type = "Unknown";
            break;
        }

        ui->deviceType->setText(type);
    }
}

void MainWindow::on_removeButton_clicked()
{
    if (QMessageBox::warning(this, "Remove Device", "If you remove this device, you won't be able to connect to it until you pair it again. Do you want to remove this device?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
        dbusHandler->btMan->usableAdapter().data()->removeDevice(devices.at(ui->devicesList->currentRow())->toSharedPtr());
    }
}

void MainWindow::on_pairButton_clicked()
{
    devices.at(ui->devicesList->currentRow())->pair();
}

void MainWindow::on_scanButton_clicked()
{
    if (dbusHandler->btMan->usableAdapter()) {
        dbusHandler->btMan->usableAdapter().data()->startDiscovery();
        ui->scanButton->setVisible(false);
    } else {
        ui->scanButton->setVisible(true);
    }
}

void MainWindow::on_visibilityBox_toggled(bool checked)
{
    dbusHandler->btMan->usableAdapter().data()->setDiscoverable(checked);
}

void MainWindow::on_connectButton_clicked()
{
    //devices.at(ui->devicesList->currentRow())->connectToDevice();
}

void MainWindow::on_disconnectButton_clicked()
{
    devices.at(ui->devicesList->currentRow())->disconnectFromDevice();
}

void MainWindow::on_bluetoothEnabled_toggled(bool checked)
{
    dbusHandler->btMan->setBluetoothBlocked(!checked);
}
