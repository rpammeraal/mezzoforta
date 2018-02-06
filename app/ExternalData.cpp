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
    _currentKey=key;

    //	1.	Artwork
    QPixmap p;
    if(_loadImageFromCache(p,_currentKey))
    {
        _artworkRetrievedFlag=1;
        qDebug() << SB_DEBUG_INFO << "emit imageDataReady";
        emit imageDataReady(p);
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
    qDebug() << SB_DEBUG_INFO;
    _currentKey=key;
    SBIDPtr ptr=CacheManager::get(key);
    SB_RETURN_VOID_IF_NULL(ptr);

    //	1.	Performer home page
    if(ptr->url().length()>0)
    {
    qDebug() << SB_DEBUG_INFO;
        _performerHomepageRetrievedFlag=1;
        emit performerHomePageAvailable(ptr->url());
    }

    //	2.	Wikipedia page
    if(ptr->wiki().length()>0)
    {
    qDebug() << SB_DEBUG_INFO;
        _wikipediaURLRetrievedFlag=1;
        emit performerWikipediaPageAvailable(ptr->wiki());
    }

    qDebug() << SB_DEBUG_INFO;
    //	3.	Artwork
    QPixmap p;
    if(_loadImageFromCache(p,key))
    {
        qDebug() << SB_DEBUG_INFO;
        _artworkRetrievedFlag=1;
        qDebug() << SB_DEBUG_INFO << "emit imageDataReady";
        emit imageDataReady(p);
    }
    qDebug() << SB_DEBUG_INFO;
    _getMBIDAndMore();
}

