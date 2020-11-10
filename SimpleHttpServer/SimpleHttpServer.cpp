#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

#include "SimpleHttpServer.h"
#include <chaiscript/chaiscript.hpp>

std::tuple<int, int, int> SimpleHttpServer::loadServerApplication()
{
    auto serverDir = QDir(ui.pathLineEdit->text() + "/app");
    serverDir.setFilter(QDir::Files | QDir::NoSymLinks);
    QStringList chaiApplicationFilters = { "*.chaiapplication" ,"*.chaiapp", "*.chp" };
    // ReSharper disable once StringLiteralTypo
    serverDir.setNameFilters(chaiApplicationFilters);
    auto appInfoList = serverDir.entryInfoList(chaiApplicationFilters);
    int appTotal = 0, appSuccess = 0, appFailed = 0;
    chaiScripts.clear();

    log("正在从" + serverDir.absolutePath() + "加载小程序...");

    QString chaiFilePathList;
    for (const auto& appFileInfo : appInfoList)
        chaiFilePathList += appFileInfo.absoluteFilePath();
    for (const auto& appFileInfo : appInfoList) 
    {
        try {
            chaiScripts.push_back(std::make_shared<chaiscript::ChaiScript>());
            auto& chai = *chaiScripts.back();
            auto chaiFilePath = appFileInfo.absoluteFilePath().toStdString();
            chai.use(chaiFilePath);
            auto path = chai.eval<std::string>("pattern");
        	
            bool hasGetMethod = false;
            bool hasPostMethod = false;
            try
            {
                auto chaiGet = chai.eval <std::function<void(std::string, std::string&, std::string&)>>("Get");
            	server->Get(path.c_str(), [chaiGet](const httplib::Request& request, httplib::Response& response)
                    {
                        std::string res, type;
                        chaiGet(request.body, res, type);
                        response.set_content(res, type.c_str());
                    });
                hasGetMethod = true;
            }
            catch (chaiscript::exception::eval_error e)
            {
            }
        	catch (chaiscript::exception::bad_boxed_cast e)
        	{
        	}
            try
            {
                auto chaiPost = chai.eval <std::function<void(std::string, std::string&, std::string&)>>("Post");
                server->Post(path.c_str(), [chaiPost](const httplib::Request& request, httplib::Response& response)
                    {
                        std::string res, type;
                        chaiPost(request.body, res, type);
                        response.set_content(res, type.c_str());
                    });
                hasPostMethod = true;
            }
            catch (chaiscript::exception::eval_error e)
            {
            }
            catch (chaiscript::exception::bad_boxed_cast e)
            {
            }
            if (hasGetMethod || hasPostMethod)
                appSuccess++;
        }
        catch (...)
        {
            log("编译小程序" + appFileInfo.fileName() + "时发生错误!请检查其语法!");
            appFailed++;
        }
        appTotal++;
    }
    return std::make_tuple(appTotal, appSuccess, appFailed);
}

SimpleHttpServer::SimpleHttpServer(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    ui.stopPushButton->setEnabled(false);

    connect(ui.pathLineEdit, &SupperLineEdit::clicked, [&]()
        {
	        const auto path{ QFileDialog::getExistingDirectory(this, "网站根目录", "") };
            if (!path.isEmpty())
            {
                ui.pathLineEdit->setText(path);
                ui.pathLineEdit->setToolTip(path + "\r\n【不支持中文路径】");
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
            log(QString::asprintf("服务器停止.%s", b ? "" : "发生错误."));
            ui.ipLineEdit->setEnabled(true);
            ui.portLineEdit->setEnabled(true);
            ui.pathLineEdit->setEnabled(true);
            ui.startPushButton->setEnabled(true);
            ui.stopPushButton->setEnabled(false);
        }, Qt::QueuedConnection);
    connect(ui.startPushButton, &QPushButton::clicked, [&]()
        {
            ui.ipLineEdit->setEnabled(false);
            ui.portLineEdit->setEnabled(false);
            ui.pathLineEdit->setEnabled(false);
            ui.startPushButton->setEnabled(false);
    	
            server = std::make_unique<httplib::Server>();
            log(QString::asprintf("网站根目录 %s.", QDir(ui.pathLineEdit->text()).absolutePath().toStdString().c_str()));    	
            auto [appTotal, appSuccess, appFailed] = loadServerApplication();
            log(QString::asprintf("发现 %d 个服务器小程序，%d 个可用小程序, %d 个小程序加载失败.", appTotal, appSuccess, appFailed));

            server->set_error_handler([&](const httplib::Request& req, httplib::Response& res)
                {
                    log(QString::asprintf("[ERROR](%s:%d)target %s.", req.remote_addr.c_str(), req.remote_port, req.target.c_str()));
                });

            //server->set_logger([&](const httplib::Request& req, const httplib::Response& res)
            //    {
            //        log(QString::asprintf("[%s]target:%s respons:%s", req.remote_addr.c_str(), req.target.c_str(), res.body.c_str()));
            //    });
    	
			server->bind_to_port(ui.ipLineEdit->text().toStdString().c_str()
				, ui.portLineEdit->text().toInt());
			server->remove_mount_point("/");
			server->set_base_dir(ui.pathLineEdit->text().toStdString().c_str());


            std::thread th([&]()
                {
                    auto mark = server->listen_after_bind();
                    server.reset();
                    chaiScripts.clear();
                    emit serverStopped(mark);
                });
            th.detach();
            log(QString::asprintf("服务器已在 %s:%d 开放.", ui.ipLineEdit->text().toStdString().c_str(), ui.portLineEdit->text().toInt()));


            ui.stopPushButton->setEnabled(true);

        });
    connect(ui.stopPushButton, &QPushButton::clicked, [this] {if (server)server->stop(); });
}

Q_INVOKABLE void SimpleHttpServer::log(const QString& s)
{
    ui.logTextBrowser->moveCursor(QTextCursor::End);
    ui.logTextBrowser->append(QTime::currentTime().toString("[HH:mm:ss]") + s);
}

Q_INVOKABLE void SimpleHttpServer::log(const std::string& s)
{
    return log(QString::fromStdString(s));
}

Q_INVOKABLE void SimpleHttpServer::log(const char* s)
{
    return log(QString::fromStdString(s));
}
