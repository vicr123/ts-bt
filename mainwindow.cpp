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

    connect(dbusHandler->btMan, SIGNAL(deviceAdded(DevicePtr)), this, SLOT(reloadDevices()));
    connect(dbusHandler->btMan, SIGNAL(deviceRemoved(DevicePtr)), this, SLOT(reloadDevices()));
    connect(dbusHandler->btMan, SIGNAL(deviceChanged(DevicePtr)), this, SLOT(reloadDevices()));
    connect(dbusHandler->btMan, SIGNAL(usableAdapterChanged(AdapterPtr)), this, SLOT(reloadAdapter()));

    reloadDevices();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::reloadDevices() {
    devices.clear();
    ui->devicesList->clear();

    for (DevicePtr devPtr : dbusHandler->btMan->devices()) {
        Device* dev = devPtr.data();
        devices.append(dev);

        QListWidgetItem* item = new QListWidgetItem();
        item->setText(dev->name());

        item->setIcon(QIcon::fromTheme(dev->icon()));
        ui->devicesList->addItem(item);
    }
}

void MainWindow::reloadAdapter() {
    Adapter* adp = dbusHandler->btMan->usableAdapter().data();
    ui->visibilityBox->setChecked(adp->isDiscoverable());
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
            if (dev->isConnected()) {
                ui->connectButton->setVisible(false);
                ui->disconnectButton->setVisible(true);
            } else {
                ui->connectButton->setVisible(true);
                ui->disconnectButton->setVisible(false);
            }
        } else {
            ui->pairButton->setVisible(true);
            ui->removeButton->setVisible(false);
            ui->connectButton->setVisible(false);
        }
    }
}

void MainWindow::on_removeButton_clicked()
{
    if (QMessageBox::warning(this, "Remove Device", "If you remove this device, you won't be able to connect to it until you pair it again. Do you want to remove this device?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
        devices.at(ui->devicesList->currentRow())->cancelPairing();
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
    devices.at(ui->devicesList->currentRow())->connectToDevice();
}

void MainWindow::on_disconnectButton_clicked()
{
    devices.at(ui->devicesList->currentRow())->disconnectFromDevice();
}
