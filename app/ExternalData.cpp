#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QLabel>
#include <QSemaphore>
#include <QStandardPaths>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QPixmap>
#include <QWebEngineView>

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

//	Main interface
void
ExternalData::loadAlbumData(const SBID &id)
{
    currentID=id;

    //	1.	Artwork
    QPixmap p;
    if(loadImageFromCache(p,currentID))
    {
        artworkRetrieved=1;
        emit imageDataReady(p);
    }

    //	2.	Wikipedia page
    if(id.wiki.length()>0)
    {
        wikipediaURLRetrieved=1;
        emit albumWikipediaPageAvailable(id.wiki);
    }
    getMBIDAndMore();
}

void
ExternalData::loadPerformerData(const SBID id)
{
    currentID=id;

    //	1.	Performer home page
    if(id.url.length()>0)
    {
        performerHomepageRetrieved=1;
        emit performerHomePageAvailable(id.url);
    }

    //	2.	Wikipedia page
    if(id.wiki.length()>0)
    {
        wikipediaURLRetrieved=1;
        emit performerWikipediaPageAvailable(id.wiki);
    }

    //	3.	Artwork
    QPixmap p;
    if(loadImageFromCache(p,id))
    {
        artworkRetrieved=1;
        emit imageDataReady(p);
    }

    getMBIDAndMore();
}

void
ExternalData::loadSongData(const SBID& id)
{
    currentID=id;

    //	1.	Wikipedia page
    if(id.wiki.length()>0)
    {
        wikipediaURLRetrieved=1;
        emit songWikipediaPageAvailable(id.wiki);
    }
    getMBIDAndMore();
}

//	Static methods
QString
ExternalData::getCachePath(const SBID& id)
{
    QString p=QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    p.replace("!","");
    QDir d;
    d.mkpath(p);
    QString f=QString("%1/%2.%3")
        .arg(p)
        .arg(id.getType())
        .arg(id.sb_item_id())
    ;
    return f;
}

///
/// \brief ExternalData::setImageFromCache
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

//	Slots
void
ExternalData::handleAlbumImageURLFromAS(QNetworkReply *r)
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
                    QString urlString;

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
                            urlString=value;
                        }
                    }

                    if(currentID.fuzzyMatch(pm)==1)
                    {
                        QNetworkAccessManager* n=new QNetworkAccessManager(this);
                        connect(n, SIGNAL(finished(QNetworkReply *)),
                                this, SLOT(handleImageDataNetwork(QNetworkReply*)));

                        qDebug() << SB_DEBUG_INFO << urlString;
                        n->get(QNetworkRequest(QUrl(urlString)));
                        return;
                    }
                }
            }
        }
    }
    else if(0)
    {
        QMessageBox messagebox;
        messagebox.setText("Error processing network request:");
        messagebox.setInformativeText(r->errorString());
        messagebox.exec();
    }
}

void
ExternalData::handleAlbumURLDataFromMB(QNetworkReply *r)
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

                                        if(e.attribute("type")=="wikipedia" && matchAlbumName==foundAlbumName && wikipediaURLRetrieved==0)
                                        {
                                            QString urlString=e.text() + "&printable=yes";
                                            urlString.replace("/wiki/","/w/index.php?title=");
                                            wikipediaURLRetrieved=1;
                                            emit albumWikipediaPageAvailable(urlString);
                                        }
                                        else if(e.attribute("type")=="review" && matchAlbumName==foundAlbumName)
                                        {
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
    else if(0)
    {
        QMessageBox messagebox;
        messagebox.setText("Error processing network request:");
        messagebox.setInformativeText(r->errorString());
        messagebox.exec();
    }
    if(allReviews.count()>0)
    {
        emit albumReviewsAvailable(allReviews);
    }
}

void
ExternalData::handleImageDataNetwork(QNetworkReply *r)
{
    if(r->error()==QNetworkReply::NoError)
    {
        if(r->open(QIODevice::ReadOnly))
        {
            QByteArray a=r->readAll();
            if(a.count()>0)
            {
                QPixmap image;

                //	Store in cache
                image.loadFromData(a);
                storeInCache(&a);
                emit imageDataReady(image);
            }
        }
    }
    else if(0)
    {
        QMessageBox messagebox;
        messagebox.setText("Error processing network request:");
        messagebox.setInformativeText(r->errorString());
        messagebox.exec();
    }
}

