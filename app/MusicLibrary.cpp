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
    //	Important lists: all have `path' as key
    QMap<QString,MLperformancePtr> pathToSong;
    QMap<QString,MLentryPtr> foundPaths;
    QVector<QString> newEntries;

    QMap<QString,MLperformerPtr> performersFound; //	key:performerName
    QMap<QString,MLalbumPtr> albumsFound;	//	key:album path	//	CWIP: shouldn't key be albumTitle, albumPerformerID
    QMap<QString,MLsongPtr> songsFound;

    SBIDPerformerMgr* pemgr=Context::instance()->getPerformerMgr();
    SBIDAlbumMgr* amgr=Context::instance()->getAlbumMgr();
    SBIDSongMgr* smgr=Context::instance()->getSongMgr();

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
    //	QMap<QString,QString> lower2Upper;	use MLentry.path to get org path
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
                path=path.mid(schemaRoot.length());
                MLentry entry;
                entry.path=path;
                foundPaths[path.toLower()]=std::make_shared<MLentry>(entry);
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

//    //	display entries
//    QMapIterator<QString,MLentry> eIT(foundPaths);
//    qDebug() << SB_DEBUG_INFO;
//    while(eIT.hasNext())
//    {
//        eIT.next();
//        qDebug() << SB_DEBUG_INFO << eIT.key();
//    }

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
    SBSqlQueryModel* sqm=SBIDPerformance::onlinePerformances();

    pd.setMaximum(sqm->rowCount());
    pd.setValue(0);
    pd.setLabelText("Retrieving existing songs...");
    QCoreApplication::processEvents();

    QMap<QString,bool> existingPath;
    for(int i=0;i<sqm->rowCount();i++)
    {
        MLperformance performance;
        performance.songID=sqm->data(sqm->index(i,0)).toInt();
        performance.performerID=sqm->data(sqm->index(i,2)).toInt();
        performance.albumID=sqm->data(sqm->index(i,4)).toInt();
        performance.albumPosition=sqm->data(sqm->index(i,6)).toInt();
        performance.path=sqm->data(sqm->index(i,7)).toString().replace("\\","");
        QString path=performance.path.toLower();


        pathToSong[path]=std::make_shared<MLperformance>(performance);
        existingPath[path]=0;

//        if(path.contains("take on me"))
//        {
//            qDebug() << SB_DEBUG_INFO << path;
//        }

        if((i%100)==0)
        {
            pd.setValue(i);
            QCoreApplication::processEvents();
        }
    }

//    QMapIterator<QString,MLperformancePtr> ptsIt(pathToSong);
//    while(ptsIt.hasNext())
//    {
//        ptsIt.next();
//        qDebug() << SB_DEBUG_INFO << ptsIt.key();
//    }

    newEntries.reserve(foundPaths.count());

    QMapIterator<QString,MLentryPtr> fpIT(foundPaths);
    int i=0;
    while(fpIT.hasNext())
    {
        fpIT.next();
        const QString currentPath=fpIT.key();

        if(pathToSong.contains(currentPath))
        {
            pathToSong[currentPath]->pathExists=1;
        }
        else
        {
            MLentryPtr entryPtr=fpIT.value();
            qDebug() << SB_DEBUG_INFO << currentPath;

            //	Determine any attributes from each file.
            MetaData md(schemaRoot+entryPtr->path);

            entryPtr->albumTitle=md.albumTitle();
            entryPtr->albumPosition=md.albumPosition();
            entryPtr->duration=md.duration();
            entryPtr->genre=md.genre();
            entryPtr->notes=md.notes();
            entryPtr->songPerformerName=md.songPerformerName();
            entryPtr->songTitle=md.songTitle();
            entryPtr->year=md.year();

            //	Update new performer data collection
            if(!performersFound.contains(entryPtr->songPerformerName))
            {
                MLperformer performer;
                performer.name=entryPtr->songPerformerName;
                performer.paths.append(currentPath);
                performersFound[entryPtr->songPerformerName]=std::make_shared<MLperformer>(performer);
            }
            else
            {
                MLperformerPtr performerPtr=performersFound[entryPtr->songPerformerName];
                performerPtr->paths.append(currentPath);
            }
            newEntries.append(currentPath);
        }
        if((i%100)==0)
        {
            pd.setValue(i);
            QCoreApplication::processEvents();
            //qDebug() << i << foundPaths.count() << performerToTmpID.count() << albumToTmpID.count();
        }
    }

