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

    QString static getCachePath(const SBID& id);
    void loadAlbumData(const SBID& id);
    static bool loadImageFromCache(QPixmap& p,const SBID& id);
    void loadPerformerData(const SBID id);
    void loadSongData(const SBID& id);

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
    void albumCoverMetadataRetrievedAS(QNetworkReply *r);
    void albumURLDataRetrievedMB(QNetworkReply* r);
    void imagedataRetrieved(QNetworkReply* r);
    void performerMBIDRetrieved(QNetworkReply* r);
    void performerImageRetrievedEN(QNetworkReply* r);
    void performerNewsRetrievedEN(QNetworkReply* r);
    void performerURLDataRetrievedMB(QNetworkReply* r);
    void songMetaDataRetrievedMB(QNetworkReply* r);
    void songURLDataRetrievedMB(QNetworkReply* r);

private:
    int currentOffset;
    SBID currentID;
    bool albumWikipediaPageRetrieved;
    bool performerWikipediaPageRetrieved;
    bool performerHomepageRetrieved;
    bool songWikipediaPageRetrieved;
    bool songLyricsURLRetrieved;
    QList<NewsItem> allNewsItems;
    QList<QString> allReviews;

    void init();
    void loadAlbumCoverAS();
    void retrievePerformerMBID();
    void storeInCache(QByteArray* a) const;
};

#endif // EXTERNALDATA_H
