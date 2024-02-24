#include <QDirIterator>
#include <QJsonObject>
#include <QApplication>
#include "WebService.h"

#include "Context.h"
#include "Common.h"
#include "PlayManager.h"
#include "PlayerController.h"
#include "SBHtmlSongsAll.h"

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
        return QHttpServerResponse::fromFile(":/www/redirect.html");
    });

    _httpServer.route("/images", [] (const QHttpServerRequest &request)
    {
        qDebug() << SB_DEBUG_INFO;
        return host(request) + u"/images/"_s;
    });

    _httpServer.route("/player/", WebService::_controlPlayer);
    // _httpServer.route("/player/", [] (QString path, const QHttpServerRequest &request)
    // {
    //     const static QString parameter("action");
    //     qDebug() << SB_DEBUG_INFO << "player=" << path;
    //     qDebug() << SB_DEBUG_INFO << "query=" << request.query().query();
    //     const QString action=request.query().queryItemValue(parameter);
    //     qDebug() << SB_DEBUG_INFO << "action=" << action;
    //     return u"%1/player/%2"_s.arg(host(request)).arg(path);
    //     return u"%1/player/%2"_s.arg(host(request)).arg(path);
    // });

    _httpServer.route("/icon", WebService::_getIconResource);
    _httpServer.route("/icon/", WebService::_getIconResource);
    //_httpServer.route("/image/", [] (QString path, const QHttpServerRequest &request)

    _httpServer.route("/images/", WebService::_getImageResource);
    // {
    //     qDebug() << SB_DEBUG_INFO << "image=" << path;
    //     return u"%1/image/%2"_s.arg(host(request)).arg(path);
    // });

    //_httpServer.route("/<arg>", [] (QString id, const QHttpServerRequest &request)
    _httpServer.route("/<arg>", WebService::_getHTMLResource);
    // {
    //     Q_UNUSED(request);
    //     qDebug() << SB_DEBUG_INFO;
    //     return u"%1/image/%2"_s.arg(id);
    // });

    // _httpServer.route("/image/<arg>/", [] (QString id, QString threshold, const QHttpServerRequest &request)
    // {
    //     qDebug() << SB_DEBUG_INFO;
    //     return u"%1/image/%2/%3"_s.arg(host(request)).arg(id).arg(threshold);
    // });


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
    if (!port)
    {
        qWarning() << QApplication::translate("QHttpServerExample","Server failed to listen on a port.");
        return;	//	CWIP: something else but -1;
    }

    qDebug() << SB_DEBUG_INFO << QApplication::translate("QHttpServerExample","Running on http://127.0.0.1:%1/ (Press CTRL+C to quit)").arg(port);


    QDirIterator it(":", QDirIterator::Subdirectories);
    static const QString images("/images");
    static const QString www("/www");

    while (it.hasNext())
    {
        QString c=it.next();
        QString path=c.mid(1,c.length());
        QFileInfo fi(path);
        if(fi.path()==images || fi.path()==www)
        {
            _availableResource[fi.fileName()]=c;
        }
    }
    if(1)
    {
        for (auto i = _availableResource.cbegin(), end = _availableResource.cend(); i != end; ++i)
        {
            qDebug() << SB_DEBUG_INFO << i.key() << " -> " << i.value();
        }
    }
}

QHttpServerResponse
WebService::_controlPlayer(QString unused, const QHttpServerRequest& r)
{
    Q_UNUSED(unused);
    const static QString p_action("action");
    const static QString k_action("key");
    const QString action=r.query().queryItemValue(p_action);
    const QString key=r.query().queryItemValue(k_action);
    SBKey sbKey(key.toLatin1());
    if(!sbKey.validFlag())
    {
        sbKey=SBKey();
    }
    qDebug() << SB_DEBUG_INFO << "action=" << action;
    qDebug() << SB_DEBUG_INFO << "sbKey=" << sbKey;
    const static QString prev("prev");
    const static QString stop("stop");
    const static QString play("play");
    const static QString next("next");
    PlayManager* pm=Context::instance()->playManager();
    if(action==prev)
    {
        pm->playerPrevious();
    }
    else if(action==stop)
    {
        pm->playerStop();
    }
    else if(action==play)
    {
        if(sbKey.validFlag())
        {
            pm->playItemNow(sbKey);
        }
        else
        {
            pm->playerPlay();
        }
    }
    else if(action==next)
    {
        pm->playerNext();
    }
    else
    {
        qDebug() << SB_DEBUG_ERROR << "Unknown action:" << action;
    }

    return QHttpServerResponse("At Your Service!");
}

