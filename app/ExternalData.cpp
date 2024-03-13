#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QLabel>
#include <QSemaphore>
#include <QStandardPaths>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QPixmap>
#include <QProcess>
#include <QWebEngineView>
#include <QXmlStreamReader>

#include "BackgroundThread.h"
#include "Common.h"
#include "CacheManager.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "ExternalData.h"
#include "SBIDAlbum.h"
#include "SBIDPerformer.h"
#include "SBIDSong.h"

#include <QMessageBox>

ExternalData::ExternalData(QObject *parent) : QObject(parent)
{
    _init();
}

ExternalData::~ExternalData()
{
}

//	Main interface
void
ExternalData::loadAlbumData(SBKey key)
{
    _softInit();
    _currentKey=key;

    //	1.	Artwork
    QPixmap p;
    if(_loadImageFromCache(p,_currentKey))
    {
        _artworkRetrievedFlag=1;
        emit imageDataReady(p, _currentKey);
    }

    //	2.	Wikipedia page
    SBIDPtr ptr=CacheManager::get(key);
    SB_RETURN_VOID_IF_NULL(ptr);

    if(ptr->wiki().length()>0)
    {
        _wikipediaURLRetrievedFlag=1;
        emit albumWikipediaPageAvailable(ptr->wiki());
    }
    _getMBIDAndMore();
}

void
ExternalData::loadPerformerData(SBKey key)
{
    _softInit();
    _currentKey=key;
    SBIDPtr ptr=CacheManager::get(key);
    SB_RETURN_VOID_IF_NULL(ptr);

    //	Always refresh performer home page.

    //	1.	Wikipedia page
    if(ptr->wiki().length()>0)
    {
        _wikipediaURLRetrievedFlag=1;
        emit performerWikipediaPageAvailable(ptr->wiki());
    }

    //	2.	Artwork
    QPixmap p;
    if(_loadImageFromCache(p,key))
    {
        _artworkRetrievedFlag=1;
        emit imageDataReady(p,key);
    }
    _getMBIDAndMore();
}

void
ExternalData::loadSongData(SBKey key)
{
    _softInit();
    _currentKey=key;

    SBIDPtr ptr=CacheManager::get(key);
    SB_RETURN_VOID_IF_NULL(ptr);

    //	1.	Wikipedia page
    if(ptr->wiki().length()>0)
    {
        _wikipediaURLRetrievedFlag=1;
        emit songWikipediaPageAvailable(ptr->wiki());
    }
    _getMBIDAndMore();
}

//	Static methods
QString
ExternalData::getCachePath(SBKey key)
{
    PropertiesPtr properties=Context::instance()->properties();
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QString prefix;

    prefix=properties->currentDatabaseSchema();
    if(!prefix.length())
    {
        prefix=dal->databaseName();
    }
    QString p=QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    p+="/Thumbnails";
    QDir d;
    d.mkpath(p);

    QString f=key.validFlag()?QString("%1/%2.%3.%4")
        .arg(p)
        .arg(prefix)
        .arg(key.itemType())
        .arg(key.itemID()):QString();
    ;
    return f.toLower();
}

QString
ExternalData::getDefaultIconPath(const SBKey::ItemType& itemType)
{
    switch(itemType)
    {
    case SBKey::Song:
    case SBKey::SongPerformance:
    case SBKey::OnlinePerformance:
        return QString("/images/SongIcon.png");

    case SBKey::Performer:
        return QString("/images/NoBandPhoto.png");

    case SBKey::Album:
    case SBKey::AlbumPerformance:
        return QString("/images/NoAlbumCover.png");

    case SBKey::Chart:
    case SBKey::ChartPerformance:
        return QString("/images/ChartIcon.png");

    case SBKey::Playlist:
    case SBKey::PlaylistDetail:
        return QString("/images/PlaylistIcon.png");

    default:
        qDebug() << SB_DEBUG_ERROR << "itemType" << itemType << "not handled.";
        ;
    }
    return _defaultIconPath;
}

