#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include "btdbus.h"

using namespace BluezQt;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void show();

private slots:
    void reloadDevices();

    void reloadAdapter();

    void on_paneSelection_currentRowChanged(int currentRow);

    void on_devicesList_currentRowChanged(int currentRow);

    void on_removeButton_clicked();

    void on_pairButton_clicked();

    void on_scanButton_clicked();

    void on_visibilityBox_toggled(bool checked);

    void on_connectButton_clicked();

    void on_disconnectButton_clicked();

private:
    Ui::MainWindow *ui;

    QList<Device*> devices;
    void closeEvent(QCloseEvent *event);
};

#endif // MAINWINDOW_H
