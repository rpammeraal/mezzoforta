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

ExternalData::ExternalData(QObject *parent) : QObject(parent)
{
    init();
}

ExternalData::~ExternalData()
{
}

void
ExternalData::loadAlbumCover(const SBID &id)
{
    currentID=id;
    loadAlbumCoverAS();
}

void
ExternalData::loadPerformerData(const SBID id)
{
    qDebug() << SB_DEBUG_INFO << id << id.url;
    currentID=id;
    if(id.url.length()>0)
    {
        performerHomepageRetrieved=1;
        emit performerHomePageAvailable(id.url);
    }
    loadPerformerDataMB();
}

void
ExternalData::albumCoverMetadataRetrievedAS(QNetworkReply *r)
{
    qDebug() << SB_DEBUG_INFO;
    bool matchFound=0;

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

                    if(currentID.fuzzyMatch(pm)==1)
                    {
                        QNetworkAccessManager* n=new QNetworkAccessManager(this);
                        connect(n, SIGNAL(finished(QNetworkReply *)),
                                this, SLOT(imagedataRetrieved(QNetworkReply*)));

                        QUrl url(URL);
                        n->get(QNetworkRequest(url));
                        matchFound=1;
                        return;
                    }
                }
            }
        }
    }
}

void
ExternalData::imagedataRetrieved(QNetworkReply *r)
{
    qDebug() << SB_DEBUG_INFO;
    bool loaded=0;

    if(r->error()==QNetworkReply::NoError)
    {
        QByteArray a=r->readAll();
        if(a.count()>0)
        {
            QPixmap image;
            loaded=1;

            //	Store in cache
            image.loadFromData(a);
            storeInCache(&a);
            qDebug() << SB_DEBUG_INFO;
            emit imageDataReady(image);
            qDebug() << SB_DEBUG_INFO;
        }
    }
}

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

            QDomElement de=doc.documentElement();

            QDomNode n=de.firstChild();
            while(!n.isNull() && matchFound==0)
            {
                QDomElement e = n.toElement();
                if(!e.isNull())
                {
                    QDomNode n=e.firstChild();
                    while(!n.isNull() && matchFound==0)
                    {
                        QString MBID;

                        QDomElement e = n.toElement();
                        if(!e.isNull())
                        {
                            MBID=e.attribute("id");

                            QDomNode n=e.firstChild();
                            while(!n.isNull() && matchFound==0)
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
                                        else
                                        {
                                            qDebug() << SB_DEBUG_INFO
                                                     << "not a match:" << title
                                                     << "vs"
                                                     << Common::removeNonAlphanumeric(e.text().toLower());
                                        }
                                    }
                                }
                                n = n.nextSibling();
                            }
                        }
                        n = n.nextSibling();
                    }
                }
                n = n.nextSibling();
            }
        }
    }
    if(matchFound==1)
    {
        //	Recall now that sb_mbid is loaded
        qDebug() << SB_DEBUG_INFO << currentID << currentID.sb_mbid;
        emit updatePerformerMBID(currentID);
        loadPerformerDataMB();
    }
    qDebug() << SB_DEBUG_INFO << "matchFound=" << matchFound;
}

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
            QString s=QString(a.data());

            QDomDocument doc;
            QString errorMsg;
            int errorLine;
            int errorColumn;
            doc.setContent(a,0,&errorMsg,&errorLine,&errorColumn);

            QDomElement de=doc.documentElement();

            QDomNode n=de.firstChild();
            while(!n.isNull() && matchFound==0)
            {
                QDomElement e = n.toElement();

                if(!e.isNull() && e.tagName()=="images")
                {
                    QDomNode n=e.firstChild();

                    while(!n.isNull() && matchFound==0)
                    {
                        QDomElement e = n.toElement();

                        if(!e.isNull())
                        {
                            QDomNode n=e.firstChild();
                            while(!n.isNull() && matchFound==0)
                            {
                                QDomElement e = n.toElement();

                                if(e.tagName()=="url")
                                {
                                    url=e.text();
                                }
                                else if(!e.isNull())
                                {
                                    QDomNode n=e.firstChild();
                                    while(!n.isNull() && matchFound==0)
                                    {
                                        QDomElement e = n.toElement();

                                        if(e.tagName()=="attribution" && e.text()!="myspace")
                                        {
                                            QNetworkAccessManager* m=new QNetworkAccessManager(this);
                                            connect(m, SIGNAL(finished(QNetworkReply *)),
                                                    this, SLOT(imagedataRetrieved(QNetworkReply*)));

                                            m->get(QNetworkRequest(QUrl(url)));
                                            return;
                                        }
                                        n = n.nextSibling();
                                    }
                                }
                                n = n.nextSibling();
                            }
                        }
                        n = n.nextSibling();
                    }
                }
                n = n.nextSibling();
            }
        }
    }
    qDebug() << SB_DEBUG_INFO << "No image data available";
}

