#ifndef EXTERNALDATA_H
#define EXTERNALDATA_H

#include <QObject>
#include <QPixmap>

class QNetworkReply;
class QNetworkAccessManager;

static const int MUSICBRAINZ_MAXNUM=100;

#include "SBKey.h"

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
    void loadAlbumData(SBKey ptr);
    void loadPerformerData(SBKey ptr);
    void loadSongData(SBKey ptr);

    //	Static methods
    QString static getCachePath(SBKey id);

signals:
    void albumWikipediaPageAvailable(const QString& url);
    void albumReviewsAvailable(const QList<QString>& allReviews);
    void imageDataReady(const QPixmap& p);
    void performerHomePageAvailable(const QString& url);
    void performerNewsAvailable(const QList<NewsItem>& news);
    void performerWikipediaPageAvailable(const QString& url);
    void songLyricsURLAvailable(const QString& url);
    void songWikipediaPageAvailable(const QString& url);
    void updatePerformerMBID(SBKey key);
    void updatePerformerHomePage(SBKey key);

public slots:
    void handleAlbumImageURLFromAS(QNetworkReply *r);
    void handleAlbumURLDataFromMB(QNetworkReply* r);
    void handleImageDataNetwork(QNetworkReply* r);
    void handleMBIDNetwork(QNetworkReply* r);
    void handlePerformerImageURLFromWC(QNetworkReply* r);
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
    SBKey _currentKey;
    bool _performerMBIDRetrievedFlag;       //	set if call to retrieve MBID already has been made.

    QList<NewsItem> _allNewsItems;
    QList<QString> _allReviews;

    void _init();
    //bool _fuzzyMatch(const SBIDBase &i, const SBIDBase &j) const;
    void _loadAlbumCoverAS();
    static bool _loadImageFromCache(QPixmap& p,SBKey key);
    void _getMBIDAndMore();
    void _sendMusicBrainzQuery(QNetworkAccessManager* mb,const QString& url);
    void _storeInCache(QByteArray* a) const;

};

#endif // EXTERNALDATA_H
