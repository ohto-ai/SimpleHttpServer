#pragma once

#include <QtWidgets/QMainWindow>
#include <QTime>
#include <QFileDialog>
#include <QDesktopServices>
#include <thread>
#include <functional>
#include <httplib.h>
#include "ui_SimpleHttpServer.h"

class SimpleHttpServer : public QMainWindow
{
    Q_OBJECT

public:
    SimpleHttpServer(QWidget *parent = Q_NULLPTR);

    void log(const QString&);
private:
    httplib::Server server;
    Ui::SimpleHttpServerClass ui;
};
