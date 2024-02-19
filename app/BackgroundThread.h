#ifndef BACKGROUNDTHREAD_H
#define BACKGROUNDTHREAD_H

#include <QObject>
#include "SBKey.h"

class BackgroundThread : public QObject
{
    Q_OBJECT

public:
    explicit BackgroundThread(QObject *parent = 0);

signals:
    void imageDataReady(const QString& path, const SBKey& key);
    void retrieveSingleAlbumImageLocations(const QString& mbid, const SBKey& key);
    void done();

public slots:
    void processAlbumImages(const QStringList& mbids, const SBKey& key);
    void retrieveNextAlbumImageLocations();
    void retrieveImageData(const QStringList& urls, const SBKey& key);

private:
    SBKey                   _currentKey;
    QStringList             _albumMBIDs;
};

#endif // BACKGROUNDTHREAD_H
