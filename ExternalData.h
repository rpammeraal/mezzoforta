#ifndef EXTERNALDATA_H
#define EXTERNALDATA_H

#include <QObject>
#include <QPixmap>

#include "SBID.h"

class QNetworkReply;

class ExternalData : public QObject
{
    Q_OBJECT
public:
    explicit ExternalData(QObject *parent = 0);
    ~ExternalData();

    void loadAlbumCover(const SBID& id);
    void loadPerformerData(const SBID id);

signals:
    void performerHomePageAvailable(const QString& url);
    void performerWikipediaPageAvailable(const QString& url);
    void imageDataReady(const QPixmap& p);
    void updatePerformerMBID(const SBID& id);
    void updatePerformerHomePage(const SBID& id);

public slots:
    void albumCoverMetadataRetrievedAS(QNetworkReply *r);
    void imagedataRetrieved(QNetworkReply *r);
    void performerMBIDRetrieved(QNetworkReply *r);
    void performerImageRetrievedEN(QNetworkReply *r);
    void performerURLDataRetrievedMB(QNetworkReply *r);

private:
    SBID currentID;
    bool performerWikipediaPageRetrieved;
    bool performerHomepageRetrieved;

    void init();
    QString getCachePath() const;
    void loadAlbumCoverAS();
    void loadPerformerDataMB();
    bool loadImageFromCache(QPixmap& p);
    void storeInCache(QByteArray* a) const;
};

#endif // EXTERNALDATA_H
