#include <QDirIterator>
#include <QJsonObject>
#include <QApplication>

#include <QHostAddress>
#include <QNetworkAddressEntry>
#include <QNetworkInterface>

#include "WebService.h"


#include "Context.h"
#include "Common.h"
#include "PlayManager.h"
#include "PlayerController.h"
#include "SBIDAlbum.h"
#include "SBIDChart.h"
#include "SBIDPerformer.h"
#include "SBIDPlaylist.h"
#include "SBIDSong.h"
#include "WebTemplate.h"

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

    _httpServer.route("/icon/", WebService::_getIconResource);

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
    if(0)
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
    qDebug() << SB_DEBUG_WARNING;
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
    Q_UNUSED(r);
    QString iconPath;
    const SBKey key=SBKey(path.toUtf8());
    if(key.validFlag())
    {
        iconPath=ExternalData::getCachePath(key);
        if(!QFile::exists(iconPath))
        {
            iconPath=ExternalData::getDefaultIconPath(SBKey::Song);
        }
    }
    else
    {
        iconPath=ExternalData::getDefaultIconPath(SBKey::Song);
    }
    return QHttpServerResponse::fromFile(iconPath);
}

QHttpServerResponse
WebService::_getImageResource(QString path, const QHttpServerRequest& r)
{
    //  qDebug() << SB_DEBUG_INFO << path;
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
        //  qDebug() << SB_DEBUG_INFO << resourcePath;
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
    bool displayPlayButtons=_displayPlayButtons(r);

    const static QString p_key("sb_key");
    if(path!="status.html")
    {
        qDebug() << SB_DEBUG_INFO << resourcePath;
        qDebug() << SB_DEBUG_INFO << path;
    }

    QFile f(resourcePath);
    f.open(QFile::ReadOnly | QFile::Text);	//	ignore results. Always works, right?
    QTextStream f_str(&f);
    QString str=f_str.readAll();

    const static QString allAlbum("album_list.html");
    const static QString allChart("chart_list.html");
    const static QString allPlaylist("playlist_list.html");
    const static QString allPerformers("performer_list.html");
    const static QString allSong("song_list.html");
    const static QString albumDetail("album_detail.html");
    const static QString chartDetail("chart_detail.html");
    const static QString playlistDetail("playlist_detail.html");
    const static QString performerDetail("performer_detail.html");
    const static QString songDetail("song_detail.html");
    const static QString status("status.html");

    if(path==status)    //  CWIP: rename to playerstatus
    {
        //  Player status
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
    else if(path==allAlbum)
    {
        const static QString p_letter("letter");
        const static QString p_offset("offset");
        const static QString p_size("size");
        QString letterStr=r.query().queryItemValue(p_letter);
        QChar letter(letterStr.size()>0?letterStr[0]:'A');
        QString offsetStr=r.query().queryItemValue(p_offset);
        QString sizeStr=r.query().queryItemValue(p_size);

        const static QString SB_ALBUM_TABLE("___SB_ALBUM_TABLE___");
        str=str.replace(SB_ALBUM_TABLE,WebTemplate<SBIDAlbum>::generateList(letter,offsetStr.toInt(),sizeStr.toInt()));

    }
    else if(path==albumDetail)
    {
        const QString key=r.query().queryItemValue(p_key);
        str=WebTemplate<SBIDAlbum>::retrieveDetail(str,key);
    }
    else if(path==allChart)
    {
        //  const static QString p_letter("letter");
        //  const static QString p_offset("offset");
        //  const static QString p_size("size");
        //  QString letterStr=r.query().queryItemValue(p_letter);

        //  For now, no paging.
        QChar letter(QChar('\x0'));                 //  letterStr.size()>0?letterStr[0]:'A';
        QString offsetStr=0;                        //  r.query().queryItemValue(p_offset);
        QString sizeStr=QString("1000000");         //  r.query().queryItemValue(p_size);

        const static QString SB_CHART_TABLE("___SB_CHART_TABLE___");
        str=str.replace(SB_CHART_TABLE,WebTemplate<SBIDChart>::generateList(letter,offsetStr.toInt(),sizeStr.toInt()));
    }
    else if(path==allPlaylist)
    {
        //  const static QString p_letter("letter");
        //  const static QString p_offset("offset");
        //  const static QString p_size("size");
        //  QString letterStr=r.query().queryItemValue(p_letter);

        //  For now, no paging.
        QChar letter(QChar('\x0'));                 //  letterStr.size()>0?letterStr[0]:'A';
        QString offsetStr=0;                        //  r.query().queryItemValue(p_offset);
        QString sizeStr=QString("1000000");         //  r.query().queryItemValue(p_size);

        const static QString SB_PLAYLIST_TABLE("___SB_PLAYLIST_TABLE___");
        str=str.replace(SB_PLAYLIST_TABLE,WebTemplate<SBIDPlaylist>::generateList(letter,offsetStr.toInt(),sizeStr.toInt()));
    }
    else if(path==allPerformers)
    {
        const static QString p_letter("letter");
        const static QString p_offset("offset");
        const static QString p_size("size");
        QString letterStr=r.query().queryItemValue(p_letter);
        QChar letter(letterStr.size()>0?letterStr[0]:'A');
        QString offsetStr=r.query().queryItemValue(p_offset);
        QString sizeStr=r.query().queryItemValue(p_size);

        const static QString SB_PERFORMER_TABLE("___SB_PERFORMER_TABLE___");
        //str=str.replace(SB_PERFORMER_TABLE,SBHtmlPerformersAll::retrieveAllPerformers(letter,offsetStr.toInt(),sizeStr.toInt()));
        str=str.replace(SB_PERFORMER_TABLE,WebTemplate<SBIDPerformer>::generateList(letter,offsetStr.toInt(),sizeStr.toInt()));
    }
    else if(path==allSong)
    {
        const static QString p_letter("letter");
        const static QString p_offset("offset");
        const static QString p_size("size");
        QString letterStr=r.query().queryItemValue(p_letter);
        QChar letter(letterStr.size()>0?letterStr[0]:'A');
        QString offsetStr=r.query().queryItemValue(p_offset);
        QString sizeStr=r.query().queryItemValue(p_size);

        const static QString SB_SONG_TABLE("___SB_SONG_TABLE___");
        str=str.replace(SB_SONG_TABLE,WebTemplate<SBIDSong>::generateList(letter,offsetStr.toInt(),sizeStr.toInt()));
    }
    else if(path==chartDetail)
    {
        const QString key=r.query().queryItemValue(p_key);
        str=WebTemplate<SBIDChart>::retrieveDetail(str,key);
    }
    else if(path==performerDetail)
    {
        const QString key=r.query().queryItemValue(p_key);
        str=WebTemplate<SBIDPerformer>::retrieveDetail(str,key);
    }
    else if(path==playlistDetail)
    {
        const QString key=r.query().queryItemValue(p_key);
        str=WebTemplate<SBIDPlaylist>::retrieveDetail(str,key);
    }
    else if(path==songDetail)
    {
        const QString key=r.query().queryItemValue(p_key);
        str=WebTemplate<SBIDSong>::retrieveDetail(str,key);
    }
    else
    {
        //	Somehow do a 404 here
    }
    qDebug() << SB_DEBUG_INFO << displayPlayButtons;
    return str;
}

bool
WebService::_displayPlayButtons(const QHttpServerRequest& r)
{
    QList<QNetworkInterface> allNI=QNetworkInterface::allInterfaces();
    qDebug() << SB_DEBUG_INFO << "local" << r.localAddress().toString();
    qDebug() << SB_DEBUG_INFO << "remote" << r.remoteAddress().toString() << r.remoteAddress().isLinkLocal() << r.remoteAddress().isLoopback();

    for(const auto& ni: allNI)
    {
        qDebug() << SB_DEBUG_INFO << ni.name();
        QList<QNetworkAddressEntry> allNAE=ni.addressEntries();
        for(const auto& nae: allNAE)
        {
            QHostAddress ha=nae.netmask();
            qDebug() << SB_DEBUG_INFO << "\t" << nae.ip() << ha.toString();
        }
    }
}