//  PUBLIC SLOTS
void
ExternalData::processAlbum(QNetworkReply *r)
{
    QString wikiDataURL;
    //  Look for wikidata and reviews
    if(r->error())
    {
        qDebug() << SB_DEBUG_ERROR << r->errorString();
        qDebug() << SB_DEBUG_ERROR << r->error();
    }
    else
    {
        //  Retrieve album
        const SBIDAlbumPtr aPtr=SBIDAlbum::retrieveAlbum(_currentKey);
        SB_RETURN_VOID_IF_NULL(aPtr);

        //  Go through JSON
        QByteArray a=r->readAll();
        QJsonParseError jpe;
        QJsonDocument jd=QJsonDocument::fromJson(a,&jpe);
        if(jpe.error!=QJsonParseError::NoError)
        {
            qDebug() << SB_DEBUG_ERROR << jpe.errorString();
            return;
        }

        QJsonArray ja=jd.object().value("release-groups").toArray();
        for(qsizetype i=0;i<ja.size();i++)
        {
            QJsonObject jo=ja[i].toObject();
            if(jo.value("title")==aPtr->albumTitle())
            {
                QJsonArray ja=jo.value("relations").toArray();
                for(qsizetype j=0;j<ja.size();j++)
                {
                    QJsonObject jo=ja[j].toObject();
                    if(jo.value("type")=="wikidata" && !wikiDataURL.size())
                    {
                        wikiDataURL=jo.value("url").toObject().value("resource").toString();
                    }
                    if(jo.value("type")=="review")
                    {
                        _allReviews << jo.value("url").toObject().value("resource").toString();
                    }
                }
            }
        }
    }
    _postNetwork(r);      //  We're done with networking.

    if(_allReviews.count()>0)
    {
        emit albumReviewsAvailable(_allReviews);
    }

    if(wikiDataURL.size())
    {
        _wikiDataQValue=wikiDataURL.mid(wikiDataURL.indexOf("wiki/Q")+5);

        //  Continue on with retrieving the wikiData for the album and get the wikipedia page
        QNetworkAccessManager* am = new QNetworkAccessManager(this);

        connect(am, &QNetworkAccessManager::finished,
                this, &ExternalData::processWikidata);

        const QString url=QString("https://www.wikidata.org/w/api.php?action=wbgetentities&format=json&ids=%1&props=claims%7Clabels%7Cdescriptions%7Csitelinks%2Furls&languages=en&languagefallback=1&normalize=1&sitefilter=enwiki&formatversion=2")
                                .arg(_wikiDataQValue);
        _sendNetworkRequest(am, url);
    }
}

void
ExternalData::processAlbumImage(QNetworkReply *r)
{
    if(r->error())
    {
        qDebug() << SB_DEBUG_ERROR << r->errorString();
        qDebug() << SB_DEBUG_ERROR << r->error();
        qDebug() << SB_DEBUG_ERROR << "Continue with next";
        emit(handleNextAlbumImageLocation());
    }
    else
    {
        QByteArray a=r->readAll();

        QJsonParseError jpe;
        QJsonDocument jd=QJsonDocument::fromJson(a,&jpe);
        if(jpe.error!=QJsonParseError::NoError)
        {
            qDebug() << SB_DEBUG_ERROR << jpe.errorString();
            return;
        }

        QString searchImage=QString("images/image");
        const QStringList sl=_inspectJsonDoc(jd,searchImage,0);
        if(sl.size())
        {
            emit(startRetrieveImageData(sl,_currentKey));
        }
    }
    _postNetwork(r);      //  We're done with networking.
}

void
ExternalData::processAlbumsByPerformer (QNetworkReply *r)
{
    if(r->error())
    {
        qDebug() << SB_DEBUG_ERROR << r->errorString();
        qDebug() << SB_DEBUG_ERROR << r->error();
    }
    else
    {
        QByteArray a=r->readAll();
        QJsonParseError jpe;
        QJsonDocument jd=QJsonDocument::fromJson(a,&jpe);
        if(jpe.error!=QJsonParseError::NoError)
        {
            qDebug() << SB_DEBUG_ERROR << jpe.errorString();
            return;
        }

        //  Retrieve album
        const SBIDAlbumPtr aPtr=SBIDAlbum::retrieveAlbum(_currentKey);
        SB_RETURN_VOID_IF_NULL(aPtr);

        //  Search for album images
        QString searchAlbumMBID=QString("releases/title=%1?id").arg(aPtr->albumTitle());   //  In each item in `release`,
                                                                                            //  look for matching `title`
                                                                                            //  and retrieve values for `id`.
        QStringList sl=_inspectJsonDoc(jd,searchAlbumMBID,0);

        if(sl.size())
        {
            //  This will cause more networking, we can't do wikipedia retrieval yet.
            //  Yes-- this can be all done way more elegantly.
            emit(startAlbumImageProcess(sl,_currentKey));
        }
    }
    _postNetwork(r);      //  We're done with networking.
}