void
ExternalData::handleMBIDNetwork(QNetworkReply *r)
{
    bool matchFound=0;
    QString title=Common::removeNonAlphanumeric(currentID.performerName.toLower());
    performerMBIDRetrieved=1;

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
    else if(0)
    {
        QMessageBox messagebox;
        messagebox.setText("Error processing network request:");
        messagebox.setInformativeText(r->errorString());
        messagebox.exec();
    }
    if(matchFound==1)
    {
        //	Recall now that sb_mbid is loaded
        emit updatePerformerMBID(currentID);
        getMBIDAndMore();
    }
}

void
ExternalData::handlePerformerImageURLFromEN(QNetworkReply *r)
{
    bool matchFound=0;

    QString urlString;

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
                                    urlString=e.text();
                                }
                                else if(!e.isNull())
                                {
                                    for(QDomNode n=e.firstChild();!n.isNull() && matchFound==0;n = n.nextSibling())
                                    {
                                        QDomElement e = n.toElement();

                                        if(e.tagName()=="attribution" && e.text()!="myspace")
                                        {
                                            QNetworkAccessManager* m=new QNetworkAccessManager(this);
                                            connect(m, SIGNAL(finished(QNetworkReply *)),
                                                    this, SLOT(handleImageDataNetwork(QNetworkReply*)));

                                            qDebug() << SB_DEBUG_INFO << urlString;
                                            m->get(QNetworkRequest(QUrl(urlString)));
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
    else if(0)
    {
        QMessageBox messagebox;
        messagebox.setText("Error processing network request:");
        messagebox.setInformativeText(r->errorString());
        messagebox.exec();
    }
}

void
ExternalData::handlePerformerNewsURLFromEN(QNetworkReply *r)
{
    bool matchFound=0;
    allNewsItems.clear();

    QString urlString;

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
    else if(0)
    {
        QMessageBox messagebox;
        messagebox.setText("Error processing network request:");
        messagebox.setInformativeText(r->errorString());
        messagebox.exec();
    }
    if(allNewsItems.count()>0)
    {
        emit performerNewsAvailable(allNewsItems);
    }
}

void
ExternalData::handlePerformerURLFromMB(QNetworkReply *r)
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

            for(QDomNode n=e.firstChild();!n.isNull() && (wikipediaURLRetrieved==0 || performerHomepageRetrieved==0);n = n.nextSibling())
            {
                QDomElement e = n.toElement();

                if(!e.isNull())
                {
                    for(QDomNode n=e.firstChild();!n.isNull() && (wikipediaURLRetrieved==0 || performerHomepageRetrieved==0);n = n.nextSibling())
                    {
                        QString MBID;

                        QDomElement e = n.toElement();

                        if(!e.isNull())
                        {
                            for(QDomNode n=e.firstChild();!n.isNull() && (wikipediaURLRetrieved==0 || performerHomepageRetrieved==0);n = n.nextSibling())
                            {
                                QDomElement e = n.toElement();

                                if(e.attribute("type")=="wikipedia" && wikipediaURLRetrieved==0)
                                {
                                    QString urlString=e.text() + "&printable=yes";
                                    urlString.replace("/wiki/","/w/index.php?title=");
                                    wikipediaURLRetrieved=1;

                                    emit performerWikipediaPageAvailable(urlString);
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
    else if(0)
    {
        QMessageBox messagebox;
        messagebox.setText("Error processing network request:");
        messagebox.setInformativeText(r->errorString());
        messagebox.exec();
    }
}

void
ExternalData::handleSongMetaDataFromMB(QNetworkReply *r)
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
                                                this, SLOT(handleSongURLFromMB(QNetworkReply*)));

                                        QString urlString=QString("http://musicbrainz.org/ws/2/work/%1?inc=url-rels")
                                            .arg(mbid)
                                        ;
                                        qDebug() << SB_DEBUG_INFO << urlString;
                                        mb->get(QNetworkRequest(QUrl(urlString)));
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
                getMBIDAndMore();
            }
        }
    }
    else if(0)
    {
        QMessageBox messagebox;
        messagebox.setText("Error processing network request:");
        messagebox.setInformativeText(r->errorString());
        messagebox.exec();
    }
}

void
ExternalData::handleSongURLFromMB(QNetworkReply *r)
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

            for(QDomNode n=e.firstChild();!n.isNull() && (songLyricsURLRetrieved==0 || wikipediaURLRetrieved==0);n = n.nextSibling())
            {
                QDomElement e = n.toElement();

                if(!e.isNull())
                {
                    for(QDomNode n=e.firstChild();!n.isNull() && (songLyricsURLRetrieved==0 || wikipediaURLRetrieved==0);n = n.nextSibling())
                    {
                        QDomElement e = n.toElement();

                        if(!e.isNull())
                        {
                            for(QDomNode n=e.firstChild();!n.isNull() && (songLyricsURLRetrieved==0 || wikipediaURLRetrieved==0);n = n.nextSibling())
                            {
                                QDomElement e = n.toElement();

                                type=e.attribute("type");
                                if(!e.isNull())
                                {
                                    for(QDomNode n=e.firstChild();!n.isNull() && (songLyricsURLRetrieved==0 || wikipediaURLRetrieved==0);n = n.nextSibling())
                                    {
                                        QDomElement e = n.toElement();

                                        if(type=="lyrics" && songLyricsURLRetrieved==0)
                                        {
                                            songLyricsURLRetrieved=1;
                                            emit songLyricsURLAvailable(e.text());
                                        }
                                        else if(type=="wikipedia" && wikipediaURLRetrieved==0)
                                        {
                                            QString urlString=e.text() + "&printable=yes";
                                            urlString.replace("/wiki/","/w/index.php?title=");
                                            wikipediaURLRetrieved=1;
                                            emit songWikipediaPageAvailable(urlString);
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
    else if(0)
    {
        QMessageBox messagebox;
        messagebox.setText("Error processing network request:");
        messagebox.setInformativeText(r->errorString());
        messagebox.exec();
    }
}

///	PRIVATE
///
void
ExternalData::loadAlbumCoverAS()
{
}

///
/// \brief ExternalData::getMBIDAndMore
///
/// Due to the asynchronous processing of network requets, the purpose of this function is to:
/// -	retrieve the MBID for the performer, if this is not already retrieved from the database.
/// 	This is another asynchronous call, the receiving end should call getMBIDAndMore() in order
/// 	to continue loading whatever is needed for this.currentID.
/// -	load whatever data that is relevant for this.currentID. This function is called in each
/// 	of the main interface functions to load whatever is remaining (as some items are cached or
/// 	or stored in the database).
void
ExternalData::getMBIDAndMore()
{
    QNetworkAccessManager* en;
    QString urlString;
    QNetworkAccessManager* mb;

    if(currentID.sb_mbid.length()==0)
    {
        if(performerMBIDRetrieved==1)
        {
            //	Unable to retrieve performerMBID, bail out
            qDebug() << SB_DEBUG_INFO << "No performerMBID available";
            return;
        }

        //	Unknown MBID -- look it up and store.
        QNetworkAccessManager* m=new QNetworkAccessManager(this);
        connect(m, SIGNAL(finished(QNetworkReply *)),
                this, SLOT(handleMBIDNetwork(QNetworkReply*)));

        QString urlString=QString("http://musicbrainz.org/ws/2/artist/?query=artist:%1")
            .arg(currentID.performerName)
        ;
        qDebug() << SB_DEBUG_INFO << urlString;
        m->get(QNetworkRequest(QUrl(urlString)));
    }
    else
    {
        if(currentID.sb_item_type()==SBID::sb_type_performer)
        {
            //	1.	Wikipedia, performer home page
            if(wikipediaURLRetrieved==0 || performerHomepageRetrieved==0)
            {
                mb=new QNetworkAccessManager(this);
                connect(mb, SIGNAL(finished(QNetworkReply *)),
                        this, SLOT(handlePerformerURLFromMB(QNetworkReply*)));

                urlString=QString("http://musicbrainz.org/ws/2/artist/%1?inc=url-rels")
                    .arg(currentID.sb_mbid)
                ;
                qDebug() << SB_DEBUG_INFO << urlString;
                mb->get(QNetworkRequest(QUrl(urlString)));
            }
            else
            {
                qDebug() << SB_DEBUG_INFO << "Looks like we already have wiki and home page for performer";
            }

            //	2.	Artwork
            if(artworkRetrieved==0)
            {
                //	Get performer image from echonest
                en=new QNetworkAccessManager(this);
                connect(en, SIGNAL(finished(QNetworkReply *)),
                        this, SLOT(handlePerformerImageURLFromEN(QNetworkReply *)));

                urlString=QString("http://developer.echonest.com/api/v4/artist/images?api_key=BYNRSUS9LPOC2NYUI&id=musicbrainz:artist:%1&format=xml")
                    .arg(currentID.sb_mbid)
                ;
                qDebug() << SB_DEBUG_INFO << urlString;
                en->get(QNetworkRequest(QUrl(urlString)));
            }

            //	3.	Get news items from echonest
            en=new QNetworkAccessManager(this);
            connect(en, SIGNAL(finished(QNetworkReply *)),
                    this, SLOT(handlePerformerNewsURLFromEN(QNetworkReply*)));

            urlString=QString("http://developer.echonest.com/api/v4/artist/news?api_key=BYNRSUS9LPOC2NYUI&id=musicbrainz:artist:%1&format=xml")
                .arg(currentID.sb_mbid)
            ;
            qDebug() << SB_DEBUG_INFO << urlString;
            en->get(QNetworkRequest(QUrl(urlString)));
        }
        else if(currentID.sb_item_type()==SBID::sb_type_album)
        {
            //	1.	Artwork
            if(artworkRetrieved==0)
            {
                QNetworkAccessManager* m=new QNetworkAccessManager(this);
                connect(m, SIGNAL(finished(QNetworkReply *)),
                        this, SLOT(handleAlbumImageURLFromAS(QNetworkReply*)));

                QString urlString=QString("http://ws.audioscrobbler.com/2.0/?method=album.search&limit=9999&api_key=5dacbfb3b24d365bcd43050c6149a40d&album=%1").
                        arg(currentID.albumTitle);
                qDebug() << SB_DEBUG_INFO << urlString;
                m->get(QNetworkRequest(QUrl(urlString)));

            }

            //	2.	Wikipedia page
            if(wikipediaURLRetrieved==0)
            {
                //	Get urls for given album
                QNetworkAccessManager* mb=new QNetworkAccessManager(this);
                connect(mb, SIGNAL(finished(QNetworkReply *)),
                        this, SLOT(handleAlbumURLDataFromMB(QNetworkReply*)));

                urlString=QString("https://musicbrainz.org/ws/2/release-group?artist=%1&inc=url-rels&offset=0&limit=%2")
                    .arg(currentID.sb_mbid)
                    .arg(MUSICBRAINZ_MAXNUM)
                ;
                qDebug() << SB_DEBUG_INFO << urlString;
                mb->get(QNetworkRequest(QUrl(urlString)));
            }
            else
            {
                qDebug() << SB_DEBUG_INFO << "Looks like we already have wiki for album";
            }
        }
        else if(currentID.sb_item_type()==SBID::sb_type_song)
        {
            //	CWIP:
            //	Songlyrics are only downloaded if wikipedia page is not known.
            //	Wikipedia page for song is neither stored nor cached.
            //	Ergo: both will always be retrieved. Logical hack alert.

            //	1.	Wikipedia page
            if(wikipediaURLRetrieved==0)
            {
                //	Find meta data for song
                QNetworkAccessManager* mb=new QNetworkAccessManager(this);
                connect(mb, SIGNAL(finished(QNetworkReply *)),
                        this, SLOT(handleSongMetaDataFromMB(QNetworkReply*)));

                urlString=QString("http://musicbrainz.org/ws/2/work?artist=%1&offset=%2&limit=%3")
                    .arg(currentID.sb_mbid)
                    .arg(currentOffset)
                    .arg(MUSICBRAINZ_MAXNUM)
                ;
                qDebug() << SB_DEBUG_INFO << urlString;
                mb->get(QNetworkRequest(QUrl(urlString)));
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
    artworkRetrieved=0;
    performerMBIDRetrieved=0;
    performerHomepageRetrieved=0;
    songLyricsURLRetrieved=0;
    wikipediaURLRetrieved=0;
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
