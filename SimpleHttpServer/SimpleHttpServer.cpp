#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

#include "SimpleHttpServer.h"

std::tuple<int, int, int> SimpleHttpServer::loadServerApplication()
{
    auto serverDir = QDir(ui.pathLineEdit->text() + "/app");
    const QStringList chaiApplicationFilters = { "*.chai", "*.chaiapplication" ,"*.chaiapp", "*.chp" };
    auto appTotal = 0, appSuccess = 0, appFailed = 0;
	
    serverDir.setFilter(QDir::Files | QDir::NoSymLinks);
    serverDir.setNameFilters(chaiApplicationFilters);
    auto appInfoList = serverDir.entryInfoList(chaiApplicationFilters);
	
    log("Loading applications in " + serverDir.absolutePath());
    	
    chaiScripts.clear();
    for (const auto& appFileInfo : appInfoList) 
    {
        try {
            auto chai_ptr = std::make_shared<chaiscript::ChaiScript>();
            auto& chai = *chai_ptr;
        	
            chai.add(chaiModuleHttpPtr);
            chai.add(chaiModuleStringPtr);
            chai.add(chaiscript::fun([=](const std::string& message) {log("<" + appFileInfo.fileName() + ">: " + message.c_str()); }), "log");

            chai.use(appFileInfo.absoluteFilePath().toStdString());
        	
            auto path = chai.eval<std::string>("pattern");        	
            bool hasGetMethod = false;
            bool hasPostMethod = false;

            try
            {
            	server->Get(path.c_str(), chai.eval<httplib::Server::Handler>("Get"));
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
                server->Post(path.c_str(), chai.eval<httplib::Server::Handler>("Post"));
                hasPostMethod = true;
            }
            catch (chaiscript::exception::eval_error e)
            {
            }
            catch (chaiscript::exception::bad_boxed_cast e)
            {
            }
            if (hasGetMethod || hasPostMethod)
            {
                            	
                chaiScripts.push_back(chai_ptr);
                log(appFileInfo.fileName() + " loaded.");
                appSuccess++;
            }
        }
        catch (chaiscript::exception::eval_error e)
        {
            log("Application " + appFileInfo.fileName() + " load failed!");
            log(e.what());
            log(QString::asprintf("line: %d, col: %d.\n", e.start_position.line, e.start_position.column));
            
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

    using chaiscript::Module;
    using chaiscript::ModulePtr;
    using chaiscript::ChaiScript;
    using chaiscript::type_conversion;
    using chaiscript::fun;
    using chaiscript::constructor;
    using httplib::Request;
    using httplib::Response;

    auto& chaiModuleHttp = *chaiModuleHttpPtr;
    auto& chaiModuleString = *chaiModuleStringPtr;
	
    chaiModuleString
        .add(type_conversion<const char*, std::string>())
        .add(type_conversion<std::string, const char*>([](const std::string& s) {return s.c_str(); }));

    chaiscript::utility::add_class<Request>(chaiModuleHttp, "Request"
        , {
            constructor<Request()>()
            , constructor<Request(const Request&)>()
        }
        , {
            // variable
            {fun(&Request::method), "method"}
            , {fun(&Request::path), "path"}
            , {fun(&Request::body), "body"}
            , {fun(&Request::remote_addr), "remote_addr"}
            , {fun(&Request::remote_port), "remote_port"}
            , {fun(&Request::version), "version"}
            , {fun(&Request::target), "target"}
            , {fun(&Request::redirect_count), "redirect_count"}
            , {fun(&Request::content_length), "content_length"}
        // method
        , {fun(&Request::has_header), "has_header"}
        , {fun(static_cast<std::string(Request::*)(const char*, size_t) const>(&Request::get_header_value)), "get_header_value"}
        , {fun(&Request::get_header_value_count), "get_header_value_count"}
        , {fun(static_cast<void(Request::*)(const char*, const char*)>(&Request::set_header)), "set_header"}
        , {fun(&Request::get_param_value), "get_param_value"}
        , {fun(&Request::get_param_value_count), "get_param_value_count"}
        , {fun(&Request::is_multipart_form_data), "is_multipart_form_data"}
        , {fun(&Request::has_file), "has_file"}
        });

    chaiscript::utility::add_class<Response>(chaiModuleHttp, "Response"
        , {
            constructor<Response()>()
            , constructor<Response(const Response&)>()
            , constructor<Response(Response&&)>()
        },
        {
            // variable
            {fun(&Response::version), "version"}
            , {fun(&Response::status), "status"}
            , {fun(&Response::reason), "reason"}
            , {fun(&Response::body), "body"}
        // method
        , {fun(&Response::has_header), "has_header"}
        , {fun(static_cast<std::string(Response::*)(const char*, size_t)const>(&Response::get_header_value)), "get_header_value"}
        , {fun(&Response::get_header_value_count), "get_header_value_count"}
        , {fun(static_cast<void(Response::*)(const char*, const char*)>(&Response::set_header)), "set_header"}
        , {fun(static_cast<void(Response::*)(const char*, int)>(&Response::set_redirect)), "set_redirect"}
        , {fun(static_cast<void(Response::*)(std::string, const char*)>(&Response::set_content)), "set_content"}
        }
        );


	
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
            QDesktopServices::openUrl(QUrl("http://thatboy.info"));
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
    	
			server->bind_to_port(ui.ipLineEdit->text().toStdString().c_str()
				, ui.portLineEdit->text().toInt());
			server->remove_mount_point("/");
			server->set_base_dir(ui.pathLineEdit->text().toStdString().c_str());

            serverFuture = std::async(std::launch::async, [&]()
            {
	            const auto mark = server->listen_after_bind();
	            server.reset();
	            chaiScripts.clear();
	            emit serverStopped(mark);
            }).share();
    	
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

void SimpleHttpServer::closeEvent(QCloseEvent*e)
{
    if (server)
        server->stop();
    if(serverFuture.valid())
        serverFuture.wait();
    server.reset();
    chaiScripts.clear();
    QWidget::closeEvent(e);
}
