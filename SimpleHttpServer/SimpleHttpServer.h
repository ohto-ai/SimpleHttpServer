#pragma once

#include <QtWidgets/QMainWindow>
#include <QTime>
#include <QFileDialog>
#include <QDesktopServices>
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
    void closeEvent(QCloseEvent*) override;
    std::tuple<int, int, int> loadServerApplication();
private:
    std::unique_ptr<httplib::Server> server;
    std::shared_future<void> serverFuture;;
    Ui::SimpleHttpServerClass ui;

    const chaiscript::ModulePtr chaiModuleHttpPtr = std::make_shared<chaiscript::Module>();
    const chaiscript::ModulePtr chaiModuleStringPtr = std::make_shared<chaiscript::Module>();
	
    std::vector<std::shared_ptr<chaiscript::ChaiScript>> chaiScripts;

signals:
    void serverStopped(bool);
};