void
ExternalData::loadSongData(SBKey key)
{
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
    p.replace("!","");
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

///
/// \brief ExternalData::setImageFromCache
/// \return 1 if image is successfully loaded from cache
///
bool
ExternalData::_loadImageFromCache(QPixmap& p,SBKey key)
{
    qDebug() << SB_DEBUG_INFO;
    QString fn=getCachePath(key);
    qDebug() << SB_DEBUG_INFO << key << fn;
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
            qDebug() << SB_DEBUG_INFO;
            return 1;
        }
        free(mem);
    }
    qDebug() << SB_DEBUG_INFO;
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
                    SBIDAlbumPtr albumPtr;
                    QString key;
                    QString value;
                    QString urlString;

                    QDomNode     level2node    =level1nodelist.at(i);
                    QDomNodeList level2nodelist=level2node.childNodes();

                    QString albumTitle;
                    QString albumPerformerName;

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
                            albumTitle=value;
                        }
                        else if(key=="artist")
                        {
                            albumPerformerName=value;
                        }
                        else if(key=="image")
                        {
                            urlString=value;
                        }
                    }

                    if(_currentKey.itemType()==SBKey::Album)
                    {
                        SBIDAlbumPtr aPtr=SBIDAlbum::retrieveAlbum(_currentKey);
                        SB_RETURN_VOID_IF_NULL(aPtr);

                        if(Common::simplified(aPtr->albumTitle())==Common::simplified(albumTitle) &&
                            Common::simplified(aPtr->albumPerformerName())==Common::simplified(albumPerformerName))
                        {
                            QNetworkAccessManager* n=new QNetworkAccessManager(this);
                            connect(n, SIGNAL(finished(QNetworkReply *)),
                                    this, SLOT(handleImageDataNetwork(QNetworkReply*)));

                            n->get(QNetworkRequest(QUrl(urlString)));
                            return;
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
ExternalData::handleAlbumURLDataFromMB(QNetworkReply *r)
{
    if(_currentKey.itemType()==SBKey::Album)
    {
        SBIDAlbumPtr aPtr=SBIDAlbum::retrieveAlbum(_currentKey);
        SB_RETURN_VOID_IF_NULL(aPtr);

        QString matchAlbumName=Common::removeNonAlphanumeric(aPtr->albumTitle()).toLower();
        QString foundAlbumName;
        _allReviews.clear();

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

                                            if(e.attribute("type")=="wikipedia" && matchAlbumName==foundAlbumName && _wikipediaURLRetrievedFlag==0)
                                            {
                                                QString urlString=e.text() + "&printable=yes";
                                                urlString.replace("/wiki/","/w/index.php?title=");
                                                _wikipediaURLRetrievedFlag=1;
                                                emit albumWikipediaPageAvailable(urlString);
                                            }
                                            else if(e.attribute("type")=="review" && matchAlbumName==foundAlbumName)
                                            {
                                                _allReviews.append(e.text());
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
        if(_allReviews.count()>0)
        {
            emit albumReviewsAvailable(_allReviews);
        }
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
                qDebug() << SB_DEBUG_INFO << "store in cache";
                image.loadFromData(a);
                _storeInCache(&a);
                qDebug() << SB_DEBUG_INFO << "emit imageDataReady";
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
    SBIDPtr ptr=CacheManager::get(_currentKey);
    SB_RETURN_VOID_IF_NULL(ptr);

    QString title=Common::removeNonAlphanumeric(ptr->commonPerformerName().toLower());
    _performerMBIDRetrievedFlag=1;

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
                                            ptr->setMBID(MBID);
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
        emit updatePerformerMBID(_currentKey);
        _getMBIDAndMore();
    }
}

void
ExternalData::handlePerformerImageURLFromWC(QNetworkReply *r)
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

                if(!e.isNull() && e.tagName()=="query")
                {
                    for(QDomNode n=e.firstChild();!n.isNull() && matchFound==0;n = n.nextSibling())
                    {
                        QDomElement e = n.toElement();

                        if(!e.isNull() && e.tagName()=="pages")
                        {
                            for(QDomNode n=e.firstChild();!n.isNull() && matchFound==0;n = n.nextSibling())
                            {
                                QDomElement e = n.toElement();

                                if(!e.isNull() && e.tagName()=="page")
                                {
                                    for(QDomNode n=e.firstChild();!n.isNull() && matchFound==0;n = n.nextSibling())
                                    {
                                        QDomElement e = n.toElement();

                                        if(!e.isNull() && e.tagName()=="imageinfo")
                                        {
                                            for(QDomNode n=e.firstChild();!n.isNull() && matchFound==0;n = n.nextSibling())
                                            {
                                                QDomElement e = n.toElement();

                                                if(!e.isNull() && e.tagName()=="ii")
                                                {
                                                    //	Found imagio!
                                                    QString urlString=e.attribute("thumburl");

                                                    QNetworkAccessManager* m=new QNetworkAccessManager(this);
                                                    connect(m, SIGNAL(finished(QNetworkReply *)),
                                                            this, SLOT(handleImageDataNetwork(QNetworkReply*)));

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

//                        if(!e.isNull())
//                        {
//                            for(QDomNode n=e.firstChild();!n.isNull() && matchFound==0;n = n.nextSibling())
//                            {
//                                QDomElement e = n.toElement();

//                                if(e.tagName()=="url")
//                                {
//                                    urlString=e.text();
//                                }
//                                else if(!e.isNull())
//                                {
//                                    for(QDomNode n=e.firstChild();!n.isNull() && matchFound==0;n = n.nextSibling())
//                                    {
//                                        QDomElement e = n.toElement();

//                                        if(e.tagName()=="attribution" && e.text()!="myspace")
//                                        {
//                                            QNetworkAccessManager* m=new QNetworkAccessManager(this);
//                                            connect(m, SIGNAL(finished(QNetworkReply *)),
//                                                    this, SLOT(handleImageDataNetwork(QNetworkReply*)));

//                                            m->get(QNetworkRequest(QUrl(urlString)));
//                                            return;
//                                        }
//                                    }
//                                }
//                            }
//                        }

void
ExternalData::handlePerformerNewsURLFromEN(QNetworkReply *r)
{
    bool matchFound=0;
    _allNewsItems.clear();

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
                            _allNewsItems.append(item);
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
    if(_allNewsItems.count()>0)
    {
        emit performerNewsAvailable(_allNewsItems);
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

            for(QDomNode n=e.firstChild();!n.isNull() && (_wikipediaURLRetrievedFlag==0 || _performerHomepageRetrievedFlag==0);n = n.nextSibling())
            {
                QDomElement e = n.toElement();

                if(!e.isNull())
                {
                    for(QDomNode n=e.firstChild();!n.isNull() && (_wikipediaURLRetrievedFlag==0 || _performerHomepageRetrievedFlag==0);n = n.nextSibling())
                    {
                        QString MBID;

                        QDomElement e = n.toElement();

                        if(!e.isNull())
                        {
                            for(QDomNode n=e.firstChild();!n.isNull() && (_wikipediaURLRetrievedFlag==0 || _performerHomepageRetrievedFlag==0);n = n.nextSibling())
                            {
                                QDomElement e = n.toElement();

                                if(e.attribute("type")=="wikipedia" && _wikipediaURLRetrievedFlag==0)
                                {
                                    QString urlString=e.text() + "&printable=yes";
                                    urlString.replace("/wiki/","/w/index.php?title=");
                                    _wikipediaURLRetrievedFlag=1;

                                    emit performerWikipediaPageAvailable(urlString);
                                }
                                else if(e.attribute("type")=="official homepage" && _performerHomepageRetrievedFlag==0)
                                {
                                    SBIDPerformerPtr pPtr=SBIDPerformer::retrievePerformer(_currentKey);
                                    SB_RETURN_VOID_IF_NULL(pPtr);

                                    pPtr->setURL(e.text());
                                    _performerHomepageRetrievedFlag=1;
                                    emit performerHomePageAvailable(pPtr->url());
                                    emit updatePerformerHomePage(_currentKey);
                                }
                                else if(e.attribute("type")=="image")
                                {
                                    QNetworkAccessManager* mb=new QNetworkAccessManager(this);
                                    connect(mb, SIGNAL(finished(QNetworkReply *)),
                                            this, SLOT(handlePerformerImageURLFromWC(QNetworkReply*)));

                                    QStringList parts=e.text().split(":");
                                    if(parts.count()==3)
                                    {
                                        QString imageFileName=parts[2];

                                        QString urlString=QString("https://commons.wikimedia.org/w/api.php?action=query&prop=imageinfo&iiprop=url&redirects&format=xml&iiurlwidth=250&titles=File:%1")
                                            .arg(imageFileName)
                                        ;
                                        mb->get(QNetworkRequest(QUrl(urlString)));
                                    }
                                    else
                                    {
                                        qDebug() << SB_DEBUG_ERROR << "parts.count!=3:" << parts.count();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        qDebug() << SB_DEBUG_ERROR
                 << r->errorString()
                 << r->error()
        ;
    }
}

void
ExternalData::handleSongMetaDataFromMB(QNetworkReply *r)
{
    if(_currentKey.itemType()==SBKey::Song)
    {
        SBIDSongPtr sPtr=SBIDSong::retrieveSong(_currentKey);
        SB_RETURN_VOID_IF_NULL(sPtr);

        QString matchSongName=Common::removeNonAlphanumeric(sPtr->songTitle()).toLower();
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
                                            _sendMusicBrainzQuery(mb,urlString);
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
                    _currentOffset+=MUSICBRAINZ_MAXNUM;
                    _getMBIDAndMore();
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

            for(QDomNode n=e.firstChild();!n.isNull() && (_songLyricsURLRetrievedFlag==0 || _wikipediaURLRetrievedFlag==0);n = n.nextSibling())
            {
                QDomElement e = n.toElement();

                if(!e.isNull())
                {
                    for(QDomNode n=e.firstChild();!n.isNull() && (_songLyricsURLRetrievedFlag==0 || _wikipediaURLRetrievedFlag==0);n = n.nextSibling())
                    {
                        QDomElement e = n.toElement();

                        if(!e.isNull())
                        {
                            for(QDomNode n=e.firstChild();!n.isNull() && (_songLyricsURLRetrievedFlag==0 || _wikipediaURLRetrievedFlag==0);n = n.nextSibling())
                            {
                                QDomElement e = n.toElement();

                                type=e.attribute("type");
                                if(!e.isNull())
                                {
                                    for(QDomNode n=e.firstChild();!n.isNull() && (_songLyricsURLRetrievedFlag==0 || _wikipediaURLRetrievedFlag==0);n = n.nextSibling())
                                    {
                                        QDomElement e = n.toElement();

                                        if(type=="lyrics" && _songLyricsURLRetrievedFlag==0)
                                        {
                                            _songLyricsURLRetrievedFlag=1;
                                            emit songLyricsURLAvailable(e.text());
                                        }
                                        else if(type=="wikipedia" && _wikipediaURLRetrievedFlag==0)
                                        {
                                            QString urlString=e.text() + "&printable=yes";
                                            urlString.replace("/wiki/","/w/index.php?title=");
                                            _wikipediaURLRetrievedFlag=1;
                                            emit songWikipediaPageAvailable(urlString);
                                        }
                                        else
                                        {
                                            qDebug() << SB_DEBUG_WARNING << "Other URL:type=" << type << e.text();
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
ExternalData::_loadAlbumCoverAS()
{
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
    QNetworkAccessManager* en;
    QString urlString;
    QNetworkAccessManager* mb;

    SBIDPtr ptr=CacheManager::get(_currentKey);
    SB_RETURN_VOID_IF_NULL(ptr);

    if(ptr->MBID().length()==0)
    {
        if(_performerMBIDRetrievedFlag==1)
        {
            //	Unable to retrieve performerMBID, bail out
            qDebug() << SB_DEBUG_WARNING << "No performerMBID available";
            return;
        }

        //	Unknown MBID -- look it up and store.
        QNetworkAccessManager* m=new QNetworkAccessManager(this);
        connect(m, SIGNAL(finished(QNetworkReply *)),
                this, SLOT(handleMBIDNetwork(QNetworkReply*)));

        QString urlString=QString("http://musicbrainz.org/ws/2/artist/?query=artist:%1")
            .arg(ptr->commonPerformerName())
        ;
        _sendMusicBrainzQuery(m,urlString);
    }
    else
    {
        if(ptr->itemType()==SBKey::Performer)
        {
            //	1.	Wikipedia, performer home page
            if(_wikipediaURLRetrievedFlag==0 || _performerHomepageRetrievedFlag==0)
            {
                mb=new QNetworkAccessManager(this);
                connect(mb, SIGNAL(finished(QNetworkReply *)),
                        this, SLOT(handlePerformerURLFromMB(QNetworkReply*)));

                urlString=QString("http://musicbrainz.org/ws/2/artist/%1?inc=url-rels")
                    .arg(ptr->MBID())
                ;
                _sendMusicBrainzQuery(mb,urlString);
            }
            else
            {
                qDebug() << SB_DEBUG_WARNING << "Looks like we already have wiki and home page for performer";
            }

            //	2.	Artwork
            //	Echnonest was used -- now using wikimedia commons


            //	3.	Get news items from echonest
            en=new QNetworkAccessManager(this);
            connect(en, SIGNAL(finished(QNetworkReply *)),
                    this, SLOT(handlePerformerNewsURLFromEN(QNetworkReply*)));

            urlString=QString("http://developer.echonest.com/api/v4/artist/news?api_key=BYNRSUS9LPOC2NYUI&id=musicbrainz:artist:%1&format=xml")
                .arg(ptr->MBID())
            ;
            en->get(QNetworkRequest(QUrl(urlString)));
        }
        else if(_currentKey.itemType()==SBKey::Album)
        {
            SBIDAlbumPtr aPtr=SBIDAlbum::retrieveAlbum(_currentKey);
            SB_RETURN_VOID_IF_NULL(aPtr);

            //	1.	Artwork
            if(_artworkRetrievedFlag==0)
            {
                QNetworkAccessManager* m=new QNetworkAccessManager(this);
                connect(m, SIGNAL(finished(QNetworkReply *)),
                        this, SLOT(handleAlbumImageURLFromAS(QNetworkReply*)));

                QString urlString=QString("http://ws.audioscrobbler.com/2.0/?method=album.search&limit=9999&api_key=5dacbfb3b24d365bcd43050c6149a40d&album=%1").
                        arg(aPtr->albumTitle());
                m->get(QNetworkRequest(QUrl(urlString)));
            }

            //	2.	Wikipedia page
            if(_wikipediaURLRetrievedFlag==0)
            {
                //	Get urls for given album
                QNetworkAccessManager* mb=new QNetworkAccessManager(this);
                connect(mb, SIGNAL(finished(QNetworkReply *)),
                        this, SLOT(handleAlbumURLDataFromMB(QNetworkReply*)));

                QString performerMBID=aPtr->albumPerformerMBID();
                urlString=QString("https://musicbrainz.org/ws/2/release-group?artist=%1&inc=url-rels&offset=0&limit=%2")
                    .arg(performerMBID)
                    .arg(MUSICBRAINZ_MAXNUM)
                ;
                _sendMusicBrainzQuery(mb,urlString);
            }
            else
            {
                qDebug() << SB_DEBUG_WARNING << "Looks like we already have wiki for album";
            }
        }
        else if(_currentKey.itemType()==SBKey::Song)
        {
            //	CWIP:
            //	Songlyrics are only downloaded if wikipedia page is not known.
            //	Wikipedia page for song is neither stored nor cached.
            //	Ergo: both will always be retrieved. Logical hack alert.

            SBIDSongPtr sPtr=SBIDSong::retrieveSong(_currentKey);
            SB_RETURN_VOID_IF_NULL(sPtr);

            //	1.	Wikipedia page
            if(_wikipediaURLRetrievedFlag==0)
            {
                //	Find meta data for song
                QNetworkAccessManager* mb=new QNetworkAccessManager(this);
                connect(mb, SIGNAL(finished(QNetworkReply *)),
                        this, SLOT(handleSongMetaDataFromMB(QNetworkReply*)));

                urlString=QString("http://musicbrainz.org/ws/2/work?artist=%1&offset=%2&limit=%3")
                    .arg(sPtr->MBID())
                    .arg(_currentOffset)
                    .arg(MUSICBRAINZ_MAXNUM)
                ;
                _sendMusicBrainzQuery(mb,urlString);
            }
            else
            {
                qDebug() << SB_DEBUG_WARNING << "Looks like we already have wiki for song";
            }
        }
    }
}

void
ExternalData::_sendMusicBrainzQuery(QNetworkAccessManager* mb,const QString &urlString)
{
    QNetworkRequest nr;
    nr.setUrl(QUrl(urlString));
    nr.setHeader(QNetworkRequest::UserAgentHeader,QVariant("MezzoForta! ( to@be.provided )"));
    mb->get(nr);

    //	CWIP store mb pointer somewhere so we can remove this later
}

void
ExternalData::_init()
{
    _currentOffset=0;
    _artworkRetrievedFlag=0;
    _performerMBIDRetrievedFlag=0;
    _performerHomepageRetrievedFlag=0;
    _songLyricsURLRetrievedFlag=0;
    _wikipediaURLRetrievedFlag=0;
}

void
ExternalData::_storeInCache(QByteArray *a) const
{
    QString fn=getCachePath(_currentKey);
    qDebug() << SB_DEBUG_INFO << fn;
    QFile f(fn);
    if(f.open(QIODevice::WriteOnly))
    {
        QDataStream s(&f);
        s.writeRawData(a->constData(),a->length());
        f.close();
    }
}
