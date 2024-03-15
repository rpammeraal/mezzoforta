#ifndef EXTERNALDATA_H
#define EXTERNALDATA_H

#include <QDomElement>
#include <QObject>
#include <QPixmap>
#include <QThread>

class QDomElement;
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

static const QString _defaultIconPath("/images/SongIcon.png");

class ExternalData : public QObject
{
    Q_OBJECT
    QThread t_SingleImageData;
    QThread t_AlbumImageLocations;

public:
    explicit ExternalData(QObject *parent = 0);
    ~ExternalData();

    //	Main interface
    void loadAlbumData(SBKey ptr);
    void loadPerformerData(SBKey ptr);
    void loadSongData(SBKey ptr);

    //	Static methods
    QString static getCachePath(SBKey id);
    QString static getDefaultIconPath(const SBKey::ItemType& itemType);

signals:
    void albumWikipediaPageAvailable(const QString& url);
    void albumReviewsAvailable(const QList<QString>& allReviews);
    void imageDataReady(const QPixmap& p, const SBKey& key);
    void performerHomePageAvailable(const QString& url);
    void performerNewsAvailable(const QList<NewsItem>& news);
    void performerWikipediaPageAvailable(const QString& url);
    void songLyricsURLAvailable(const QString& url);
    void songWikipediaPageAvailable(const QString& url);
    void updatePerformerMBID(const SBKey& key, const QString& mbid);
    void updatePerformerHomePage(SBKey key);
    void startRetrieveImageData(const QStringList& urls, const SBKey& key);

    void startAlbumImageProcess(const QStringList& mbids, const SBKey& key);
    void handleNextAlbumImageLocation();

public slots:
    void processAlbum(QNetworkReply* r);
    void processAlbumImage (QNetworkReply *reply);
    void processAlbumsByPerformer (QNetworkReply *reply);
    void processImageRetrieved(const QString& path, const SBKey& key);
    void processMBID(QNetworkReply* r);
    void processPerformer(QNetworkReply* r);
    void processSong(QNetworkReply* r);
    void processSingleAlbumImageLocations(const QString& mbid, const SBKey& key);
    void processWikidata(QNetworkReply* r);

private:
    bool                    _artworkRetrievedFlag;              //	album performer
    bool                    _performerHomepageRetrievedFlag;    //	performer
    bool                    _performerMBIDRetrievedFlag;        //	set if call to retrieve MBID already has been made.
    bool                    _songLyricsURLRetrievedFlag;        //	song (always retrieved, not stored)
    bool                    _wikipediaURLRetrievedFlag;         // 	album performer song

    int                     _currentOffset;
    SBKey                   _currentKey;
    QString                 _wikiDataQValue;                    //  ID used by wikimedia

    QList<NewsItem>         _allNewsItems;
    QList<QString>          _allReviews;

    void                    _init();
    void                    _softInit();

    QString                 _crtIndent(int tabstops) const;
    void                    _getMBIDAndMore();
    bool                    _lengthCompare(const QString& s1, const QString& s2) const;
    static bool             _loadImageFromCache(QPixmap& p, const SBKey& key);
    QString                 _normalizeString(const QString& s) const;
    void                    _postNetwork(QNetworkReply* r);
    void                    _sendNetworkRequest(QNetworkAccessManager* am, const QString& url);
    void                    _sendNetworkRequest(QNetworkAccessManager* am, const QUrl& url);
    void                    _storeInCache(QByteArray* a) const;


    QStringList             _inspectJsonDoc(const QJsonDocument& jd, const QString& search=QString(), const bool& debug=0) const;
    QStringList             _iterateJsonArray(const QJsonArray& ja, const QStringList& search, const int& recursion, const bool& debug) const;
    QStringList             _recurseJsonObject(const QJsonObject& jo, const QStringList& search, const int& recursion, const bool& debug) const;
    QStringList             _inspectJsonValue(const QJsonValue& jv, const QStringList& search, const int& recursion, const bool& debug) const;
};

#endif // EXTERNALDATA_H
