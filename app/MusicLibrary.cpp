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
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    const QString databaseRestorePoint=dal->createRestorePoint();
    qDebug() << SB_DEBUG_INFO << databaseRestorePoint;

    //	Important lists: all have `path' as key
    QMap<QString,MLperformancePtr> pathToSong;
    QMap<QString,MLentryPtr> foundPaths;
    QVector<QString> newEntries;
    QVector<MLentryPtr> skippedEntries;

    QMap<QString,MLperformerPtr> performersFound; //	key:performerName
    QMap<QString,MLalbumPtr> albumsFound;	//	key:album path	//	CWIP: shouldn't key be albumTitle, albumPerformerID
    QMap<QString,MLsongPerformancePtr> songsFound;

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
    ///	Section A:	Retrieve existing data
    ///////////////////////////////////////////////////////////////////////////////////

    //	For the next sections, set up a progress bar.
    int maxValue=numFiles;
    QProgressDialog pd("Starting",QString(),0,maxValue);
    pd.setWindowModality(Qt::WindowModal);
    pd.show();
    pd.raise();
    pd.activateWindow();

    //	1.	Retrieve existing paths
    SBSqlQueryModel* sqm=SBIDAlbumPerformance::onlinePerformances();

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

    //	newEntries
    for(int i=0;i<newEntries.count();i++)
    {
        QString path=newEntries.at(i);
        MLentryPtr entryPtr=foundPaths[path];
        qDebug() << SB_DEBUG_INFO << "found: " << path
                 << ":performer=" << entryPtr->songPerformerName
                 << ":songTitle=" << entryPtr->songTitle
                 << ":albumTitle=" << entryPtr->albumTitle
                 << ":genre=" << entryPtr->genre
        ;
    }

    ///////////////////////////////////////////////////////////////////////////////////
    ///	Section B:	Validation and selection of the big three:
    /// 	-	Performers,
    /// 	-	Albums,
    /// 	-	Songs.
    ///////////////////////////////////////////////////////////////////////////////////

    //	1.	Validate performers
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
            dal->restore(databaseRestorePoint);
            return;
        }
        foundPerformerPtr->performerID=selectedPerformerPtr->performerID();

        for(int i=0;i<foundPerformerPtr->paths.count();i++)
        {
            qDebug() << SB_DEBUG_INFO << foundPerformerPtr->name << foundPerformerPtr->performerID << foundPerformerPtr->paths[i];
            //	Set performerID on new entries
            MLentryPtr entryPtr=foundPaths[foundPerformerPtr->paths[i]];
            entryPtr->performerPtr=selectedPerformerPtr;
        }
    }

    //	2.	Validate albums

    //	Collect all unique albumPaths
    SBIDPerformerPtr variousPerformerPtr=SBIDPerformer::retrieveVariousArtists();
    for(int i=0;i<newEntries.count();i++)
    {
        const QString currentPath=newEntries[i];
        QFileInfo songFile=schemaRoot+'/'+currentPath;
        MLentryPtr currentPtr=foundPaths[currentPath];

        //	CWIP: need to take currentPtr->albumTitle into account
        currentPtr->dirName=currentPath.left(currentPath.length()-songFile.fileName().length()-1);

        if(!albumsFound.contains(currentPtr->dirName))
        {
            MLalbum album;

            album.path=currentPtr->dirName;
            album.albumPerformerID=currentPtr->performerPtr->performerID();	//	Assign songPerformerID initially
            album.title=currentPtr->albumTitle;
            album.paths.append(currentPath);
            album.year=currentPtr->year;
            album.genre=currentPtr->genre;

            qDebug() << SB_DEBUG_INFO << album.genre;
            albumsFound[currentPtr->dirName]=std::make_shared<MLalbum>(album);
        }
        else
        {
            MLalbumPtr albumPtr=albumsFound[currentPtr->dirName];
            albumPtr->paths.append(currentPath);
            if(albumPtr->albumPerformerID!=currentPtr->performerPtr->performerID())
            {
                albumPtr->albumPerformerID=variousPerformerPtr->performerID();	//	If existing does not match, overwrite with variousPerformers
            }
            if(albumPtr->year!=1900 and albumPtr->year<currentPtr->year)
            {
                albumPtr->year=currentPtr->year;
            }
            if(albumPtr->genre.length()==0 && currentPtr->genre.length()!=0)
            {
                albumPtr->genre=currentPtr->genre;
            }
            qDebug() << SB_DEBUG_INFO << albumPtr->genre;
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
        parameters.year=foundAlbumPtr->year;
        parameters.genre=foundAlbumPtr->genre;
        qDebug() << SB_DEBUG_INFO << parameters.genre;

        qDebug() << SB_DEBUG_INFO << parameters.albumTitle << parameters.performerID;
        SBIDAlbumPtr selectedAlbumPtr=amgr->userMatch(parameters,SBIDAlbumPtr());
        if(!selectedAlbumPtr)
        {
            qDebug() << SB_DEBUG_INFO << "none selected -- exit from import";
            dal->restore(databaseRestorePoint);
            return;
        }
        qDebug() << SB_DEBUG_INFO << selectedAlbumPtr->key() << selectedAlbumPtr->genericDescription();
        foundAlbumPtr->albumID=selectedAlbumPtr->albumID();
        foundAlbumPtr->offset=selectedAlbumPtr->numPerformances();
        foundAlbumPtr->albumPtr=selectedAlbumPtr;

        for(int i=0;i<foundAlbumPtr->paths.count();i++)
        {
            //	Set album fields
            MLentryPtr entryPtr=foundPaths[foundAlbumPtr->paths[i]];
            entryPtr->albumPtr=selectedAlbumPtr;
        }
    }

    //	3.	Validate songs
    for(int i=0;i<newEntries.count();i++)
    {
        const QString currentPath=newEntries[i];
        MLentryPtr currentPtr=foundPaths[currentPath];

        MLsongPerformance song;
        song.performerID=currentPtr->performerPtr->performerID();
        song.songTitle=currentPtr->songTitle;
        song.songPerformerName=currentPtr->songPerformerName;
        song.year=currentPtr->year;
        song.notes=currentPtr->notes;

        QString key=song.key();

        if(!songsFound.contains(key))
        {
            song.paths.append(currentPath);
            songsFound[key]=std::make_shared<MLsongPerformance>(song);
        }
        else
        {
            MLsongPerformancePtr songPtr=songsFound[key];
            songPtr->paths.append(currentPath);
        }
    }

    QMapIterator<QString,MLsongPerformancePtr> sfIT(songsFound);
    while(sfIT.hasNext())
    {
        sfIT.next();

        MLsongPerformancePtr foundSongPtr=sfIT.value();

        Common::sb_parameters parameters;
        parameters.songTitle=foundSongPtr->songTitle;
        parameters.performerID=foundSongPtr->performerID;
        parameters.performerName=foundSongPtr->songPerformerName;
        parameters.year=foundSongPtr->year;
        parameters.notes=foundSongPtr->notes;

        qDebug() << SB_DEBUG_INFO << "CREATENEWSONG";
        qDebug() << SB_DEBUG_INFO << sfIT.key() << foundSongPtr->songTitle << foundSongPtr->performerID;
        SBIDSongPtr selectedSongPtr=smgr->userMatch(parameters,SBIDSongPtr());
        qDebug() << SB_DEBUG_INFO << "exit now";
        if(!selectedSongPtr)
        {
            qDebug() << SB_DEBUG_INFO << "none selected -- exit from import";
            dal->restore(databaseRestorePoint);
            return;
        }

        //	At this point, original song is selected.
        foundSongPtr->songID=selectedSongPtr->songID();

        for(int i=0;i<foundSongPtr->paths.count();i++)
        {
            //	Set song fields
            MLentryPtr entryPtr=foundPaths[foundSongPtr->paths[i]];
            entryPtr->songPtr=selectedSongPtr;

            //	Add song performance
            if(entryPtr->songPtr->songPerformerID()!=entryPtr->performerPtr->performerID())
            {
                qDebug() << SB_DEBUG_INFO;
                entryPtr->songPtr->addSongPerformance(entryPtr->performerPtr->performerID(),entryPtr->year,entryPtr->notes);
            }
            if(smgr->commit(entryPtr->songPtr,dal)==0)
            {
                //	something happened -- error out
                qDebug() << SB_DEBUG_ERROR << "No go. Error out";
                dal->restore(databaseRestorePoint);
                return;
            }
        }
        qDebug() << SB_DEBUG_INFO;
    }

    ///////////////////////////////////////////////////////////////////////////////////
    ///	Section C:	Populate database.
    /// At this point, songs, performers, performances and albums are populated.
    /// Actual album and album_performance tables need to be populated.
    ///////////////////////////////////////////////////////////////////////////////////

    //	Sanity check
    qDebug() << SB_DEBUG_INFO << "SANITYCHECK";
    for(int i=0;i<newEntries.count();i++)
    {
        const QString currentPath=newEntries[i];
        if(foundPaths.contains(currentPath))
        {
            MLentryPtr currentPtr=foundPaths[currentPath];
            qDebug() << SB_DEBUG_INFO << currentPtr->path
                     << currentPtr->performerPtr->performerID()
                     << currentPtr->albumPerformerID
                     << currentPtr->albumPtr->albumID()
                     << currentPtr->albumPosition
                     << currentPtr->songPtr->songID()
                     << currentPtr->genre
            ;
        }
        else
        {
            //	CWIP: error dialog box
            qDebug() << SB_DEBUG_ERROR << "not found!" << currentPath;
        }
    }

    //	Iterate through new albums
    afIT.toFront();
    while(afIT.hasNext())
    {
        afIT.next();
        MLalbumPtr albumPtr=afIT.value();
        SBIDAlbumPtr selectedAlbumPtr=albumPtr->albumPtr;
        QMap<int,SBIDAlbumPerformancePtr> performances=selectedAlbumPtr->performanceList();

        for(int i=0;i<albumPtr->paths.count();i++)
        {
            MLentryPtr entryPtr=foundPaths[albumPtr->paths.at(i)];
            if(performances.contains(entryPtr->albumPosition))
            {
                //	Already exists -- move to skipped list
                skippedEntries.append(entryPtr);
            }
            else
            {
                //	Add!
                qDebug() << SB_DEBUG_INFO;
                selectedAlbumPtr->addAlbumPerformance(entryPtr->songPtr->songID(),entryPtr->performerPtr->performerID(),entryPtr->albumPosition,entryPtr->year,entryPtr->path,entryPtr->duration,entryPtr->notes);
            }
        }
        if(amgr->commit(selectedAlbumPtr,dal)!=1)
        {
            //	something happened -- error out
            qDebug() << SB_DEBUG_ERROR << "No go. Error out";
            dal->restore(databaseRestorePoint);
            return;
        }
    }

    //	Refresh caches
    Context::instance()->getController()->refreshModels();

    qDebug() << SB_DEBUG_INFO << "Finished";
    return;
}
