#include <QDir>
#include <QProgressDialog>

#include "MusicLibrary.h"

#include "Common.h"
#include "Context.h"
#include "Controller.h"
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
    SBSqlQueryModel* sqm=SBIDSong::getOnlineSongs();

    pd.setMaximum(sqm->rowCount());
    pd.setValue(0);
    pd.setLabelText("Retrieving existing songs...");
    QCoreApplication::processEvents();

    QHash<QString,SBIDSong> pathToSong;
    QHash<QString,bool> existingPath;
    for(int i=0;i<sqm->rowCount();i++)
    {
        SBIDSong song(sqm->data(sqm->index(i,0)).toInt());
        song.setSongPerformerID(sqm->data(sqm->index(i,1)).toInt());
        song.setAlbumID(sqm->data(sqm->index(i,2)).toInt());
        song.setAlbumPosition(sqm->data(sqm->index(i,3)).toInt());
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

    //	NEED TO RECREATE THIS AFTER IMPLEMENTATION OF SBIDMANAGERTEMPLATE

//    QHash<SBIDPerformer,int> performerToTmpID;
//    QHash<SBIDAlbumSimpleCompare,int> albumToTmpID;
//    QList<SBIDSong> newEntries;
//    QMap<QString,QString> errors;

//    performerToTmpID.reserve(entries.count());
//    albumToTmpID.reserve(entries.count());
//    newEntries.reserve(entries.count());

//    pd.setMaximum(sqm->rowCount());
//    pd.setValue(0);
//    pd.setLabelText("Processing new music");
//    QCoreApplication::processEvents();

//    QHashIterator<QString,bool> hi(existingPath);

//    for(int i=0;i<entries.count();i++)
//    {
//        if(pathToSong.contains(entries.at(i)))
//        {
//            existingPath[entries.at(i)]=1;
//        }
//        else
//        {
//            qDebug() << SB_DEBUG_INFO << entries.at(i);

//            //	Determine any attributes from each file.
//            MetaData md(lower2Upper[entries.at(i)]);

//            //	For each song, determine all tmp performers and albums
//            SBIDSong song=md.parse();
//            song.setPath(lower2Upper[entries.at(i)]);
//            song.assignTmpItemID();
//            qDebug() << SB_DEBUG_INFO << song << song.path();

//            SBIDPerformer performer=song;
//            performer.setTmpSongID(song.tmpItemID());
//            if(performerToTmpID.contains(performer)==0)
//            {
//                performer.assignTmpItemID();
//                performerToTmpID[performer]=performer.tmpItemID();
//            }
//            else
//            {
//                performer.setTmpItemID(performerToTmpID[performer]);
//            }
//            song.setTmpPerformerID(performer.tmpItemID());

//            SBIDAlbumSimpleCompare album=song;
//            ///	CWIP
//            ///	NEED TO THINK ABOUT
//            /// -	missing album performer name
//            /// -	mixed album names in one directory
//            album.setAlbumPerformerName(song.songPerformerName());

//            album.setTmpSongID(song.tmpItemID());
//            if(albumToTmpID.contains(album)==0)
//            {
//                album.assignTmpItemID();
//                albumToTmpID[album]=album.tmpItemID();
//            }
//            else
//            {
//                album.setTmpItemID(albumToTmpID[album]);
//            }
//            song.setTmpAlbumID(album.tmpItemID());

//            newEntries.append(song);
//        }
//        if((i%100)==0)
//        {
//            pd.setValue(i);
//            QCoreApplication::processEvents();
//            //qDebug() << i << entries.count() << performerToTmpID.count() << albumToTmpID.count();
//        }
//    }
//    SBIDBase::listTmpIDItems();

//    ///////////////////////////////////////////////////////////////////////////////////
//    ///	Validation
//    ///////////////////////////////////////////////////////////////////////////////////

//    qDebug() << SB_DEBUG_INFO << "L7";
//    QMapIterator<QString,QString> shit(errors);
//    while(shit.hasNext())
//    {
//        shit.next();
//        qDebug() << SB_DEBUG_ERROR << shit.key() << ":" << shit.value();
//    }
//    qDebug() << SB_DEBUG_INFO << "total files found" << entries.count();
//    qDebug() << SB_DEBUG_INFO << "new files" << newEntries.count();
//    /*
//    QListIterator<SBIDSong> newEntriesIT(newEntries);
//    while(newEntriesIT.hasNext())
//    {
//        SBIDSong s=newEntriesIT.next();
//        qDebug() << SB_DEBUG_INFO << "new" << s.path;
//    }
//    */

//    //	Deal with missing files
//    int missingFiles=0;
//    QHashIterator<QString,bool> pe_i(existingPath);
//    QMap<QString,int> missingFilesMap;	//	QMap provides sorting, the value is not used.

//    while(pe_i.hasNext())
//    {
//        pe_i.next();
//        if(pe_i.value()==0)
//        {
//            missingFilesMap[pe_i.key()]=1;
//        }
//        missingFiles+=(pe_i.value()==0?1:0);
//    }
//    qDebug() << SB_DEBUG_INFO << "#missing files" << missingFiles;

//    //	Now list missing files alphabetically
//    QMapIterator<QString,int> mfi(missingFilesMap);
//    while(mfi.hasNext())
//    {
//        mfi.next();
//        qDebug() << SB_DEBUG_INFO << "missing" << mfi.key();
//    }

//    ///////////////////////////////////////////////////////////////////////////////////
//    /// Lookup phase
//    ///
//    /// It would be great if the various SBIDBase* objects are somehow connected with
//    /// signals that if the actual sb_item_id changes, all dependent SBIDBase objects
//    /// are changed as well.
//    ///////////////////////////////////////////////////////////////////////////////////
//    QMap<int,int> tmpPerformerID2realPerformerID;
//    QMap<int,int> tmpAlbumID2realAlbumID;

//    //	Lookup performers based on performer name, create if not exist.
//    qDebug() << SB_DEBUG_INFO << performerToTmpID.count();
//    QHashIterator<SBIDPerformer,int> performerToTmpIDIT(performerToTmpID);
//    performerToTmpIDIT.toFront();
//    while(performerToTmpIDIT.hasNext())
//    {
//        performerToTmpIDIT.next();
//        SBIDPerformer performer=performerToTmpIDIT.key();
//        tmpPerformerID2realPerformerID[performer.tmpItemID()]=performer.getDetail(true);
//        qDebug() << SB_DEBUG_INFO << performer;
//    }

//    QMapIterator<int,int> unP2ID(tmpPerformerID2realPerformerID);
//    while(unP2ID.hasNext())
//    {
//        unP2ID.next();
//        qDebug() << SB_DEBUG_INFO << unP2ID.key() << unP2ID.value();
//    }

//    //	Lookup albums
//    QHashIterator<SBIDAlbumSimpleCompare,int> albumToTmpIDIT(albumToTmpID);
//    albumToTmpIDIT.toFront();
//    QMap<int,SBIDAlbum> savedAlbums;
//    while(albumToTmpIDIT.hasNext())
//    {
//        albumToTmpIDIT.next();
//        SBIDAlbum album=albumToTmpIDIT.key();

//        //	Now put in actual sb_album_performer_id
//        album.setAlbumPerformerID(tmpPerformerID2realPerformerID[album.tmpPerformerID()]);

//        //	Look up
//        tmpAlbumID2realAlbumID[album.tmpItemID()]=album.getDetail(true);
//        qDebug() << SB_DEBUG_INFO << album;

//        savedAlbums[album.albumID()]=album;
//    }

//    QMapIterator<int,int> unA2ID(tmpAlbumID2realAlbumID);
//    while(unA2ID.hasNext())
//    {
//        unA2ID.next();
//        qDebug() << SB_DEBUG_INFO << unA2ID.key() << unA2ID.value();
//    }

//    //	Lookup songs
//    QListIterator<SBIDSong> newEntriesIT(newEntries);
//    newEntriesIT.toFront();
//    while(newEntriesIT.hasNext())
//    {
//        SBIDSong song=newEntriesIT.next();

//        //	Put in actual sb_album_id, sb_song_performer_id
//        song.setAlbumID(tmpAlbumID2realAlbumID[song.tmpAlbumID()]);
//        song.setSongPerformerID(tmpPerformerID2realPerformerID[song.tmpPerformerID()]);

//        int sb_song_id=song.getDetail(true);
//        if(sb_song_id<0)
//        {
//            SBMessageBox::standardWarningBox("Unable to continue, abort save.");
//            return;
//        }
//        SBIDAlbum album;
//        album=savedAlbums[song.albumID()];
//        if(album.albumID()<0)
//        {
//            album=SBIDAlbum(song.albumID());
//            album.getDetail();
//        }
//        if(album.saveSongToAlbum(song)==0)
//        {
//            SBMessageBox::standardWarningBox("Unable to continue, abort save.");
//            return;
//        }
//        qDebug() << SB_DEBUG_INFO << song;
//    }

//    //	Refresh caches
//    //if(newPerformersSavedFlag)	//	CWIP: set nowhere, need to be set somewhere
//    {
//        Context::instance()->getController()->refreshModels();
//    }

    qDebug() << SB_DEBUG_INFO << "Finished";

    return;
}