void
ExternalData::processImageRetrieved(const QString& path, const SBKey& key)
{
    QPixmap image(path);
    if(!image.isNull())
    {
        _artworkRetrievedFlag=1;
        emit imageDataReady(image, key);
    }
    else
    {
        qDebug() << SB_DEBUG_WARNING << "No image data at " << path;
    }
}

void
ExternalData::processMBID(QNetworkReply *r)
{
    SBIDPtr ptr=CacheManager::get(_currentKey);
    SB_RETURN_VOID_IF_NULL(ptr);
    bool found=0;

    if(r->error())
    {
        qDebug() << SB_DEBUG_ERROR << r->errorString();
        qDebug() << SB_DEBUG_ERROR << r->error();
    }
    else
    {
        QByteArray a=r->readAll();
        QJsonParseError jpe;
        QJsonDocument jd=QJsonDocument::fromJson(a,&jpe);
        if(jpe.error!=QJsonParseError::NoError)
        {
            qDebug() << SB_DEBUG_ERROR << jpe.errorString();
            return;
        }

        const QString mbidPath("artists/score=100?id");
        QStringList sl=_inspectJsonDoc(jd,mbidPath,0);

        if(sl.size())
        {
            const QString mbid=sl[0];

            emit updatePerformerMBID(_currentKey,mbid);
            found=1;
        }
    }
    _postNetwork(r);      //  We're done with networking.

    if(found)
    {
        _getMBIDAndMore();
    }
}

void
ExternalData::processPerformer(QNetworkReply *r)
{
    QString wikiDataURL;
    QString officialPerformerHomePageURL;
    if(r->error())
    {
        qDebug() << SB_DEBUG_ERROR << r->errorString();
        qDebug() << SB_DEBUG_ERROR << r->error();
    }
    else
    {
        QByteArray a=r->readAll();
        QJsonParseError jpe;
        QJsonDocument jd=QJsonDocument::fromJson(a,&jpe);
        if(jpe.error!=QJsonParseError::NoError)
        {
            qDebug() << SB_DEBUG_ERROR << jpe.errorString();
            return;
        }

        QJsonArray ja=jd.object().value("relations").toArray();
        for(qsizetype i=0;i<ja.size();i++)
        {
            QJsonObject jo=ja[i].toObject();
            if(jo.value("type")=="wikidata" && !wikiDataURL.size())
            {
                wikiDataURL=jo.value("url").toObject().value("resource").toString();
            }
            if(jo.value("type")=="official homepage")
            {
                officialPerformerHomePageURL=jo.value("url").toObject().value("resource").toString();
            }
            if(jo.value("type")=="fanpage" && !officialPerformerHomePageURL.size())
            {
                //  Fallback on fanpage if no official home page is available
                officialPerformerHomePageURL=jo.value("url").toObject().value("resource").toString();
            }
            //  CWIP: lyrics URL to be found in this data set.
        }
    }
    _postNetwork(r);      //  We're done with networking.

    if(officialPerformerHomePageURL.size())
    {
        SBIDPerformerPtr pPtr=SBIDPerformer::retrievePerformer(_currentKey);
        SB_RETURN_VOID_IF_NULL(pPtr);

        pPtr->setURL(officialPerformerHomePageURL);
        _performerHomepageRetrievedFlag=1;
        qDebug() << SB_DEBUG_INFO << "\t\tofficial home page:" << officialPerformerHomePageURL;
        emit performerHomePageAvailable(pPtr->url());
        emit updatePerformerHomePage(_currentKey);
    }

    if(wikiDataURL.size())
    {
        _wikiDataQValue=wikiDataURL.mid(wikiDataURL.indexOf("wiki/Q")+5);

        //  Continue on with retrieving the wikiData for the album and get the wikipedia page
        QNetworkAccessManager* am = new QNetworkAccessManager(this);

        connect(am, &QNetworkAccessManager::finished,
                this, &ExternalData::processWikidata);

        const QString url=QString("https://www.wikidata.org/w/api.php?action=wbgetentities&format=json&ids=%1&props=claims%7Clabels%7Cdescriptions%7Csitelinks%2Furls&languages=en&languagefallback=1&normalize=1&sitefilter=enwiki&formatversion=2")
                                .arg(_wikiDataQValue);
        _sendNetworkRequest(am, url);
    }
    return;
}

void
ExternalData::processSingleAlbumImageLocations(const QString& mbid, const SBKey& key)
{
    Q_UNUSED(key);
    QNetworkAccessManager* am = new QNetworkAccessManager(this);

    connect(am, &QNetworkAccessManager::finished,
            this, &ExternalData::processAlbumImage);

    QUrl url=QUrl(QString("https://coverartarchive.org/release/%1").arg(mbid));
    _sendNetworkRequest(am, url);
}

