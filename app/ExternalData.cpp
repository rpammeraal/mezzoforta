#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QLabel>
#include <QSemaphore>
#include <QStandardPaths>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QPixmap>
#include <QWebView>

#include "Common.h"
#include "ExternalData.h"

#include <QMessageBox>

ExternalData::ExternalData(QObject *parent) : QObject(parent)
{
    init();
}

ExternalData::~ExternalData()
{
}

QString
ExternalData::getCachePath(const SBID& id)
{
    QString p=QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    p.replace("!","");
    QDir d;
    d.mkpath(p);
    QString f=QString("%1/%2.%3").arg(p).arg(id.getType()).arg(id.sb_item_id);
    qDebug() << SB_DEBUG_INFO << f;
    return f;
}

void
ExternalData::loadAlbumData(const SBID &id)
{
    currentID=id;
    qDebug() << SB_DEBUG_INFO << currentID;

    //	Load album image
    //	1.	Clear image
    QPixmap p;

    //	2.	Try load from cache, other retrieve using audioscrobbler
    if(loadImageFromCache(p,currentID))
    {
        emit imageDataReady(p);
    }
    else
    {
        qDebug() << SB_DEBUG_INFO;

        QNetworkAccessManager* m=new QNetworkAccessManager(this);
        connect(m, SIGNAL(finished(QNetworkReply *)),
                this, SLOT(albumCoverMetadataRetrievedAS(QNetworkReply*)));

        QString URL=QString("http://ws.audioscrobbler.com/2.0/?method=album.search&limit=99999&api_key=5dacbfb3b24d365bcd43050c6149a40d&album=%1").
                arg(currentID.albumTitle);
        m->get(QNetworkRequest(QUrl(URL)));
    }

    //	3.	Load wikipedia page from musicbrainz
    //		Thanks to async nature of network retrievals, there is some hacking involved.
    //		Call retrievePerformerMBID to get the mbid of performer (either from database or network).
    //		When this is retrieved, this same function will continue other data that depends on this mbid.

    qDebug() << SB_DEBUG_INFO << "album wiki:" << id.wiki;
    if(id.wiki.length()>0)
    {
        qDebug() << SB_DEBUG_INFO;
        albumWikipediaPageRetrieved=1;
        emit albumWikipediaPageAvailable(id.wiki);
    }
    qDebug() << SB_DEBUG_INFO;
    retrievePerformerMBID();
}

///
/// \brief ExternalData::setImageFromCache
/// \param id
/// \param l
/// \return 1 if image is successfully loaded from cache
///
bool
ExternalData::loadImageFromCache(QPixmap& p,const SBID& id)
{
    QString fn=getCachePath(id);
    QFile f(fn);

    if(f.open(QIODevice::ReadOnly))
    {
        int len=f.size();
        char* mem=(char *)malloc(len);
        QDataStream s(&f);
        int n=s.readRawData(mem,len);
            qDebug() << SB_DEBUG_INFO;

        f.close();

        if(n>0)
        {
            QByteArray a=QByteArray(mem,len);
            p.loadFromData(a);
            qDebug() << SB_DEBUG_INFO;
            return 1;
        }
        free(mem);
    }
    return 0;
}

void
ExternalData::loadPerformerData(const SBID id)
{
    qDebug() << SB_DEBUG_INFO << id << id.url << id.wiki;
    currentID=id;
    if(id.url.length()>0)
    {
        qDebug() << SB_DEBUG_INFO;
        performerHomepageRetrieved=1;
        emit performerHomePageAvailable(id.url);
        qDebug() << SB_DEBUG_INFO;
    }
    if(id.wiki.length()>0)
    {
        qDebug() << SB_DEBUG_INFO;
        performerWikipediaPageRetrieved=1;
        emit performerWikipediaPageAvailable(id.wiki);
    }
    qDebug() << SB_DEBUG_INFO;
    retrievePerformerMBID();
    qDebug() << SB_DEBUG_INFO;
}

void
ExternalData::loadSongData(const SBID& id)
{
    currentID=id;
    if(id.wiki.length()>0)
    {
        songWikipediaPageRetrieved=1;
        emit songWikipediaPageAvailable(id.wiki);
    }
    retrievePerformerMBID();
}

