#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

#include "SimpleHttpServer.h"

SimpleHttpServer::SimpleHttpServer(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    server.set_error_handler([&](const httplib::Request& req, httplib::Response& res)
        {
            log(QString::asprintf("[ERROR](%s:%d)target %s.", req.remote_addr.c_str(), req.remote_port, req.target.c_str()));
        });

    server.set_logger([&](const httplib::Request& req, const httplib::Response& res)
        {
            log(QString::asprintf("[%s]target:%s respons:%s", req.remote_addr.c_str(), req.target.c_str(), res.body.c_str()));
        });

    ui.stopPushButton->setEnabled(false);

    connect(ui.pathLineEdit, &SupperLineEdit::clicked, [&]()
        {
            auto path{ QFileDialog::getExistingDirectory(this, "��վ��Ŀ¼", "") };
            if (!path.isEmpty())
            {
                ui.pathLineEdit->setText(path);
                ui.pathLineEdit->setToolTip(path+"\r\n����֧������·����");
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

    connect(this, &SimpleHttpServer::serverStopped, this, [&](bool b)
        {
            log(QString::asprintf("������ֹͣ.%s", b ? "" : "��������."));
            ui.ipLineEdit->setEnabled(true);
            ui.portLineEdit->setEnabled(true);
            ui.pathLineEdit->setEnabled(true);
            ui.startPushButton->setEnabled(true);
            ui.stopPushButton->setEnabled(false);
        }, Qt::QueuedConnection);
    connect(ui.startPushButton, &QPushButton::clicked, [&]()
        {
            log(QString::asprintf("��վ��Ŀ¼ %s.", ui.pathLineEdit->text().toStdString().c_str()));

			server.stop();
			server.bind_to_port(ui.ipLineEdit->text().toStdString().c_str()
				, ui.portLineEdit->text().toInt());
			server.remove_mount_point("/");
			server.set_base_dir(ui.pathLineEdit->text().toStdString().c_str());

			ui.ipLineEdit->setEnabled(false);
			ui.portLineEdit->setEnabled(false);
			ui.pathLineEdit->setEnabled(false);
			ui.startPushButton->setEnabled(false);
			ui.stopPushButton->setEnabled(true);

            std::thread th([&]()
                {
                    emit serverStopped(server.listen_after_bind());
                });
            th.detach();
            log(QString::asprintf("���������� %s:%d ����.", ui.ipLineEdit->text().toStdString().c_str(), ui.portLineEdit->text().toInt()));

        });
    connect(ui.stopPushButton, &QPushButton::clicked, std::bind(&httplib::Server::stop, &server));
}

Q_INVOKABLE void SimpleHttpServer::log(const QString& s)
{
    ui.logTextBrowser->moveCursor(QTextCursor::End);
    ui.logTextBrowser->append(QTime::currentTime().toString("[HH:mm:ss]") + s);
}