void
ExternalData::processSong(QNetworkReply *r)
{
    //  CWIP include pagination
    QString wikiDataURL;

    if(r->error())
    {
        qDebug() << SB_DEBUG_ERROR << r->errorString();
        qDebug() << SB_DEBUG_ERROR << r->error();
    }
    else
    {
        //  Retrieve song
        const SBIDSongPtr sPtr=SBIDSong::retrieveSong(_currentKey);
        SB_RETURN_VOID_IF_NULL(sPtr);
        const QString songTitle=_normalizeString(sPtr->songTitle());

        QByteArray a=r->readAll();
        QJsonParseError jpe;
        QJsonDocument jd=QJsonDocument::fromJson(a,&jpe);
        if(jpe.error!=QJsonParseError::NoError)
        {
            qDebug() << SB_DEBUG_ERROR << jpe.errorString();
            return;
        }

        static const QString song("Song");
        QJsonArray ja=jd.object().value("works").toArray();
        for(qsizetype i=0;i<ja.size();i++)
        {
            QJsonObject jo=ja[i].toObject();
            if(jo.value("type")==song && _normalizeString(jo.value("title").toString())==songTitle)
            {
                QJsonArray ja=jo.value("relations").toArray();
                for(qsizetype j=0;j<ja.size();j++)
                {
                    QJsonObject jo=ja[j].toObject();
                    if(jo.value("type")=="wikidata" && !wikiDataURL.size())
                    {
                        wikiDataURL=jo.value("url").toObject().value("resource").toString();
                        qDebug() << SB_DEBUG_INFO <<  wikiDataURL;
                    }
                }
            }
        }
    }
    _postNetwork(r);      //  We're done with networking.

    if(wikiDataURL.size())
    {
        _wikiDataQValue=wikiDataURL.mid(wikiDataURL.indexOf("wiki/Q")+5);

        //  Continue on with retrieving the wikiData for the album and get the wikipedia page
        QNetworkAccessManager* am = new QNetworkAccessManager(this);

        connect(am, &QNetworkAccessManager::finished,
                this, &ExternalData::processWikidata);

        const QString url=QString("https://www.wikidata.org/w/api.php?action=wbgetentities&format=json&ids=%1&props=claims%7Clabels%7Cdescriptions%7Csitelinks%2Furls&languages=en&languagefallback=1&normalize=1&sitefilter=enwiki&formatversion=2")
                                .arg(_wikiDataQValue);
        _sendNetworkRequest(am, url);
    }
    else
    {
        //  Not much luck finding the data we're looking for. On to the next set of data.
        if(_currentOffset < MUSICBRAINZ_MAXNUM * 10)
        {
            _currentOffset+=MUSICBRAINZ_MAXNUM;
            _getMBIDAndMore();
        }
        else
        {
            _wikipediaURLRetrievedFlag=1;   //  No more. At least not for _currentKey.
        }
    }
    return;
}


void
ExternalData::processWikidata(QNetworkReply *r)
{
    if(r->error())
    {
        qDebug() << SB_DEBUG_ERROR << r->errorString();
        qDebug() << SB_DEBUG_ERROR << r->error();
    }
    else
    {
        QString searchWikipediaPage=QString("entities/%1/sitelinks/enwiki/url").arg(_wikiDataQValue);
        QByteArray a=r->readAll();
        QJsonParseError jpe;
        QJsonDocument jd=QJsonDocument::fromJson(a,&jpe);
        if(jpe.error!=QJsonParseError::NoError)
        {
            qDebug() << SB_DEBUG_ERROR << jpe.errorString();
            return;
        }
        QStringList sl=_inspectJsonDoc(jd,searchWikipediaPage,0);
        QString url;
        if(sl.size())
        {
            //  Don't expect multiple results-- take the 1st one
            url =sl[0] + QString("?printable=yes");
        }
        qDebug() << SB_DEBUG_INFO << "wikipedia page:" << url;
        _wikipediaURLRetrievedFlag=1;

        switch(_currentKey.itemType())
        {
            case SBKey::Song:
            {
                emit songWikipediaPageAvailable(url);
                break;
            }

            case SBKey::Performer:
            {
                emit performerWikipediaPageAvailable(url);
                break;
            }

            case SBKey::Album:
            {
                emit albumWikipediaPageAvailable(url);
                break;
            }

            default:
            {
                qDebug() << SB_DEBUG_ERROR << "type" << _currentKey.itemType()
                         << "not handled for" << _currentKey.toString()
                    ;
            }
        }

        //  Additional processing
        if(_currentKey.itemType()==SBKey::Performer)
        {
            //  Check if we don't have an performer image, if not, download
            QString fn=getCachePath(_currentKey);
            QFile f(fn);
            if(!f.exists())
            {
                QString searchImage=QString("entities/%1/claims/P18/mainsnak/datavalue/value").arg(_wikiDataQValue);
                const QStringList sl=_inspectJsonDoc(jd,searchImage,0);
                QStringList il;
                for(auto result : sl)
                {
                    QString fileName=result.replace(' ','_');
                    QString md5sum = QString(QCryptographicHash::hash((fileName.toUtf8()),QCryptographicHash::Md5).toHex());
                    QString imageURL=QString("https://upload.wikimedia.org/wikipedia/commons/%1/%2/%3").arg(md5sum.mid(0,1)).arg(md5sum.mid(0,2)).arg(fileName);
                    il << imageURL;
                }
                _artworkRetrievedFlag=1;
                emit(startRetrieveImageData(il,_currentKey));
                //  CWIP: find out if song artwork can be retrieved like this.
            }
        }
    }
    _postNetwork(r);      //  We're done with networking.
}

