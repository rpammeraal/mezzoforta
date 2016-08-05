#include <QDir>
#include <QProgressDialog>

#include "MusicLibrary.h"

#include "AudioDecoder.h"
#include "AudioDecoderFactory.h"
#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "DataEntityPerformer.h"
#include "DataEntitySong.h"
#include "SBIDAlbum.h"
#include "SBIDPerformer.h"
#include "SBIDSong.h"
#include "SBSqlQueryModel.h"

///	Public methods
MusicLibrary::MusicLibrary(QObject *parent) : QObject(parent)
{
}

void
MusicLibrary::rescanMusicLibrary()
{
    QStringList l=Context::instance()->getDataAccessLayer()->availableSchemas();
    if(l.count()==0)
    {
        //	Add an empty schema in case there are no schema.
        l.append(QString());
    }

    l.clear();
    l.append("rock");

    for(int i=0;i<l.count();i++)
    {
        _rescanMusicLibrary(l.at(i));
    }
}

///	Private methods
void
MusicLibrary::_rescanMusicLibrary(const QString& schema)
{
    qDebug() << SB_DEBUG_INFO << schema;

    const QString schemaRoot=
        Context::instance()->getProperties()->musicLibraryDirectory()
        +"/"
        +schema.toLower()
        +(schema.length()?"/":"");
    qDebug() << SB_DEBUG_INFO << schemaRoot;

    //	1.	Get list of entries
    QStringList entries;
    Controller* c=Context::instance()->getController();
    int numFiles=0;
    QDirIterator it(schemaRoot, QDir::Files, QDirIterator::Subdirectories);
    c->updateStatusBarText(QString("Found %1 files").arg(numFiles));
    QCoreApplication::processEvents();
    while (it.hasNext())
    {
        it.next();
        if((numFiles%1000)==0)
        {
            c->updateStatusBarText(QString("Found %1 files").arg(numFiles));
            QCoreApplication::processEvents();
        }
        QFileInfo fi=it.fileInfo();
        QString path=it.filePath();
        if(fi.size()>10*1024)
        {
            entries.append(path);
            numFiles++;
        }
        else
        {
            qDebug() << SB_DEBUG_WARNING << "File size too short:" << path;
        }
    }
    c->updateStatusBarText(QString("Found %1 files").arg(numFiles));
    QCoreApplication::processEvents();

    //	For the next sections, set up a progress bar.
    int maxValue=numFiles;
    QProgressDialog pd("Starting",QString(),0,maxValue);
    pd.setWindowModality(Qt::WindowModal);
    pd.show();
    pd.raise();
    pd.activateWindow();

    //	1.	Retrieve existing paths
    SBSqlQueryModel* sqm=DataEntitySong::getOnlineSongs();

    pd.setMaximum(sqm->rowCount());
    pd.setValue(0);
    pd.setLabelText("Retrieving existing songs...");
    QCoreApplication::processEvents();

    QHash<QString,SBIDSong> pathToSong;
    QHash<QString,bool> existingPath;
    for(int i=0;i<sqm->rowCount();i++)
    {
        SBIDSong song;
        song.assign(sqm->data(sqm->index(i,0)).toInt());
        song.sb_performer_id=sqm->data(sqm->index(i,1)).toInt();
        song.sb_album_id=sqm->data(sqm->index(i,2)).toInt();
        song.sb_position=sqm->data(sqm->index(i,3)).toInt();
        QString path=schemaRoot+sqm->data(sqm->index(i,4)).toString().replace("\\","");

        pathToSong[path]=song;
        existingPath[path]=0;

        if((i%100)==0)
        {
            pd.setValue(i);
            QCoreApplication::processEvents();
        }
        if(path.contains("Visitors"))
        {
            qDebug() << SB_DEBUG_INFO << path;
        }
    }

    //	2.	Process entries, collect performers
    QHash<SBIDPerformer,int> performerToID;
    AudioDecoderFactory adf;
    QList<SBIDSong> newEntries;
    QHash<SBIDAlbumSimpleCompare,int> albumToID;
    QMap<QString,QString> errors;

    performerToID.reserve(entries.count());
    albumToID.reserve(entries.count());
    newEntries.reserve(entries.count());

    pd.setMaximum(sqm->rowCount());
    pd.setValue(0);
    pd.setLabelText("Processing new music");
    QCoreApplication::processEvents();

    QHashIterator<QString,bool> hi(existingPath);
    int cnt=0;

    for(int i=0;i<entries.count();i++)
    {
        if(pathToSong.contains(entries.at(i)))
        {
            existingPath[entries.at(i)]=1;
        }
        else
        {
            qDebug() << SB_DEBUG_INFO << cnt++ << entries.at(i);
            return;

            //	Determine any attributes from each file.
            AudioDecoder* ad=adf.openFileHeader(entries.at(i));
            if(ad)
            {
                //	For each song, determine all unique performers and albums
                SBIDSong item=ad->header();

                SBIDPerformer p=item;
                if(performerToID.contains(p)==0)
                {
                    performerToID[p]=-1;
                }

                SBIDAlbumSimpleCompare a=item;
                if(albumToID.contains(a)==0)
                {
                    albumToID[a]=-1;
                }
                newEntries.append(item);

                delete ad;
            }
            else
            {
                errors[entries.at(i)]=adf.error();
            }
        }
        if((i%100)==0)
        {
            pd.setValue(i);
            QCoreApplication::processEvents();
            qDebug() << i << entries.count() << performerToID.count() << albumToID.count();
        }
    }
    return;
    qDebug() << SB_DEBUG_INFO << "L7";
    QMapIterator<QString,QString> shit(errors);
    while(shit.hasNext())
    {
        shit.next();
        qDebug() << SB_DEBUG_ERROR << shit.key() << ":" << shit.value();
    }
    qDebug() << SB_DEBUG_INFO << "total files found" << entries.count();
    qDebug() << SB_DEBUG_INFO << "new files" << newEntries.count();
    int missingFiles=0;
    QHashIterator<QString,bool> pe_i(existingPath);
    while(pe_i.hasNext())
    {
        pe_i.next();
        missingFiles+=(pe_i.value()==0?1:0);
    }
    qDebug() << SB_DEBUG_INFO << "missing files" << missingFiles;

    //	Lookup performers, create if not exists
    QHashIterator<SBIDPerformer,int> pit(performerToID);
    DataEntityPerformer dep;
    while(pit.hasNext())
    {
        pit.next();
        SBIDPerformer p=pit.key();
        performerToID[p]=p.getDetail(1);
        qDebug() << SB_DEBUG_INFO << p;
    }
    pit.toFront();

    //	next section to be thrown away
    SBIDPerformer lookup;
    while(pit.hasNext())
    {
        pit.next();
        if(pit.value()==-1)
        {
            qDebug() << SB_DEBUG_INFO << "Unknown artist" << pit.key();
            for(int i=0;i<=newEntries.count();i++)
            {
                lookup.performerName=newEntries.at(i).performerName;
                if(performerToID[lookup]==-1)
                {
                    qDebug() << SB_DEBUG_INFO << newEntries[i];
                }
            }
        }
    }
    //	end next section to be thrown away

    return;
    /*
    QHashIterator<SBIDAlbum,int> ait(albumToID);
    while(ait.hasNext())
    {
        ait.next();
        qDebug() << SB_DEBUG_INFO << "album" << ait.key();
    }
    */
}
