#include "WebService.h"
#include "QHttpServer"
#include "QHttpServerResponse"

#include "Context.h"

WebService::WebService()
{
    qDebug() << SB_DEBUG_INFO;
    this->_init();
    qDebug() << SB_DEBUG_INFO;
}

WebService::~WebService()
{

}

QString
WebService::root()
{
    QFile f(":/www/index.html");
    qDebug() << SB_DEBUG_INFO << f.fileName();
    if (!f.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug() << SB_DEBUG_ERROR << "Cannot open file";
    }
    QTextStream in(&f);
    QString qs=in.readAll();
    return qs;
}

void
WebService::_init()
{
    qDebug() << SB_DEBUG_INFO << "Setting up web server";

    QHttpServer httpServer;
    httpServer.route("/", []()
    {
        //return QHttpServerResponse::fromFile(QString(":/www/index.html"));
        return WebService::root();
    });

    const auto port = httpServer.listen(QHostAddress::Any,80);
    if(!port)
    {
       qDebug() << SB_DEBUG_ERROR << QCoreApplication::translate("QHttpServerExample", "Server failed to listen on a port.");
    }
    qDebug() << SB_DEBUG_INFO << "Listening on port" << port;

    qDebug() << SB_DEBUG_INFO << "Setting up web server done.";
}