//  PRIVATE METHODS
void
ExternalData::_init()
{
    _currentOffset=0;
    _softInit();

    //	Start thread to retrieve images
    BackgroundThread* bgt;

    bgt=new BackgroundThread();
    bgt->moveToThread(&t_SingleImageData);
    connect(&t_SingleImageData, &QThread::finished, bgt, &QObject::deleteLater);
    connect(this, &ExternalData::startRetrieveImageData, bgt, &BackgroundThread::retrieveImageData);
    connect(bgt, &BackgroundThread::imageDataReady, this, &ExternalData::processImageRetrieved);
    t_SingleImageData.start();

    bgt=new BackgroundThread();
    bgt->moveToThread(&t_AlbumImageLocations);
    connect(&t_AlbumImageLocations, &QThread::finished, bgt, &QObject::deleteLater);
    connect(this, &ExternalData::startAlbumImageProcess, bgt, &BackgroundThread::processAlbumImages);   //  Initiate processing list
    connect(bgt, &BackgroundThread::retrieveSingleAlbumImageLocations, this, &ExternalData::processSingleAlbumImageLocations);                   //
                                                                                                        //  BGT class asking this to retrieve
                                                                                                        //  and process image locations
    connect(this, &ExternalData::handleNextAlbumImageLocation, bgt, &BackgroundThread::retrieveNextAlbumImageLocations);
    t_AlbumImageLocations.start();
}

void
ExternalData::_softInit()
{
    _artworkRetrievedFlag=0;
    _performerMBIDRetrievedFlag=0;
    _performerHomepageRetrievedFlag=0;
    _songLyricsURLRetrievedFlag=0;
    _wikipediaURLRetrievedFlag=0;
    _wikiDataQValue.clear();
    _allNewsItems.clear();
    _allReviews.clear();
    _currentOffset=0;
}

