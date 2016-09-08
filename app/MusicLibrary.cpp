#include <QDir>
#include <QProgressDialog>

#include "MusicLibrary.h"

#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "DataEntityPerformer.h"
#include "DataEntitySong.h"
#include "MetaData.h"
#include "SBIDAlbum.h"
#include "SBIDPerformer.h"
#include "SBIDSong.h"
#include "SBMessageBox.h"
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
    bool newPerformersSavedFlag=0;
    qDebug() << SB_DEBUG_INFO << schema;

    const QString schemaRoot=
        Context::instance()->getProperties()->musicLibraryDirectory()
        +"/"
        +schema
        +(schema.length()?"/":"");
    qDebug() << SB_DEBUG_INFO << schemaRoot;
    //schemaRoot="/Volumes/Home/roy/Music/songbase/music/files/rock/V/VARIOUS ARTISTS/100 Chillout Classics 4of5/";

    ///////////////////////////////////////////////////////////////////////////////////
    ///	Retrieve paths
    ///////////////////////////////////////////////////////////////////////////////////
    QStringList entries;
    QMap<QString,QString> lower2Upper;
    Controller* c=Context::instance()->getController();
    int numFiles=0;
    QDirIterator it(schemaRoot,
                    QDir::AllDirs | QDir::AllEntries | QDir::Files | QDir::NoSymLinks | QDir::Readable,
                    QDirIterator::Subdirectories);
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
        if(fi.isFile())
        {
            if(fi.size()>10*1024)
            {
                entries.append(path.toLower());
                lower2Upper[path.toLower()]=path;
                numFiles++;
            }
            else
            {
                qDebug() << SB_DEBUG_WARNING << "File size too short:" << path;
            }
        }
    }
    c->updateStatusBarText(QString("Found %1 files").arg(numFiles));
    QCoreApplication::processEvents();

    ///////////////////////////////////////////////////////////////////////////////////
    ///	Retrieve existing data
    ///////////////////////////////////////////////////////////////////////////////////

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
        song.sb_song_performer_id=sqm->data(sqm->index(i,1)).toInt();
        song.sb_album_id=sqm->data(sqm->index(i,2)).toInt();
        song.sb_position=sqm->data(sqm->index(i,3)).toInt();
        QString path=schemaRoot.toLower()+sqm->data(sqm->index(i,4)).toString().replace("\\","").toLower();


        pathToSong[path]=song;
        existingPath[path]=0;

        if(path.contains("take on me"))
        {
            qDebug() << SB_DEBUG_INFO << path;
        }

        if((i%100)==0)
        {
            pd.setValue(i);
            QCoreApplication::processEvents();
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////
    ///	Creating data structures
    ///////////////////////////////////////////////////////////////////////////////////

    QHash<SBIDPerformer,int> performerToUniqueID;
    QHash<SBIDAlbumSimpleCompare,int> albumToUniqueID;
    QList<SBIDSong> newEntries;
    QMap<QString,QString> errors;

    performerToUniqueID.reserve(entries.count());
    albumToUniqueID.reserve(entries.count());
    newEntries.reserve(entries.count());

    pd.setMaximum(sqm->rowCount());
    pd.setValue(0);
    pd.setLabelText("Processing new music");
    QCoreApplication::processEvents();

    QHashIterator<QString,bool> hi(existingPath);

    for(int i=0;i<entries.count();i++)
    {
        if(pathToSong.contains(entries.at(i)))
        {
            existingPath[entries.at(i)]=1;
        }
        else
        {
            qDebug() << SB_DEBUG_INFO << entries.at(i);

            //	Determine any attributes from each file.
            MetaData md(lower2Upper[entries.at(i)]);

            //	For each song, determine all unique performers and albums
            SBIDSong song=md.parse();
            song.path=lower2Upper[entries.at(i)];
            song.assignUniqueItemID();
            qDebug() << SB_DEBUG_INFO << song << song.path;

            SBIDPerformer performer=song;
            performer.sb_unique_song_id=song.sb_unique_item_id;
            if(performerToUniqueID.contains(performer)==0)
            {
                performer.assignUniqueItemID();
                performerToUniqueID[performer]=performer.sb_unique_item_id;
            }
            else
            {
                performer.sb_unique_item_id=performerToUniqueID[performer];
            }
            song.sb_unique_performer_id=performer.sb_unique_item_id;

            SBIDAlbumSimpleCompare album=song;
            ///	CWIP
            ///	NEED TO THINK ABOUT
            /// -	missing album performer name
            /// -	mixed album names in one directory
            album.albumPerformerName=song.songPerformerName;

            album.sb_unique_song_id=song.sb_unique_item_id;
            if(albumToUniqueID.contains(album)==0)
            {
                album.assignUniqueItemID();
                albumToUniqueID[album]=album.sb_unique_item_id;
            }
            else
            {
                album.sb_unique_item_id=albumToUniqueID[album];
            }
            song.sb_unique_album_id=album.sb_unique_item_id;

            newEntries.append(song);
        }
        if((i%100)==0)
        {
            pd.setValue(i);
            QCoreApplication::processEvents();
            //qDebug() << i << entries.count() << performerToUniqueID.count() << albumToUniqueID.count();
        }
    }
    SBID::listUniqueIDItems();

    ///////////////////////////////////////////////////////////////////////////////////
    ///	Validation
    ///////////////////////////////////////////////////////////////////////////////////

    qDebug() << SB_DEBUG_INFO << "L7";
    QMapIterator<QString,QString> shit(errors);
    while(shit.hasNext())
    {
        shit.next();
        qDebug() << SB_DEBUG_ERROR << shit.key() << ":" << shit.value();
    }
    qDebug() << SB_DEBUG_INFO << "total files found" << entries.count();
    qDebug() << SB_DEBUG_INFO << "new files" << newEntries.count();
    /*
    QListIterator<SBIDSong> newEntriesIT(newEntries);
    while(newEntriesIT.hasNext())
    {
        SBIDSong s=newEntriesIT.next();
        qDebug() << SB_DEBUG_INFO << "new" << s.path;
    }
    */

    //	Deal with missing files
    int missingFiles=0;
    QHashIterator<QString,bool> pe_i(existingPath);
    QMap<QString,int> missingFilesMap;	//	QMap provides sorting, the value is not used.

    while(pe_i.hasNext())
    {
        pe_i.next();
        if(pe_i.value()==0)
        {
            missingFilesMap[pe_i.key()]=1;
        }
        missingFiles+=(pe_i.value()==0?1:0);
    }
    qDebug() << SB_DEBUG_INFO << "#missing files" << missingFiles;

    //	Now list missing files alphabetically
    QMapIterator<QString,int> mfi(missingFilesMap);
    while(mfi.hasNext())
    {
        mfi.next();
        qDebug() << SB_DEBUG_INFO << "missing" << mfi.key();
    }

    ///////////////////////////////////////////////////////////////////////////////////
    /// Lookup phase
    ///
    /// It would be great if the various SBID* objects are somehow connected with
    /// signals that if the actual sb_item_id changes, all dependent SBID objects
    /// are changed as well.
    ///////////////////////////////////////////////////////////////////////////////////
    QMap<int,int> uniquePerformerID2realPerformerID;
    QMap<int,int> uniqueAlbumID2realAlbumID;

    //	Lookup performers based on performer name, create if not exist.
    qDebug() << SB_DEBUG_INFO << performerToUniqueID.count();
    QHashIterator<SBIDPerformer,int> performerToUniqueIDIT(performerToUniqueID);
    performerToUniqueIDIT.toFront();
    while(performerToUniqueIDIT.hasNext())
    {
        performerToUniqueIDIT.next();
        SBIDPerformer performer=performerToUniqueIDIT.key();
        uniquePerformerID2realPerformerID[performer.sb_unique_item_id]=performer.getDetail(true);
        qDebug() << SB_DEBUG_INFO << performer;
    }

    QMapIterator<int,int> unP2ID(uniquePerformerID2realPerformerID);
    while(unP2ID.hasNext())
    {
        unP2ID.next();
        qDebug() << SB_DEBUG_INFO << unP2ID.key() << unP2ID.value();
    }

    //	Lookup albums
    QHashIterator<SBIDAlbumSimpleCompare,int> albumToUniqueIDIT(albumToUniqueID);
    albumToUniqueIDIT.toFront();
    QMap<int,SBIDAlbum> savedAlbums;
    while(albumToUniqueIDIT.hasNext())
    {
        albumToUniqueIDIT.next();
        SBIDAlbum album=albumToUniqueIDIT.key();

        //	Now put in actual sb_album_performer_id
        album.sb_album_performer_id=uniquePerformerID2realPerformerID[album.sb_unique_performer_id];

        //	Look up
        uniqueAlbumID2realAlbumID[album.sb_unique_item_id]=album.getDetail(true);
        qDebug() << SB_DEBUG_INFO << album;

        savedAlbums[album.sb_album_id]=album;
    }

    QMapIterator<int,int> unA2ID(uniqueAlbumID2realAlbumID);
    while(unA2ID.hasNext())
    {
        unA2ID.next();
        qDebug() << SB_DEBUG_INFO << unA2ID.key() << unA2ID.value();
    }

    //	Lookup songs
    QListIterator<SBIDSong> newEntriesIT(newEntries);
    newEntriesIT.toFront();
    while(newEntriesIT.hasNext())
    {
        SBIDSong song=newEntriesIT.next();

        //	Put in actual sb_album_id, sb_song_performer_id
        song.sb_album_id=uniqueAlbumID2realAlbumID[song.sb_unique_album_id];
        song.sb_song_performer_id=uniquePerformerID2realPerformerID[song.sb_unique_performer_id];

        int sb_song_id=song.getDetail(true);
        if(sb_song_id<0)
        {
            SBMessageBox::standardWarningBox("Unable to continue, abort save.");
            return;
        }
        SBIDAlbum album;
        album=savedAlbums[song.sb_album_id];
        if(album.sb_album_id<0)
        {
            album=SBIDAlbum(song.sb_album_id);
            album.getDetail();
        }
        if(album.saveSongToAlbum(song)==0)
        {
            SBMessageBox::standardWarningBox("Unable to continue, abort save.");
            return;
        }
        qDebug() << SB_DEBUG_INFO << song;
    }

    //	Refresh caches
    //if(newPerformersSavedFlag)	//	CWIP: set nowhere, need to be set somewhere
    {
        Context::instance()->getController()->refreshModels();
    }

    qDebug() << SB_DEBUG_INFO << "Finished";

    return;
}
