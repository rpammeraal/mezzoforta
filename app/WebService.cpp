#include <QDirIterator>
#include <QJsonObject>
#include <QApplication>
#include "WebService.h"

#include "Common.h"

using namespace Qt::StringLiterals;

static inline QString
host(const QHttpServerRequest &request)
{
    return QString::fromLatin1(request.value("Host"));
}

WebService::WebService()
{
    _init();
}

WebService::~WebService()
{
}

void
WebService::_init()
{
    _httpServer.route("/", []()
    {
        return "Hello world";
    });

    _httpServer.route("/image", [] (const QHttpServerRequest &request)
    {
        qDebug() << SB_DEBUG_INFO;
        return host(request) + u"/image/"_s;
    });

    //_httpServer.route("/image/", [] (QString path, const QHttpServerRequest &request)
    _httpServer.route("/image/", WebService::_handleImage);
    // {
    //     qDebug() << SB_DEBUG_INFO << "image=" << path;
    //     return u"%1/image/%2"_s.arg(host(request)).arg(path);
    // });

    _httpServer.route("/image/<arg>", [] (QString id, const QHttpServerRequest &request)
    {
        qDebug() << SB_DEBUG_INFO;
        return u"%1/image/%2"_s.arg(host(request)).arg(id);
    });

    _httpServer.route("/image/<arg>/", [] (QString id, QString threshold, const QHttpServerRequest &request)
    {
        qDebug() << SB_DEBUG_INFO;
        return u"%1/image/%2/%3"_s.arg(host(request)).arg(id).arg(threshold);
    });


    // _httpServer.route("/user/", [] (const qint32 id) {
    //     return u"User "_s + QString::number(id);
    // });

    // _httpServer.route("/user/<arg>/detail", [] (const qint32 id) {
    //     return u"User %1 detail"_s.arg(id);
    // });

    // _httpServer.route("/user/<arg>/detail/", [] (const qint32 id, const qint32 year) {
    //     return u"User %1 detail year - %2"_s.arg(id).arg(year);
    // });

    _httpServer.route("/json/", [] {
        return QJsonObject{
            {
                {"key1", "1"},
                {"key2", "2"},
                {"key3", "3"}
            }
        };
    });

    // _httpServer.route("/resources/<arg>", [] (const QUrl &url, const QHttpServerRequest& r)
    // {
    //     qDebug() << SB_DEBUG_INFO << r;
    //     QString completePath=QString(":/resources/%1");//.arg(url.path);
    //     return QHttpServerResponse::fromFile(u":/resources/"_s + url.path());
    // });

    // _httpServer.route("/remote_address", [](const QHttpServerRequest &request) {
    //     return request.remoteAddress().toString();
    // });

    // // Basic authentication example (RFC 7617)
    // _httpServer.route("/auth", [](const QHttpServerRequest &request) {
    //     auto auth = request.value("authorization").simplified();

    //     if (auth.size() > 6 && auth.first(6).toLower() == "basic ") {
    //         auto token = auth.sliced(6);
    //         auto userPass = QByteArray::fromBase64(token);

    //         if (auto colon = userPass.indexOf(':'); colon > 0) {
    //             auto userId = userPass.first(colon);
    //             auto password = userPass.sliced(colon + 1);

    //             if (userId == "Aladdin" && password == "open sesame")
    //                 return QHttpServerResponse("text/plain", "Success\n");
    //         }
    //     }
    //     QHttpServerResponse response("text/plain", "Authentication required\n",
    //                                  QHttpServerResponse::StatusCode::Unauthorized);
    //     response.setHeader("WWW-Authenticate", R"(Basic realm="Simple example", charset="UTF-8")");
    //     return response;
    // });

    _httpServer.afterRequest([](QHttpServerResponse &&resp) {
        resp.setHeader("Server", "Qt HTTP Server");
        return std::move(resp);
    });

    const auto port = _httpServer.listen(QHostAddress::Any,80);
    if (!port) {
        qWarning() << QApplication::translate("QHttpServerExample",
                                                  "Server failed to listen on a port.");
        return;	//	CWIP: something else but -1;
    }

    qDebug() << SB_DEBUG_INFO << QApplication::translate("QHttpServerExample","Running on http://127.0.0.1:%1/ (Press CTRL+C to quit)").arg(port);


    QDirIterator it(":", QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        QString c=it.next();
        QString path=c.mid(1,c.length());
        QFileInfo fi=QFileInfo(path);
        if(fi.path()==QString("/images"))
        {
            if(_isImage(path))
            {
                _availableImages[fi.fileName()]=c;
            }
        }
    }
    for (auto i = _availableImages.cbegin(), end = _availableImages.cend(); i != end; ++i)
    {
        qDebug() << SB_DEBUG_INFO << i.key() << " -> " << i.value();
    }
}

bool
WebService::_isImage(const QString& path) const
{
    static const QString ico=QString("ico");
    static const QString png=QString("png");

    QFileInfo fi=QFileInfo(path);
    const QString suffix=fi.suffix();

    return(suffix==ico || suffix==png)?1:0;
}

QHttpServerResponse
WebService::_handleImage(QString path, const QHttpServerRequest& r)
{
    qDebug() << SB_DEBUG_INFO;
    const QFileInfo fi_p=QFileInfo(path);
    const QString resourcePath=_availableImages[fi_p.fileName()];

    if(resourcePath.size()==0)
    {
        qDebug() << SB_DEBUG_ERROR << "Resource does not exist" << path;
        //	this->_fourOhFour(s);
        //	return;
        //	CWIP: not handled
    }
    qDebug() << SB_DEBUG_INFO << resourcePath;
    static const QString png=QString("png");
    static const QString ico=QString("ico");	//	Huge assumption that these files are PNG files.

    const QFileInfo fi_rp=QFileInfo(resourcePath);
    QString suffix=fi_rp.suffix();
    if(suffix==ico)
    {
        suffix=png;
    }

    if(suffix!=png)
    {
        qDebug() << SB_DEBUG_ERROR << "Suffix " << suffix << " not handled.";
        //	this->_fourOhFour(s);
        //return;
        //	CWIP: not handles
    }
    else
    {
        qDebug() << SB_DEBUG_INFO;
        return QHttpServerResponse::fromFile(resourcePath);
    }

    return u"%1/image/%2"_s.arg(host(r)).arg(path);
}
