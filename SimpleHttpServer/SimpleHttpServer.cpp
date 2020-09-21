#include "SimpleHttpServer.h"

SimpleHttpServer::SimpleHttpServer(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    server.set_error_handler([&](const httplib::Request& req, httplib::Response& res)
        {
            log(QString::asprintf("ERROR:(%s:%d)target %s.", req.remote_addr.c_str(), req.remote_port, req.target.c_str()));
        });

    ui.stopPushButton->setEnabled(false);

    connect(ui.pathLineEdit, &SupperLineEdit::clicked, [&]()
        {
            auto path{ QFileDialog::getExistingDirectory(this, "Base of website.", "") };
            if (!path.isEmpty())
            {
                ui.pathLineEdit->setText(path);
                ui.pathLineEdit->setToolTip(path);
            }
        });
    connect(ui.serverLabel, &SupperLabel::leftClicked, [&]()
        {
            QDesktopServices::openUrl("http://" + ui.ipLineEdit->text() + ":" + ui.portLineEdit->text());
        });
    connect(ui.serverLabel, &SupperLabel::rightClicked, [&]()
        {
            QDesktopServices::openUrl(QUrl("https://thatboy.info"));
        });
    connect(ui.startPushButton, &QPushButton::clicked, [&]()
        {
            if (server.is_running())
                server.stop();
            server.bind_to_port(ui.ipLineEdit->text().toStdString().c_str()
                , ui.portLineEdit->text().toInt());
            server.remove_mount_point("/");
            server.set_base_dir(ui.pathLineEdit->text().toStdString().c_str());
            log(QString::asprintf("Locate at %s", ui.pathLineEdit->text().toStdString().c_str()));
            std::thread th([&]()
                {
                    ui.ipLineEdit->setEnabled(false);
                    ui.portLineEdit->setEnabled(false);
                    ui.pathLineEdit->setEnabled(false);
                    ui.startPushButton->setEnabled(false);
                    ui.stopPushButton->setEnabled(true);
                    bool b = server.listen_after_bind();
                    log(QString::asprintf("Server stopped.%s", b ? "" : "Error occurred."));
                    ui.ipLineEdit->setEnabled(true);
                    ui.portLineEdit->setEnabled(true);
                    ui.pathLineEdit->setEnabled(true);
                    ui.startPushButton->setEnabled(true);
                    ui.stopPushButton->setEnabled(false);
                });
            th.detach();
            log(QString::asprintf("Server open at %s:%d", ui.ipLineEdit->text().toStdString().c_str(), ui.portLineEdit->text().toInt()));

        });
    connect(ui.stopPushButton, &QPushButton::clicked, std::bind(&httplib::Server::stop, &server));
}

void SimpleHttpServer::log(const QString& s)
{
    ui.logTextBrowser->moveCursor(QTextCursor::End);
    ui.logTextBrowser->append(QTime::currentTime().toString("[HH:mm:ss]") + s);
}
