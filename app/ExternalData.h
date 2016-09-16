#ifndef EXTERNALDATA_H
#define EXTERNALDATA_H

#include <QObject>
#include <QPixmap>

#include "SBIDPerformer.h"

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
    void loadAlbumData(const SBIDBase& id);
    void loadPerformerData(const SBIDBase id);
    void loadSongData(const SBIDBase& id);

    //	Static methods
    QString static getCachePath(const SBIDBase& id);
    static bool loadImageFromCache(QPixmap& p,const SBIDBase& id);

signals:
    void albumWikipediaPageAvailable(const QString& url);
    void albumReviewsAvailable(const QList<QString>& allReviews);
    void imageDataReady(const QPixmap& p);
    void performerHomePageAvailable(const QString& url);
    void performerNewsAvailable(const QList<NewsItem>& news);
    void performerWikipediaPageAvailable(const QString& url);
    void songLyricsURLAvailable(const QString& url);
    void songWikipediaPageAvailable(const QString& url);
    void updatePerformerMBID(const SBIDPerformer& id);
    void updatePerformerHomePage(const SBIDPerformer& id);

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
    bool _artworkRetrievedFlag;             //	album performer
    bool _performerHomepageRetrievedFlag;   //	performer
    bool _songLyricsURLRetrievedFlag;       //	song (always retrieved, not stored)
    bool _wikipediaURLRetrievedFlag;        // 	album performer song

    int _currentOffset;
    SBIDBase _currentID;
    bool _performerMBIDRetrievedFlag;       //	set if call to retrieve MBID already has been made.

    QList<NewsItem> _allNewsItems;
    QList<QString> _allReviews;

    void _init();
    bool _fuzzyMatch(const SBIDBase &i, const SBIDBase &j) const;
    void _loadAlbumCoverAS();
    void _getMBIDAndMore();
    void _storeInCache(QByteArray* a) const;
};

#endif // EXTERNALDATA_H