void
ExternalData::albumCoverMetadataRetrievedAS(QNetworkReply *r)
{
    qDebug() << SB_DEBUG_INFO;

    if(r->error()==QNetworkReply::NoError)
    {
    qDebug() << SB_DEBUG_INFO;
        if(r->open(QIODevice::ReadOnly))
        {
    qDebug() << SB_DEBUG_INFO;
            QByteArray a=r->readAll();
            QString s=QString(a.data());

            QDomDocument doc;
            QString errorMsg;
            int errorLine;
            int errorColumn;
            doc.setContent(a,0,&errorMsg,&errorLine,&errorColumn);

            QDomNodeList nl=doc.elementsByTagName(QString("albummatches"));

            if(nl.count()>0)
            {
                QDomNode     level1node    =nl.at(0);
                QDomNodeList level1nodelist=level1node.childNodes();

                for(int i=0; i<level1nodelist.count(); i++)
                {
                    SBID pm;
                    QString key;
                    QString value;
                    QString URL;

                    QDomNode     level2node    =level1nodelist.at(i);
                    QDomNodeList level2nodelist=level2node.childNodes();

                    for(int j=0; j<level2nodelist.count();j++)
                    {
                        QDomNode     level3node    =level2nodelist.at(j);
                        QDomNodeList level3nodelist=level3node.childNodes();

                        key=level3node.nodeName();

                        for(int k=0; k<level3nodelist.count();k++)
                        {
                            QDomNode level4node=level3nodelist.at(k);
                            value=level4node.nodeValue();
                        }

                        if(key=="name")
                        {
                            pm.albumTitle=value;
                        }
                        else if(key=="artist")
                        {
                            pm.performerName=value;
                        }
                        else if(key=="image")
                        {
                            URL=value;
                        }
                    }

                    qDebug() << SB_DEBUG_INFO << URL;
                    if(currentID.fuzzyMatch(pm)==1)
                    {
                        QNetworkAccessManager* n=new QNetworkAccessManager(this);
                        connect(n, SIGNAL(finished(QNetworkReply *)),
                                this, SLOT(imagedataRetrieved(QNetworkReply*)));

                        QUrl url(URL);
                        n->get(QNetworkRequest(url));
                        return;
                    }
                }
            }
        }
    }
    else
    {
        QMessageBox messagebox;
        messagebox.setText(r->errorString());
        messagebox.exec();
    }
}