QHttpServerResponse
WebService::_fourOhFour()
{
    qDebug() << SB_DEBUG_INFO;
    return QHttpServerResponse::fromFile(":/www/404.html");
}

QHttpServerResponse
WebService::_getHTMLResource(QString path, const QHttpServerRequest& r)
{
    const static QString status("status.html");
    if(path!=status)
    {
        qDebug() << SB_DEBUG_INFO << path;
    }
    return WebService::_getResource(path,r);
}

QHttpServerResponse
WebService::_getIconResource(QString path, const QHttpServerRequest& r)
{
    const static QString defaultIconPath("/images/SongIcon.png");
    QString iconPath;
    qDebug() << SB_DEBUG_INFO << "****** " << path;
    const SBKey key=SBKey(path.toUtf8());
    if(key.validFlag())
    {
        iconPath=ExternalData::getCachePath(key);
        if(!QFile::exists(iconPath))
        {
            iconPath=defaultIconPath;
        }
    }
    else
    {
        iconPath=defaultIconPath;
    }
    qDebug() << SB_DEBUG_INFO << "****** " << iconPath;
    return QHttpServerResponse::fromFile(iconPath);
}

QHttpServerResponse
WebService::_getImageResource(QString path, const QHttpServerRequest& r)
{
    qDebug() << SB_DEBUG_INFO << path;
    return WebService::_getResource(path,r,1);
}

QHttpServerResponse
WebService::_getResource(QString path, const QHttpServerRequest& r, bool isImage)
{
    Q_UNUSED(r);
    const QFileInfo fi_p=QFileInfo(path);
    QString resourcePath=_availableResource[fi_p.fileName()];

    if(resourcePath.size()==0)
    {
        qDebug() << SB_DEBUG_ERROR << "Resource does not exist" << path;
        return WebService::_fourOhFour();
    }
    const static QString status(":/www/status.html");
    if(resourcePath!=status)
    {
        qDebug() << SB_DEBUG_INFO << resourcePath;
    }

    if(isImage)
    {
        return QHttpServerResponse::fromFile(resourcePath);
    }
    return QHttpServerResponse(WebService::_populateData(resourcePath, path, r));
}

QString
WebService::_populateData(const QString& resourcePath, const QString& path, const QHttpServerRequest& r)
{
    QFile f(resourcePath);
    f.open(QFile::ReadOnly | QFile::Text);	//	ignore results. Always works, right?
    QTextStream f_str(&f);
    QString str=f_str.readAll();

    QString allSong("song_list.html");
    QString status("status.html");

    if(path==status)
    {
        PlayerController* pc=Context::instance()->playerController();

        QString playerStatus;
        QString playerSongTitle;

        const QMediaPlayer::PlaybackState playState=pc->playState();
        switch(playState)
        {
            case QMediaPlayer::StoppedState:
                playerStatus="Stopped";
                break;

            case QMediaPlayer::PlayingState:
                playerStatus="Now Playing";
                break;

            case QMediaPlayer::PausedState:
                playerStatus="Paused:";
                break;
        }

        //	Populate song specific
        SBIDOnlinePerformancePtr opPtr=pc->currentPerformancePlaying();
        if(opPtr)
        {
            playerSongTitle=opPtr->songTitle();
            playerStatus+=QString(": %1 by %2 from the '%3' album").arg(opPtr->songTitle()).arg(opPtr->songPerformerName()).arg(opPtr->albumTitle());
        }
        else
        {
            playerStatus+=QString('.');
        }

        const static QString SB_PLAYER_STATUS("___SB_PLAYER_STATUS___");
        str=str.replace(SB_PLAYER_STATUS,playerStatus);
    }
    else if(path==allSong)
    {
        const static QString p_start("start");
        QString startStr=r.query().queryItemValue(p_start);
        QChar start(startStr.size()>0?startStr[0]:'A');

        QString allSongs;

        const static QString SB_SONG_TABLE("___SB_SONG_TABLE___");
        str=str.replace(SB_SONG_TABLE,SBHtmlSongsAll::retrieveAllSongs(start));
    }
    else
    {
        //	Somehow do a 404 here
    }

    return str;
}