//    //	newEntries
//    for(int i=0;i<newEntries.count();i++)
//    {
//        QString path=newEntries.at(i);
//        MLentryPtr entryPtr=foundPaths[path];
//        qDebug() << SB_DEBUG_INFO << "found: " << path
//                 << ":performer=" << entryPtr->songPerformerName
//                 << ":songTitle=" << entryPtr->songTitle
//                 << ":albumTitle=" << entryPtr->albumTitle
//        ;
//    }

    ///////////////////////////////////////////////////////////////////////////////////
    ///	Iterating Validation
    /// 	-	Validate performers first.
    /// 	-	Based on this, validate albums.
    /// 	-	Validate songs
    ///////////////////////////////////////////////////////////////////////////////////

    //	A.	Validate performers
    QMapIterator<QString,MLperformerPtr> npIT(performersFound);
    while(npIT.hasNext())
    {
        npIT.next();
        MLperformerPtr foundPerformerPtr=npIT.value();
        Common::sb_parameters tobeMatched;
        tobeMatched.performerName=foundPerformerPtr->name;
        SBIDPerformerPtr selectedPerformerPtr=pemgr->userMatch(tobeMatched,SBIDPerformerPtr());
        if(!selectedPerformerPtr)
        {
            qDebug() << SB_DEBUG_INFO << "none selected -- exit from import";
            return;
        }
        foundPerformerPtr->performerID=selectedPerformerPtr->performerID();

        for(int i=0;i<foundPerformerPtr->paths.count();i++)
        {
            qDebug() << SB_DEBUG_INFO << foundPerformerPtr->name << foundPerformerPtr->performerID << foundPerformerPtr->paths[i];
            //	Set performerID on new entries
            foundPaths[foundPerformerPtr->paths[i]]->songPerformerID=selectedPerformerPtr->performerID();
        }
    }

    //	B.	Validate albums

    //	Collect all unique albumPaths
    SBIDPerformerPtr variousPerformerPtr=SBIDPerformer::retrieveVariousArtists();
    for(int i=0;i<newEntries.count();i++)
    {
        const QString currentPath=newEntries[i];
        QFileInfo songFile=schemaRoot+'/'+currentPath;
        MLentryPtr currentPtr=foundPaths[currentPath];
        //	need to take currentPtr->albumTitle into account
        currentPtr->dirName=currentPath.left(currentPath.length()-songFile.fileName().length()-1);

        if(!albumsFound.contains(currentPtr->dirName))
        {
            MLalbum album;

            album.path=currentPtr->dirName;
            album.albumPerformerID=currentPtr->songPerformerID;	//	Assign songPerformerID initially
            album.title=currentPtr->albumTitle;
            album.paths.append(currentPath);

            albumsFound[currentPtr->dirName]=std::make_shared<MLalbum>(album);
        }
        else
        {
            MLalbumPtr albumPtr=albumsFound[currentPtr->dirName];
            albumPtr->paths.append(currentPath);
            if(albumPtr->albumPerformerID!=currentPtr->songPerformerID)
            {
                albumPtr->albumPerformerID=variousPerformerPtr->performerID();	//	If existing does not match, overwrite with variousPerformers
            }
        }
    }

    //	Assign albumPerformerID to all foundPaths.
    for(int i=0;i<newEntries.count();i++)
    {
        const QString currentPath=newEntries[i];
        MLentryPtr currentPtr=foundPaths[currentPath];
        currentPtr->albumPerformerID=albumsFound[currentPtr->dirName]->albumPerformerID;
    }

    //	Validate albums
    QMapIterator<QString,MLalbumPtr> afIT(albumsFound);
    qDebug() << SB_DEBUG_INFO;
    while(afIT.hasNext())
    {
        afIT.next();
        MLalbumPtr foundAlbumPtr=afIT.value();

        Common::sb_parameters parameters;
        parameters.albumTitle=foundAlbumPtr->title;
        parameters.performerID=foundAlbumPtr->albumPerformerID;

        SBIDAlbumPtr selectedAlbumPtr=amgr->userMatch(parameters,SBIDAlbumPtr());
        if(!selectedAlbumPtr)
        {
            qDebug() << SB_DEBUG_INFO << "none selected -- exit from import";
            return;
        }
        qDebug() << SB_DEBUG_INFO << selectedAlbumPtr->key() << selectedAlbumPtr->genericDescription();
        foundAlbumPtr->albumID=selectedAlbumPtr->albumID();

        for(int i=0;i<foundAlbumPtr->paths.count();i++)
        {
            //	Set album fields
            MLentryPtr entryPtr=foundPaths[foundAlbumPtr->paths[i]];
            entryPtr->albumID=selectedAlbumPtr->albumID();
            entryPtr->albumPerformerID=selectedAlbumPtr->albumPerformerID();
        }
    }

    //	C.	Validate songs
    for(int i=0;i<newEntries.count();i++)
    {
        const QString currentPath=newEntries[i];
        MLentryPtr currentPtr=foundPaths[currentPath];

        MLsong song;
        song.performerID=currentPtr->songPerformerID;
        song.songTitle=currentPtr->songTitle;
        song.songPerformerName=currentPtr->songPerformerName;

        QString key=song.key();

        if(!songsFound.contains(key))
        {
            song.paths.append(currentPath);
            songsFound[key]=std::make_shared<MLsong>(song);
        }
        else
        {
            MLsongPtr songPtr=songsFound[key];
            songPtr->paths.append(currentPath);
        }
    }

    QMapIterator<QString,MLsongPtr> sfIT(songsFound);
    while(sfIT.hasNext())
    {
        sfIT.next();

        MLsongPtr foundSongPtr=sfIT.value();

        Common::sb_parameters parameters;
        parameters.songTitle=foundSongPtr->songTitle;
        parameters.performerID=foundSongPtr->performerID;
        parameters.performerName=foundSongPtr->songPerformerName;

        qDebug() << SB_DEBUG_INFO << sfIT.key() << foundSongPtr->songTitle << foundSongPtr->performerID;
        SBIDSongPtr selectedSongPtr=smgr->userMatch(parameters,SBIDSongPtr());
        if(!selectedSongPtr)
        {
            qDebug() << SB_DEBUG_INFO << "none selected -- exit from import";
            return;
        }

        foundSongPtr->songID=selectedSongPtr->songID();
        foundSongPtr->performerID=selectedSongPtr->songPerformerID();

        for(int i=0;i<foundSongPtr->paths.count();i++)
        {
            //	Set song fields
            MLentryPtr entryPtr=foundPaths[foundSongPtr->paths[i]];
            entryPtr->songID=selectedSongPtr->songID();
            entryPtr->songPerformerID=selectedSongPtr->songPerformerID();
        }
    }

    //	CWIP: need to merge new songs with existing songs on the same album based on matching on title *and* song position.
    //	songs that cannot be merged, need to be *appended* at the end of the album and be assigned a new album position.

    //	Sanity check
    for(int i=0;i<newEntries.count();i++)
    {
        const QString currentPath=newEntries[i];
        if(foundPaths.contains(currentPath))
        {
            MLentryPtr currentPtr=foundPaths[currentPath];
            qDebug() << SB_DEBUG_INFO << currentPtr->path
                     << currentPtr->songPerformerID
                     << currentPtr->albumPerformerID
                     << currentPtr->albumID
                     << currentPtr->albumPosition
                     << currentPtr->songID
            ;
        }
        else
        {
            //	CWIP: error dialog box
            qDebug() << SB_DEBUG_ERROR << "not found!" << currentPath;
        }
    }
    return;

//    qDebug() << SB_DEBUG_INFO << "L7";
//    QMapIterator<QString,QString> shit(errors);
//    while(shit.hasNext())
//    {
//        shit.next();
//        qDebug() << SB_DEBUG_ERROR << shit.key() << ":" << shit.value();
//    }
//    qDebug() << SB_DEBUG_INFO << "total files found" << foundPaths.count();
//    qDebug() << SB_DEBUG_INFO << "new files" << newEntries.count();

//    QListIterator<SBIDSong> newEntriesIT(newEntries);
//    while(newEntriesIT.hasNext())
//    {
//        SBIDSong s=newEntriesIT.next();
//        qDebug() << SB_DEBUG_INFO << "new" << s.path;
//    }
//    */

//    //	Deal with missing files
//    int missingFiles=0;
//    QMapIterator<QString,bool> pe_i(existingPath);
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
//    QMapIterator<SBIDPerformer,int> performerToTmpIDIT(performerToTmpID);
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
//    QMapIterator<SBIDAlbumSimpleCompare,int> albumToTmpIDIT(albumToTmpID);
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