///
/// \brief ExternalData::_getMBIDAndMore
///
/// Due to the asynchronous processing of network requets, the purpose of this function is to:
/// -	retrieve the MBID for the performer, if this is not already retrieved from the database.
/// 	This is another asynchronous call, the receiving end should call _getMBIDAndMore() in order
/// 	to continue loading whatever is needed for this.currentID.
/// -	load whatever data that is relevant for this.currentID. This function is called in each
/// 	of the main interface functions to load whatever is remaining (as some items are cached or
/// 	or stored in the database).
void
ExternalData::_getMBIDAndMore()
{
    qDebug() << SB_DEBUG_INFO;
    QString urlString;

    SBIDPtr ptr=CacheManager::get(_currentKey);
    SB_RETURN_VOID_IF_NULL(ptr);

    qDebug() << SB_DEBUG_INFO << ptr->genericDescription();

    //  Check if MBID has been retrieved for performer-- if not, retrieve
    if(ptr->itemType()==SBKey::Performer && (ptr->MBID().length()==0))
    {
        if(_performerMBIDRetrievedFlag==1)
        {
            //	Unable to retrieve performerMBID, bail out
            qDebug() << SB_DEBUG_WARNING << "No performerMBID available for" << _currentKey;
            return;
        }

        QNetworkAccessManager* am = new QNetworkAccessManager(this);

        connect(am, &QNetworkAccessManager::finished,
                this, &ExternalData::processMBID);

        const QString url=QString("http://musicbrainz.org/ws/2/artist/?query=artist:%1&fmt=json")
                                .arg(ptr->commonPerformerName());
        _sendNetworkRequest(am, url);
    }
    else
    {
        if(ptr->itemType()==SBKey::Performer)
        {
            //	1.	Wikipedia, performer home page
            if(_wikipediaURLRetrievedFlag==0 || _performerHomepageRetrievedFlag==0)
            {
                QNetworkAccessManager* am = new QNetworkAccessManager(this);

                connect(am, &QNetworkAccessManager::finished,
                        this, &ExternalData::processPerformer);

                const QString url=QString("http://musicbrainz.org/ws/2/artist/%1?inc=url-rels&fmt=json")
                    .arg(ptr->MBID());
                _sendNetworkRequest(am, url);
            }
            else
            {
                qDebug() << SB_DEBUG_WARNING << "Looks like we already have wiki and home page for performer";
            }

            //	2.	Get news items from echonest
            //      Defunct
        }
        else if(_currentKey.itemType()==SBKey::Album)
        {
            qDebug() << SB_DEBUG_INFO << "handling album";
            SBIDAlbumPtr aPtr=SBIDAlbum::retrieveAlbum(_currentKey);
            SB_RETURN_VOID_IF_NULL(aPtr);
            qDebug() << SB_DEBUG_INFO << aPtr->genericDescription();
            qDebug() << SB_DEBUG_INFO << _artworkRetrievedFlag << _wikipediaURLRetrievedFlag;

            //	1.	Artwork
            if(_artworkRetrievedFlag==0)
            {
                //      A.  Find recordings for performer.
                const SBIDPerformerPtr pPtr=aPtr->albumPerformerPtr();
                if(pPtr)
                {
                    QNetworkAccessManager* am = new QNetworkAccessManager(this);

                    connect(am, &QNetworkAccessManager::finished,
                            this, &ExternalData::processAlbumsByPerformer);

                    const QString url=QString("https://musicbrainz.org/ws/2/release/?artist=%1&inc=recordings&fmt=json")
                                         .arg(pPtr->MBID());
                    _sendNetworkRequest(am, url);
                }
                else
                {
                    qDebug() << SB_DEBUG_ERROR << "Performer name not found for album" << aPtr->albumID();
                }
            }

            //	2.	Wikipedia page
            if(_wikipediaURLRetrievedFlag==0)
            {
                //	Get urls for given album
                QNetworkAccessManager* am = new QNetworkAccessManager(this);

                connect(am, &QNetworkAccessManager::finished,
                        this, &ExternalData::processAlbum);

                QString performerMBID=aPtr->albumPerformerMBID();
                const QString url=QString("https://musicbrainz.org/ws/2/release-group?artist=%1&inc=url-rels&offset=0&limit=%2&fmt=json")
                                        .arg(performerMBID)
                                        .arg(MUSICBRAINZ_MAXNUM);
                _sendNetworkRequest(am, url);
            }
            else
            {
                qDebug() << SB_DEBUG_WARNING << "Looks like we already have wiki for album";
            }
        }
        else if(_currentKey.itemType()==SBKey::Song)
        {
            SBIDSongPtr sPtr=SBIDSong::retrieveSong(_currentKey);
            SB_RETURN_VOID_IF_NULL(sPtr);

            SBIDPerformerPtr pPtr=SBIDPerformer::retrievePerformer(sPtr->songOriginalPerformerID());
            SB_RETURN_VOID_IF_NULL(pPtr);

            //	1.	Wikipedia page
            if(_wikipediaURLRetrievedFlag==0)
            {
                //	Find meta data for song
                QNetworkAccessManager* am=new QNetworkAccessManager(this);
                connect(am, &QNetworkAccessManager::finished,
                        this, &ExternalData::processSong);

                const QString url=QString("http://musicbrainz.org/ws/2/work?artist=%1&inc=url-rels&offset=%2&limit=%3&fmt=json")
                    .arg(pPtr->MBID())
                    .arg(_currentOffset)
                    .arg(MUSICBRAINZ_MAXNUM)
                ;
                qDebug() << SB_DEBUG_INFO << url;
                _sendNetworkRequest(am,url);
            }
            else
            {
                qDebug() << SB_DEBUG_WARNING << "Looks like we already have wiki for song";
            }
        }
    }
}