void
ExternalData::performerURLDataRetrievedMB(QNetworkReply *r)
{
    bool matchFound=0;

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

            QDomElement de=doc.documentElement();

            QDomNode n=de.firstChild();
            while(!n.isNull() && matchFound==0)
            {
                QDomElement e = n.toElement();

                if(!e.isNull())
                {
                    QDomNode n=e.firstChild();

                    while(!n.isNull() && matchFound==0)
                    {
                        QString MBID;

                        QDomElement e = n.toElement();

                        if(!e.isNull())
                        {

                            QDomNode n=e.firstChild();
                            while(!n.isNull() && matchFound==0)
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

                                n = n.nextSibling();
                            }
                        }
                        n = n.nextSibling();
                    }
                }
                n = n.nextSibling();
            }
        }
    }
    qDebug() << SB_DEBUG_INFO;
}

///	PRIVATE
///
QString
ExternalData::getCachePath() const
{
    QString p=QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    p.replace("!","");
    QDir d;
    d.mkpath(p);
    QString f=QString("%1/%2.%3").arg(p).arg(currentID.getType()).arg(currentID.sb_item_id);
    return f;
}

void
ExternalData::loadAlbumCoverAS()
{
    qDebug() << SB_DEBUG_INFO;

    //	Clear image
    QPixmap p;

    //	Revisit entire caching method
    if(loadImageFromCache(p))
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
}

void
ExternalData::loadPerformerDataMB()
{
    if(currentID.sb_mbid.length()==0)
    {
        //	Unknown MBID -- look it up and store.
        QNetworkAccessManager* m=new QNetworkAccessManager(this);
        connect(m, SIGNAL(finished(QNetworkReply *)),
                this, SLOT(performerMBIDRetrieved(QNetworkReply*)));

        QString URL=QString("http://musicbrainz.org/ws/2/artist/?query=artist:%1").arg(currentID.performerName);
        qDebug() << SB_DEBUG_INFO << URL;
        m->get(QNetworkRequest(QUrl(URL)));
    }
    else
    {
        QString URL;

        //	Get wikipedia and homepage urls from musicbrainz
        QNetworkAccessManager* mb=new QNetworkAccessManager(this);
        connect(mb, SIGNAL(finished(QNetworkReply *)),
                this, SLOT(performerURLDataRetrievedMB(QNetworkReply*)));

        URL=QString("http://musicbrainz.org/ws/2/artist/%1?inc=url-rels").arg(currentID.sb_mbid);
        qDebug() << SB_DEBUG_INFO << URL;
        mb->get(QNetworkRequest(QUrl(URL)));

        QPixmap p;
        if(loadImageFromCache(p))
        {
            emit imageDataReady(p);
        }
        else
        {
            //	Get performer image from echonest
            QNetworkAccessManager* en=new QNetworkAccessManager(this);
            connect(en, SIGNAL(finished(QNetworkReply *)),
                    this, SLOT(performerImageRetrievedEN(QNetworkReply *)));

            //get news items from artist
            //URL=QString("http://developer.echonest.com/api/v4/artist/news?api_key=BYNRSUS9LPOC2NYUI&id=musicbrainz:artist:%1&format=xml").arg(currentID.sb_mbid);

            URL=QString("http://developer.echonest.com/api/v4/artist/images?api_key=BYNRSUS9LPOC2NYUI&id=musicbrainz:artist:%1&format=xml").arg(currentID.sb_mbid);
            qDebug() << SB_DEBUG_INFO << URL;
            en->get(QNetworkRequest(QUrl(URL)));
        }
    }
}

void
ExternalData::init()
{
    currentID=SBID();
    performerWikipediaPageRetrieved=0;
    performerHomepageRetrieved=0;
}

///
/// \brief ExternalData::setImageFromCache
/// \param id
/// \param l
/// \return 1 if image is successfully loaded from cache
///
bool
ExternalData::loadImageFromCache(QPixmap& p)
{
    QString fn=getCachePath();
    QFile f(fn);

    if(f.open(QIODevice::ReadOnly))
    {
        int len=f.size();
        char* mem=(char *)malloc(len);
        QDataStream s(&f);
        int n=s.readRawData(mem,len);
        qDebug() << SB_DEBUG_INFO << "bytes read:" << n;

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
ExternalData::storeInCache(QByteArray *a) const
{
    QString fn=getCachePath();
    qDebug() << SB_DEBUG_INFO << fn << "ln(a)=" << a->length();
    QFile f(fn);
    if(f.open(QIODevice::WriteOnly))
    {
        QDataStream s(&f);
        int n=s.writeRawData(a->constData(),a->length());
        qDebug() << SB_DEBUG_INFO << "bytes written:" << n;
        f.close();
    }
}