//	TESTED
void
ExternalData::albumURLDataRetrievedMB(QNetworkReply *r)
{
    QString matchAlbumName=Common::removeNonAlphanumeric(currentID.albumTitle).toLower();
    QString foundAlbumName;
    allReviews.clear();

    if(r->error()==QNetworkReply::NoError)
    {
        if(r->open(QIODevice::ReadOnly))
        {
            QByteArray a=r->readAll();
            QString s=QString(a.data());

            QDomDocument doc;
            QString errorMsg;
            int errorLine;
            int errorColumn;
            doc.setContent(a,0,&errorMsg,&errorLine,&errorColumn);

            QDomElement e=doc.documentElement();

            for(QDomNode n=e.firstChild();!n.isNull();n = n.nextSibling())
            {
                QDomElement e = n.toElement();

                if(!e.isNull())
                {
                    for(QDomNode n=e.firstChild();!n.isNull();n = n.nextSibling())
                    {
                        QDomElement e = n.toElement();

                        if(!e.isNull())
                        {
                            for(QDomNode n=e.firstChild();!n.isNull();n = n.nextSibling())
                            {
                                QDomElement e = n.toElement();

                                if(e.tagName()=="title")
                                {
                                    foundAlbumName=Common::removeNonAlphanumeric(e.text()).toLower();
                                }
                                if(!e.isNull())
                                {
                                    for(QDomNode n=e.firstChild();!n.isNull();n = n.nextSibling())
                                    {
                                        QDomElement e = n.toElement();

                                        if(e.attribute("type")=="wikipedia" && matchAlbumName==foundAlbumName && albumWikipediaPageRetrieved==0)
                                        {
                                            QString URL=e.text() + "&printable=yes";
                                            URL.replace("/wiki/","/w/index.php?title=");
                                            albumWikipediaPageRetrieved=1;
                                            emit albumWikipediaPageAvailable(URL);
                                        }
                                        else if(e.attribute("type")=="review" && matchAlbumName==foundAlbumName)
                                        {
                                            QString URL=e.text() + "&printable=yes";
                                            allReviews.append(e.text());
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if(allReviews.count()>0)
    {
        emit albumReviewsAvailable(allReviews);
    }
}

void
ExternalData::imagedataRetrieved(QNetworkReply *r)
{
        qDebug() << SB_DEBUG_INFO << r->url().toString();
    if(r->error()==QNetworkReply::NoError)
    {
        qDebug() << SB_DEBUG_INFO;
        if(r->open(QIODevice::ReadOnly))
        {
            qDebug() << SB_DEBUG_INFO << r->bytesAvailable();
            qDebug() << SB_DEBUG_INFO << r->canReadLine();
            qDebug() << SB_DEBUG_INFO << r->isReadable();
            qDebug() << SB_DEBUG_INFO << r->size();
            QByteArray a=r->readAll();
            qDebug() << SB_DEBUG_INFO << a.count();
            if(a.count()>0)
            {
            qDebug() << SB_DEBUG_INFO;
                QPixmap image;

                //	Store in cache
                image.loadFromData(a);
                storeInCache(&a);
            qDebug() << SB_DEBUG_INFO;
                emit imageDataReady(image);
            }
        }
    }
}

//	TESTED
void
ExternalData::performerMBIDRetrieved(QNetworkReply *r)
{
    bool matchFound=0;
    QString title=Common::removeNonAlphanumeric(currentID.performerName.toLower());

    if(r->error()==QNetworkReply::NoError)
    {
        if(r->open(QIODevice::ReadOnly))
        {
            QByteArray a=r->readAll();
            QString s=QString(a.data());

            QDomDocument doc;
            QString errorMsg;
            int errorLine;
            int errorColumn;
            doc.setContent(a,0,&errorMsg,&errorLine,&errorColumn);

            QDomElement e=doc.documentElement();

            for(QDomNode n=e.firstChild();!n.isNull() && matchFound==0;n = n.nextSibling())
            {
                QDomElement e = n.toElement();
                if(!e.isNull())
                {
                    for(QDomNode n=e.firstChild();!n.isNull() && matchFound==0;n = n.nextSibling())
                    {
                        QString MBID;

                        QDomElement e = n.toElement();
                        if(!e.isNull())
                        {
                            MBID=e.attribute("id");

                            for(QDomNode n=e.firstChild();!n.isNull() && matchFound==0;n = n.nextSibling())
                            {
                                QDomElement e = n.toElement();
                                if(!e.isNull())
                                {
                                    if(e.tagName()=="name")
                                    {
                                        if(Common::removeNonAlphanumeric(e.text().toLower())==title)
                                        {
                                            currentID.sb_mbid=MBID;
                                            matchFound=1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if(matchFound==1)
    {
        //	Recall now that sb_mbid is loaded
        emit updatePerformerMBID(currentID);
        retrievePerformerMBID();
    }
}

//	TESTED
void
ExternalData::performerImageRetrievedEN(QNetworkReply *r)
{
    bool matchFound=0;

    QString url;

    if(r->error()==QNetworkReply::NoError)
    {
        if(r->open(QIODevice::ReadOnly))
        {
            QByteArray a=r->readAll();

            QDomDocument doc;
            QString errorMsg;
            int errorLine;
            int errorColumn;
            doc.setContent(a,0,&errorMsg,&errorLine,&errorColumn);

            QDomElement e=doc.documentElement();

            for(QDomNode n=e.firstChild();!n.isNull() && matchFound==0;n = n.nextSibling())
            {
                QDomElement e = n.toElement();

                if(!e.isNull() && e.tagName()=="images")
                {
                    for(QDomNode n=e.firstChild();!n.isNull() && matchFound==0;n = n.nextSibling())
                    {
                        QDomElement e = n.toElement();

                        if(!e.isNull())
                        {
                            for(QDomNode n=e.firstChild();!n.isNull() && matchFound==0;n = n.nextSibling())
                            {
                                QDomElement e = n.toElement();

                                if(e.tagName()=="url")
                                {
                                    url=e.text();
                                }
                                else if(!e.isNull())
                                {
                                    for(QDomNode n=e.firstChild();!n.isNull() && matchFound==0;n = n.nextSibling())
                                    {
                                        QDomElement e = n.toElement();

                                        if(e.tagName()=="attribution" && e.text()!="myspace")
                                        {
                                            qDebug() << SB_DEBUG_INFO << url;
                                            QNetworkAccessManager* m=new QNetworkAccessManager(this);
                                            connect(m, SIGNAL(finished(QNetworkReply *)),
                                                    this, SLOT(imagedataRetrieved(QNetworkReply*)));

                                            m->get(QNetworkRequest(QUrl(url)));
                                            return;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

//	TESTED
void
ExternalData::performerNewsRetrievedEN(QNetworkReply *r)
{
    bool matchFound=0;
    allNewsItems.clear();

    QString url;

    if(r->error()==QNetworkReply::NoError)
    {
        if(r->open(QIODevice::ReadOnly))
        {
            QByteArray a=r->readAll();
            QString s=QString(a.data());

            QDomDocument doc;
            QString errorMsg;
            int errorLine;
            int errorColumn;
            doc.setContent(a,0,&errorMsg,&errorLine,&errorColumn);

            QDomElement e=doc.documentElement();

            for(QDomNode n=e.firstChild();!n.isNull() && matchFound==0;n = n.nextSibling())
            {
                QDomElement e = n.toElement();

                if(!e.isNull() && e.tagName()=="news")
                {
                    for(QDomNode n=e.firstChild();!n.isNull();n = n.nextSibling())
                    {
                        QDomElement e = n.toElement();

                        if(!e.isNull() && e.tagName()=="news")
                        {
                            NewsItem item;

                            for(QDomNode n=e.firstChild();!n.isNull();n = n.nextSibling())
                            {
                                QDomElement e = n.toElement();
                                if(e.tagName()=="url")
                                {
                                    item.url=e.text();
                                }
                                else if(e.tagName()=="name")
                                {
                                    item.name=e.text();
                                }
                                else if(e.tagName()=="summary")
                                {
                                    item.summary=e.text();
                                }
                            }
                            allNewsItems.append(item);
                        }
                    }
                }
            }
        }
    }
    if(allNewsItems.count()>0)
    {
        emit performerNewsAvailable(allNewsItems);
    }
}

void
ExternalData::performerURLDataRetrievedMB(QNetworkReply *r)
{
    if(r->error()==QNetworkReply::NoError)
    {
        if(r->open(QIODevice::ReadOnly))
        {
            QByteArray a=r->readAll();
            QString s=QString(a.data());

            QDomDocument doc;
            QString errorMsg;
            int errorLine;
            int errorColumn;
            doc.setContent(a,0,&errorMsg,&errorLine,&errorColumn);

            QDomElement e=doc.documentElement();

            for(QDomNode n=e.firstChild();!n.isNull() && (performerWikipediaPageRetrieved==0 || performerHomepageRetrieved==0);n = n.nextSibling())
            {
                QDomElement e = n.toElement();

                if(!e.isNull())
                {
                    for(QDomNode n=e.firstChild();!n.isNull() && (performerWikipediaPageRetrieved==0 || performerHomepageRetrieved==0);n = n.nextSibling())
                    {
                        QString MBID;

                        QDomElement e = n.toElement();

                        if(!e.isNull())
                        {
                            for(QDomNode n=e.firstChild();!n.isNull() && (performerWikipediaPageRetrieved==0 || performerHomepageRetrieved==0);n = n.nextSibling())
                            {
                                QDomElement e = n.toElement();

                                if(e.attribute("type")=="wikipedia" && performerWikipediaPageRetrieved==0)
                                {
                                    QString URL=e.text() + "&printable=yes";
                                    URL.replace("/wiki/","/w/index.php?title=");
                                    performerWikipediaPageRetrieved=1;

                                    emit performerWikipediaPageAvailable(URL);
                                }
                                else if(e.attribute("type")=="official homepage" && performerHomepageRetrieved==0)
                                {
                                    currentID.url=e.text();
                                    performerHomepageRetrieved=1;
                                    emit performerHomePageAvailable(currentID.url);
                                    emit updatePerformerHomePage(currentID);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

//	TESTED
void
ExternalData::songMetaDataRetrievedMB(QNetworkReply *r)
{
    QString matchSongName=Common::removeNonAlphanumeric(currentID.songTitle).toLower();
    QString foundSongName;
    int index=0;
    QString mbid;

    if(r->error()==QNetworkReply::NoError)
    {
        if(r->open(QIODevice::ReadOnly))
        {
            QByteArray a=r->readAll();

            QDomDocument doc;
            QString errorMsg;
            int errorLine;
            int errorColumn;
            doc.setContent(a,0,&errorMsg,&errorLine,&errorColumn);

            QDomElement e=doc.documentElement();

            for(QDomNode n=e.firstChild();!n.isNull();n = n.nextSibling())
            {
                QDomElement e = n.toElement();

                if(!e.isNull() && e.hasChildNodes()==1)
                {
                    for(QDomNode n=e.firstChild();!n.isNull();n = n.nextSibling())
                    {
                        QDomElement e = n.toElement();

                        if(e.attribute("type")!="Song" && e.attribute("type")!="")
                        {
                            continue;
                        }

                        mbid=e.attribute("id");

                        if(!e.isNull())
                        {
                            for(QDomNode n=e.firstChild();!n.isNull();n = n.nextSibling())
                            {
                                QDomElement e = n.toElement();

                                if(e.tagName()=="title")
                                {
                                    index++;
                                    foundSongName=Common::removeNonAlphanumeric(e.text()).toLower();
                                    if(foundSongName==matchSongName)
                                    {
                                        //	Now fire off a request to get all URLs for this work.
                                        QNetworkAccessManager* mb=new QNetworkAccessManager(this);
                                        connect(mb, SIGNAL(finished(QNetworkReply *)),
                                                this, SLOT(songURLDataRetrievedMB(QNetworkReply*)));

                                        QString URL=QString("http://musicbrainz.org/ws/2/work/%1?inc=url-rels").arg(mbid);
                                        mb->get(QNetworkRequest(QUrl(URL)));
                                        return;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if(index>0)
            {
                //	If this point is reached, we need to do another iteration
                currentOffset+=MUSICBRAINZ_MAXNUM;
                retrievePerformerMBID();
            }
        }
    }
}

//	TESTED
void
ExternalData::songURLDataRetrievedMB(QNetworkReply *r)
{
    QString type;

    if(r->error()==QNetworkReply::NoError)
    {
        if(r->open(QIODevice::ReadOnly))
        {
            QByteArray a=r->readAll();

            QDomDocument doc;
            QString errorMsg;
            int errorLine;
            int errorColumn;
            doc.setContent(a,0,&errorMsg,&errorLine,&errorColumn);

            QDomElement e=doc.documentElement();

            for(QDomNode n=e.firstChild();!n.isNull() && (songLyricsURLRetrieved==0 || songWikipediaPageRetrieved==0);n = n.nextSibling())
            {
                QDomElement e = n.toElement();

                if(!e.isNull())
                {
                    for(QDomNode n=e.firstChild();!n.isNull() && (songLyricsURLRetrieved==0 || songWikipediaPageRetrieved==0);n = n.nextSibling())
                    {
                        QDomElement e = n.toElement();

                        if(!e.isNull())
                        {
                            for(QDomNode n=e.firstChild();!n.isNull() && (songLyricsURLRetrieved==0 || songWikipediaPageRetrieved==0);n = n.nextSibling())
                            {
                                QDomElement e = n.toElement();

                                type=e.attribute("type");
                                if(!e.isNull())
                                {
                                    for(QDomNode n=e.firstChild();!n.isNull() && (songLyricsURLRetrieved==0 || songWikipediaPageRetrieved==0);n = n.nextSibling())
                                    {
                                        QDomElement e = n.toElement();

                                        if(type=="lyrics" && songLyricsURLRetrieved==0)
                                        {
                                            songLyricsURLRetrieved=1;
                                            emit songLyricsURLAvailable(e.text());
                                        }
                                        else if(type=="wikipedia" && songWikipediaPageRetrieved==0)
                                        {
                                            QString URL=e.text() + "&printable=yes";
                                            URL.replace("/wiki/","/w/index.php?title=");
                                            songWikipediaPageRetrieved=1;
                                            emit songWikipediaPageAvailable(URL);
                                        }
                                        else
                                        {
                                            qDebug() << SB_DEBUG_INFO << "Other URL:type=" << type << e.text();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

///	PRIVATE
///
void
ExternalData::loadAlbumCoverAS()
{
}

void
ExternalData::retrievePerformerMBID()
{
    QNetworkAccessManager* en;
    QString URL;
    QNetworkAccessManager* mb;

    if(currentID.sb_mbid.length()==0)
    {
        //	Unknown MBID -- look it up and store.
        QNetworkAccessManager* m=new QNetworkAccessManager(this);
        connect(m, SIGNAL(finished(QNetworkReply *)),
                this, SLOT(performerMBIDRetrieved(QNetworkReply*)));

        QString URL=QString("http://musicbrainz.org/ws/2/artist/?query=artist:%1").arg(currentID.performerName);
        m->get(QNetworkRequest(QUrl(URL)));
    }
    else
    {
        if(currentID.sb_item_type==SBID::sb_type_performer)
        {
            //	Continue on with retrieving performer specific data

            //	Get wikipedia and homepage urls from musicbrainz
            if(performerWikipediaPageRetrieved==0 || performerHomepageRetrieved==0)
            {
                mb=new QNetworkAccessManager(this);
                connect(mb, SIGNAL(finished(QNetworkReply *)),
                        this, SLOT(performerURLDataRetrievedMB(QNetworkReply*)));

                URL=QString("http://musicbrainz.org/ws/2/artist/%1?inc=url-rels").arg(currentID.sb_mbid);
                qDebug() << SB_DEBUG_INFO << URL;
                mb->get(QNetworkRequest(QUrl(URL)));
            }
            else
            {
                qDebug() << SB_DEBUG_INFO << "Looks like we already have wiki and home page for performer";
            }

            //	Get performer image. If not in cache, find suitable image from echonest
            QPixmap p;
            if(loadImageFromCache(p,currentID))
            {
                emit imageDataReady(p);
            }
            else
            {
                //	Get performer image from echonest
                en=new QNetworkAccessManager(this);
                connect(en, SIGNAL(finished(QNetworkReply *)),
                        this, SLOT(performerImageRetrievedEN(QNetworkReply *)));

                URL=QString("http://developer.echonest.com/api/v4/artist/images?api_key=BYNRSUS9LPOC2NYUI&id=musicbrainz:artist:%1&format=xml").arg(currentID.sb_mbid);
                qDebug() << SB_DEBUG_INFO << URL;
                en->get(QNetworkRequest(QUrl(URL)));
            }

            //	Get news items from echonest
            en=new QNetworkAccessManager(this);
            connect(en, SIGNAL(finished(QNetworkReply *)),
                    this, SLOT(performerNewsRetrievedEN(QNetworkReply*)));

            URL=QString("http://developer.echonest.com/api/v4/artist/news?api_key=BYNRSUS9LPOC2NYUI&id=musicbrainz:artist:%1&format=xml").arg(currentID.sb_mbid);
            qDebug() << SB_DEBUG_INFO << URL;
            en->get(QNetworkRequest(QUrl(URL)));
        }
        else if(currentID.sb_item_type==SBID::sb_type_album)
        {
            if(albumWikipediaPageRetrieved==0)
            {
                //	Get urls for given album
                QNetworkAccessManager* mb=new QNetworkAccessManager(this);
                connect(mb, SIGNAL(finished(QNetworkReply *)),
                        this, SLOT(albumURLDataRetrievedMB(QNetworkReply*)));

                URL=QString("https://musicbrainz.org/ws/2/release-group?artist=%1&inc=url-rels&offset=0&limit=%2").arg(currentID.sb_mbid).arg(MUSICBRAINZ_MAXNUM);
                qDebug() << SB_DEBUG_INFO << URL;
                mb->get(QNetworkRequest(QUrl(URL)));
            }
            else
            {
                qDebug() << SB_DEBUG_INFO << "Looks like we already have wiki for album";
            }
        }
        else if(currentID.sb_item_type==SBID::sb_type_song)
        {
            if(songWikipediaPageRetrieved==0)
            {
                //	Find meta data for song
                QNetworkAccessManager* mb=new QNetworkAccessManager(this);
                connect(mb, SIGNAL(finished(QNetworkReply *)),
                        this, SLOT(songMetaDataRetrievedMB(QNetworkReply*)));

                URL=QString("http://musicbrainz.org/ws/2/work?artist=%1&offset=%2&limit=%3").arg(currentID.sb_mbid).arg(currentOffset).arg(MUSICBRAINZ_MAXNUM);
                mb->get(QNetworkRequest(QUrl(URL)));
            }
            else
            {
                qDebug() << SB_DEBUG_INFO << "Looks like we already have wiki for song";
            }
        }
    }
}

void
ExternalData::init()
{
    currentOffset=0;
    currentID=SBID();
    albumWikipediaPageRetrieved=0;
    performerWikipediaPageRetrieved=0;
    performerHomepageRetrieved=0;
    songWikipediaPageRetrieved=0;
    songLyricsURLRetrieved=0;
}

void
ExternalData::storeInCache(QByteArray *a) const
{
    QString fn=getCachePath(currentID);
    QFile f(fn);
    if(f.open(QIODevice::WriteOnly))
    {
        QDataStream s(&f);
        s.writeRawData(a->constData(),a->length());
        f.close();
    }
}