bool
ExternalData::_loadImageFromCache(QPixmap& p,const SBKey& key)
{
    QString fn=getCachePath(key);
    QFile f(fn);

    if(f.open(QIODevice::ReadOnly))
    {
        int len=f.size();
        char* mem=(char *)malloc(len);
        QDataStream s(&f);
        int n=s.readRawData(mem,len);
        f.close();

        if(n>0)
        {
            QByteArray a=QByteArray(mem,len);
            p.loadFromData(a);
            return 1;
        }
        free(mem);
    }
    return 0;
}

void
ExternalData::_postNetwork(QNetworkReply* r)
{
    SB_RETURN_VOID_IF_NULL(r);
    r->deleteLater();
    QNetworkAccessManager* am=r->manager();
    SB_RETURN_VOID_IF_NULL(am);
    am->deleteLater();
}

void
ExternalData::_sendNetworkRequest(QNetworkAccessManager* am, const QString &url)
{
    SB_RETURN_VOID_IF_NULL(am);
    _sendNetworkRequest(am, QUrl(url,QUrl::StrictMode));
}

void
ExternalData::_sendNetworkRequest(QNetworkAccessManager* am, const QUrl &url)
{
    SB_RETURN_VOID_IF_NULL(am);

    if(url.errorString().size())
    {
        qDebug() << SB_DEBUG_ERROR << "errorString=" << url.errorString();
    }

    QNetworkRequest nr;
    nr.setUrl(url);
    nr.setHeader(QNetworkRequest::UserAgentHeader,QVariant("MezzoForta!/1.0"));
    am->get(nr);
}

void
ExternalData::_storeInCache(QByteArray *a) const
{
    QString fn=getCachePath(_currentKey);
    QFile f(fn);
    if(f.open(QIODevice::WriteOnly))
    {
        QDataStream s(&f);
        s.writeRawData(a->constData(),a->length());
        f.close();
    }
}


//  PRIVATE JSON METHODS
///
/// \brief ExternalData::_inspectJsonDoc
///
/// Go recursively through a JSON document and:
///     -   prints outs the entire doc as debugging output
///         (search=QString(), debug=1)
///
///     -   searches hierarchically for `key` by specifying the search
///         parameter as:
///             "<path>/key>"
///         eg: "entities/<MBID>/sitelinks/enwiki/url"
///             retrieves the value of url in specified path.
///
///     -   Finds `attribute` with `value` and retrieves the value of `key`
///         by specifying the search parameter as:
///             "<path>/attribute=value?key>"
///         eg: release/title=boy/id
///             retrieves the value of `id` in any element under `release`
///             in which the title is equal to `boy`.
///
QStringList
ExternalData::_inspectJsonDoc(const QJsonDocument& jd, const QString& search, const bool& debug) const
{
    QStringList vl;
    const QStringList sl= (search.size()>0) ? search.split('/'):QStringList();
    if(debug && search.size()) qDebug() << SB_DEBUG_INFO << "search for:" << search;
    if(jd.isArray())
    {
        QJsonArray ja=jd.array();
        vl=_iterateJsonArray(ja,sl,0,debug);
    }
    else if(jd.isObject())
    {
        vl=_recurseJsonObject(jd.object(),sl,0,debug);
    }
    return vl;
}

QStringList
ExternalData::_iterateJsonArray(const QJsonArray& ar, const QStringList& search, const int& recursion, const bool& debug) const
{
    QStringList sl;
    const QString ts=_crtIndent(recursion);
    for(qsizetype i=0;i<ar.size();i++)
    {
        if(debug) qDebug() << SB_DEBUG_INFO << ts << "iterate through the next array item:";
        const QJsonValue jv=ar.at(i);
        const QStringList result=_inspectJsonValue(jv,search,recursion+1,debug);
        if(search.size() && result.size())
        {
            sl << result;
        }
    }
    return sl;
}

