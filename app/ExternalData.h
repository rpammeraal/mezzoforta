#ifndef EXTERNALDATA_H
#define EXTERNALDATA_H

#include <QObject>
#include <QPixmap>

#include "SBID.h"

class QNetworkReply;

static const int MUSICBRAINZ_MAXNUM=100;

struct NewsItem
{
    QString url;
    QString name;
    QString summary;
};

class ExternalData : public QObject
{
    Q_OBJECT
public:
    explicit ExternalData(QObject *parent = 0);
    ~ExternalData();

    //	Main interface
    void loadAlbumData(const SBID& id);
    void loadPerformerData(const SBID id);
    void loadSongData(const SBID& id);

    //	Static methods
    QString static getCachePath(const SBID& id);
    static bool loadImageFromCache(QPixmap& p,const SBID& id);

signals:
    void albumWikipediaPageAvailable(const QString& url);
    void albumReviewsAvailable(const QList<QString>& allReviews);
    void imageDataReady(const QPixmap& p);
    void performerHomePageAvailable(const QString& url);
    void performerNewsAvailable(const QList<NewsItem>& news);
    void performerWikipediaPageAvailable(const QString& url);
    void songLyricsURLAvailable(const QString& url);
    void songWikipediaPageAvailable(const QString& url);
    void updatePerformerMBID(const SBID& id);
    void updatePerformerHomePage(const SBID& id);

public slots:
    void handleAlbumImageURLFromAS(QNetworkReply *r);
    void handleAlbumURLDataFromMB(QNetworkReply* r);
    void handleImageDataNetwork(QNetworkReply* r);
    void handleMBIDNetwork(QNetworkReply* r);
    void handlePerformerImageURLFromEN(QNetworkReply* r);
    void handlePerformerNewsURLFromEN(QNetworkReply* r);
    void handlePerformerURLFromMB(QNetworkReply* r);
    void handleSongMetaDataFromMB(QNetworkReply* r);
    void handleSongURLFromMB(QNetworkReply* r);

private:
    bool artworkRetrieved;          	//	album performer
    bool performerHomepageRetrieved;	//	      performer
    bool songLyricsURLRetrieved;        //	                song (always retrieved, not stored)
    bool wikipediaURLRetrieved;         // 	album performer song

    int currentOffset;
    SBID currentID;
    bool performerMBIDRetrieved;	//	set if call to retrieve MBID already has been made.

    QList<NewsItem> allNewsItems;
    QList<QString> allReviews;

    void init();
    void loadAlbumCoverAS();
    void getMBIDAndMore();
    void storeInCache(QByteArray* a) const;
};

#endif // EXTERNALDATA_H
