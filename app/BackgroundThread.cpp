#include <QCoreApplication>
#include <QDebug>
#include <QSemaphore>

#include "BackgroundThread.h"
#include "Common.h"
#include "Context.h"
#include "ExternalData.h"

BackgroundThread::BackgroundThread(QObject *parent) : QObject(parent)
{
}

static const QStringList binPaths =
{
    "/bin/",
    "/usr/bin/",
    "/usr/local/bin/",
    "/opt/homebrew/bin/"
};

void
BackgroundThread::processAlbumImages(const QStringList& mbids, const SBKey& key)
{
    //  A collaborate process of iterating...
    _currentKey=key;
    _albumMBIDs << mbids;

    retrieveNextAlbumImageLocations();
}

void
BackgroundThread::retrieveNextAlbumImageLocations()
{
    qDebug() << SB_DEBUG_INFO << _albumMBIDs.size();
    if(_albumMBIDs.size())
    {
        const QString mbid=_albumMBIDs.first();
        _albumMBIDs.pop_front();
        qDebug() << SB_DEBUG_INFO << mbid;

        emit retrieveSingleAlbumImageLocations(mbid,_currentKey);
        return;
    }
    qDebug() << SB_DEBUG_INFO << _albumMBIDs.size();
    emit done();
}

void
BackgroundThread::retrieveImageData(const QStringList& urls, const SBKey& key)
{
    qDebug() << SB_DEBUG_INFO << urls;
    const static QString wget("wget");

    //	Locate wget
    for ( const auto& i : binPaths )
    {
        const QString wgetPath=i + wget;

        if(QFile::exists(wgetPath))
        {
            //	Retrieve image with wget
            const QString imageLocation=ExternalData::getCachePath(key);
            QStringList arguments;
            for(const auto& url: urls)
            {
                arguments << url << "-O" << imageLocation;
                QProcess runWget(NULL);

                runWget.start(wgetPath,arguments);
                runWget.waitForFinished();

                if(runWget.exitStatus()==QProcess::NormalExit)
                {
                    emit imageDataReady(imageLocation,key);
                    return; //  only need to get one image for the same SBKey
                }
                else
                {
                    qDebug() << SB_DEBUG_WARNING << "Unable to run " << wget;
                    qDebug() << SB_DEBUG_WARNING << runWget.readAll();
                }
            }
            return;
        }
    }
    qDebug() << SB_DEBUG_WARNING << "Unable to locate" << wget << "in the following directories:" << binPaths;
}
