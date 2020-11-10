#pragma once

#include <QtWidgets/QMainWindow>
#include <QTime>
#include <QFileDialog>
#include <QDesktopServices>
#include <QThread>
#include <thread>
#include <functional>
#include <httplib.h>
#include <chaiscript/chaiscript.hpp>

#include "ui_SimpleHttpServer.h"

class SimpleHttpServer : public QMainWindow
{
    Q_OBJECT

public:
    SimpleHttpServer(QWidget *parent = Q_NULLPTR);

    void log(const QString&);
    void log(const std::string&);
    void log(const char*);

protected:
    std::tuple<int, int, int> loadServerApplication();
private:
    std::unique_ptr<httplib::Server> server;
    Ui::SimpleHttpServerClass ui;
    std::vector<std::shared_ptr<chaiscript::ChaiScript>> chaiScripts;

signals:
    void serverStopped(bool);
};