QStringList
ExternalData::_recurseJsonObject(const QJsonObject& jo, const QStringList& search, const int& recursion, const bool& debug) const
{
    QStringList sl;
    const QString ts=_crtIndent(recursion);
    QString searchAttribute;
    QString searchValue;
    const static QChar question('?');
    const int searchAttrPosition=(search.size())?search[0].indexOf(question):0;
    if(searchAttrPosition>0)  //  we don't expect to find a '?' at position 0
    {
        searchAttribute=search[0].sliced(searchAttrPosition+1);
    }

    QString queryAttribute;
    QString queryValue;
    const static QChar equals('=');
    const int searchEqualPosition=(search.size())?search[0].indexOf(equals):0;
    if(searchEqualPosition>0) //  we don't expect to find a '=' at position 0
    {
        queryAttribute=search[0].left(searchEqualPosition);
        queryValue=_normalizeString(search[0].mid(searchEqualPosition+1,searchAttrPosition?searchAttrPosition-searchEqualPosition-1:-1));
    }
    if(debug && searchAttrPosition>0) qDebug() << SB_DEBUG_INFO << ts << "Look for:" << queryAttribute;
    if(debug && searchEqualPosition>0) qDebug() << SB_DEBUG_INFO << ts << "With value:" << queryValue;
    if(debug && searchAttrPosition>0) qDebug() << SB_DEBUG_INFO << ts << "And retrieve:" << searchAttribute;

    const QString findKey=(queryAttribute.size())?queryAttribute:(search.size()?search[0]:QString());

    for ( const auto& key : jo.keys() )
    {
        int position=0;
        QString isFound;
        QJsonValue jv=jo.value(key);
        if(findKey.size()==0 || (findKey==key) || (searchAttribute==key))
        {
            if(findKey==key)
            {
                if(queryAttribute.size())
                {
                    //  Don't need to report if not running a query.
                    isFound="FOUND queryAttribute:";
                }
                else
                {
                    isFound="FOUND:";
                }
                position=1;
            }
            if(searchAttribute==key)
            {
                isFound="FOUND searchAttribute:";
            }

            if(debug) qDebug() << SB_DEBUG_INFO << ts << isFound << "key=" << key;
            const QStringList jvs=_inspectJsonValue(jv,search.mid(position),recursion+2,debug);

            for(const auto& jsonValue: jvs)
            {
                if(jsonValue.size())
                {
                    if(!queryAttribute.size())
                    {
                        sl << jsonValue;
                    }
                    if(searchAttribute==key)
                    {
                        searchValue=jsonValue;
                    }
                    if(queryAttribute==key && queryValue==_normalizeString(jsonValue))
                    {
                        if(queryValue.size())
                        {
                            if(debug) qDebug() << SB_DEBUG_INFO << ts << "QUERY FOUND"
                                     << queryAttribute << "=" << queryValue << ":"
                                     << searchAttribute << "=" << searchValue;
                            sl << searchValue;
                        }
                    }
                }
            }
        }
    }
    return sl;
}

QStringList
ExternalData::_inspectJsonValue(const QJsonValue& jv, const QStringList& search, const int& recursion, const bool& debug) const
{
    const QString ts=_crtIndent(recursion);
    const QJsonValue::Type jt=jv.type();
    switch(jt)
    {
        case QJsonValue::Null:
            {
                if(debug) qDebug() << SB_DEBUG_INFO << ts << "no value=";
                return QStringList();
                break;
            }

        case QJsonValue::Bool:
            {
                if(debug) qDebug() << SB_DEBUG_INFO << ts << "bool value=" << jv.toBool();
                return QStringList(QString(jv.toBool()?"TRUE":"FALSE"));
                break;
            }

        case QJsonValue::Double:
            {
                if(debug) qDebug() << SB_DEBUG_INFO << ts << "double value=" << jv.toDouble();
                return QStringList(QString::number(jv.toDouble()));
                break;
            }

        case QJsonValue::String:
            {
                if(debug) qDebug() << SB_DEBUG_INFO << ts << "string value=" << jv.toString();
                return QStringList(jv.toString());
                break;
            }

        case QJsonValue::Array:
            {
                if(debug) qDebug() << SB_DEBUG_INFO << ts << "array:";
                QJsonArray ja=jv.toArray();
                const QStringList result=_iterateJsonArray(ja,search,recursion+1,debug);
                if(search.size() && result.size())
                {
                    return result;
                }
                break;
            }

        case QJsonValue::Object:
            {
                QJsonObject jo=jv.toObject();
                const QStringList result=_recurseJsonObject(jo,search,recursion+1,debug);
                if(search.size() && result.size())
                {
                    return result;
                }
                break;
            }

        case QJsonValue::Undefined:
            {
                qDebug() << SB_DEBUG_INFO << ts << "Undefined value. Error inside JSON data";
                break;
            }

        default:
            {
                qDebug() << SB_DEBUG_INFO << ts << "type" << jt << "not handled.";
                break;
            }
    }
    return QStringList();
}

QString
ExternalData::_crtIndent(int tabstops) const
{
    const static QString singleTab("   ");
    QString result;
    for(int i=0; i<tabstops; i++)
    {
        result+=singleTab;
    }
    return result;
}

QString
ExternalData::_normalizeString(const QString& str) const
{
    return str.toLower().replace("’","'").replace("“","\"").replace("”","\"");
}

