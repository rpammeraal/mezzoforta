#include <QDir>
#include <QProgressDialog>

#include "MusicLibrary.h"

#include "AudioDecoderFactory.h"
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
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    Properties* properties=Context::instance()->getProperties();
    const QString databaseRestorePoint=dal->createRestorePoint();
    const QString schema=dal->schema();
    SBIDPerformerPtr variousPerformerPtr=SBIDPerformer::retrieveVariousPerformers();

    //	Important lists: all have `path' as key (in lowercase)
    QVector<MLentityPtr> foundEntities;
    QMap<QString,MLperformancePtr> pathToSong;	//	existing songs as known in database

    //	SBIDManager
    SBIDPerformerMgr* pemgr=Context::instance()->getPerformerMgr();
    SBIDAlbumMgr* amgr=Context::instance()->getAlbumMgr();
    SBIDSongMgr* smgr=Context::instance()->getSongMgr();

    //	Init
    const QString schemaRoot=
        Context::instance()->getProperties()->musicLibraryDirectory()
        +"/"
        +schema
        +(schema.length()?"/":"");
    qDebug() << SB_DEBUG_INFO << schemaRoot;

    ///////////////////////////////////////////////////////////////////////////////////
    ///	Section A:	Retrieve paths found in directory
    ///////////////////////////////////////////////////////////////////////////////////
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
            path=path.mid(schemaRoot.length());
            MLentity e;
            e.filePath=path;
            e.parentDirectoryPath=fi.absoluteDir().absolutePath();
            e.parentDirectoryName=fi.absoluteDir().dirName();
            e.extension=fi.completeSuffix();

            if(fi.size()>10*1024)
            {
                if(AudioDecoderFactory::fileSupportedFlag(fi))
                {
                    numFiles++;
                }
                else
                {
                    e.errorMsg=QString("Unsupported file extension: '%1'").arg(e.extension);
                }
            }
            else
            {
                e.errorMsg=QString("File size too short: %1 bytes").arg(fi.size());
            }
            foundEntities.append(std::make_shared<MLentity>(e));
        }
    }
    c->updateStatusBarText(QString("Found %1 files").arg(numFiles));
    QCoreApplication::processEvents();

    {	//	DEBUG
        QVectorIterator<MLentityPtr> eIT(foundEntities);
        qDebug() << SB_DEBUG_INFO;
        while(eIT.hasNext())
        {
            MLentityPtr e=eIT.next();
            if(e->errorFlag()==0)
            {
                qDebug() << SB_DEBUG_INFO
                         << e->filePath
                         << e->parentDirectoryName
                         << e->parentDirectoryPath
                         << e->extension
                ;
            }
        }
        eIT.toFront();
        while(eIT.hasNext())
        {
            MLentityPtr e=eIT.next();
            if(e->errorFlag()!=0)
            {
                qDebug() << SB_DEBUG_INFO << "NOT IMPORTED"
                         << e->filePath
                         << e->extension
                         << e->errorMsg
                ;
            }
        }
    }
    qDebug() << SB_DEBUG_INFO;

    ///////////////////////////////////////////////////////////////////////////////////
    ///	Section B:	Retrieve existing data
    ///////////////////////////////////////////////////////////////////////////////////

    //	For the next sections, set up a progress bar.
    int maxValue=numFiles;
    QProgressDialog importProgressDialog("Starting",QString(),0,maxValue);
    importProgressDialog.setWindowModality(Qt::WindowModal);
    importProgressDialog.show();
    importProgressDialog.raise();
    importProgressDialog.activateWindow();

    SBSqlQueryModel* sqm=SBIDAlbumPerformance::onlinePerformances();
    importProgressDialog.setMaximum(sqm->rowCount());

    qDebug() << SB_DEBUG_INFO << sqm->rowCount();
    importProgressDialog.setValue(0);
    importProgressDialog.setMaximum(maxValue);
    importProgressDialog.setLabelText("Retrieving existing songs...");
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

        QString pathToSongKey=performance.path.toLower();
        pathToSong[pathToSongKey]=std::make_shared<MLperformance>(performance);
        existingPath[pathToSongKey]=0;

        if((i%100)==0)
        {
            importProgressDialog.setValue(i);
            QCoreApplication::processEvents();
        }
    }


    ///////////////////////////////////////////////////////////////////////////////////
    ///	Section C:	Determine new songs and retrieve meta data for these
    ///////////////////////////////////////////////////////////////////////////////////
    importProgressDialog.setValue(0);
    maxValue=foundEntities.count();
    importProgressDialog.setMaximum(maxValue);
    importProgressDialog.setLabelText("Retrieve meta data...");
    QMutableVectorIterator<MLentityPtr> feIT(foundEntities);
    int i=0;
    while(feIT.hasNext())
    {
        MLentityPtr entityPtr=feIT.next();

        const QString key=entityPtr->filePath.toLower();

        if(pathToSong.contains(key))
        {
            pathToSong[key]->pathExists=1;
            feIT.remove(); //	Gone, gone, gone. Not needed. Exists. Pleitte.
        }
        else if(entityPtr->errorFlag()==0)
        {
            //	Populate meta data
            MetaData md(schemaRoot+entityPtr->filePath);

            //	Primary meta data
            entityPtr->albumPosition=md.albumPosition();
            entityPtr->albumTitle=md.albumTitle();
            entityPtr->songPerformerName=md.songPerformerName();
            entityPtr->songTitle=md.songTitle();

            qDebug() << SB_DEBUG_INFO << entityPtr->songTitle << md.songTitle();

            //	Secondary meta data
            entityPtr->albumPerformerName=entityPtr->songPerformerName; // for now, default to <>
            entityPtr->duration=md.duration();
            entityPtr->genre=md.genre();
            entityPtr->notes=md.notes();
            entityPtr->year=md.year();

            //	Check on album title, take parent directory name if not exists
            if(entityPtr->albumTitle.length()==0)
            {
                //	Take the name of the parent directory as the album title.
                entityPtr->albumTitle=entityPtr->parentDirectoryName;
                entityPtr->createArtificialAlbumFlag=1;
            }
        }
        if((i%100)==0)
        {
            qDebug() << SB_DEBUG_INFO << i;
            importProgressDialog.setValue(i);
            QCoreApplication::processEvents();
        }
        i++;
    }
    importProgressDialog.setValue(maxValue);
    importProgressDialog.close();

    {	//	DEBUG
        QVectorIterator<MLentityPtr> eIT(foundEntities);
        qDebug() << SB_DEBUG_INFO;
        while(eIT.hasNext())
        {
            MLentityPtr e=eIT.next();
            if(e->errorFlag()==0)
            {
                qDebug() << SB_DEBUG_INFO
                         << e->filePath
                         << e->songTitle
                         << e->songPerformerName
                         << e->albumTitle
                         << e->albumPosition
                ;
            }
        }
        eIT.toFront();
        while(eIT.hasNext())
        {
            MLentityPtr e=eIT.next();
            if(e->errorFlag()!=0)
            {
                qDebug() << SB_DEBUG_INFO << "NOT IMPORTED"
                         << e->filePath
                         << e->songTitle
                         << e->songPerformerName
                         << e->albumTitle
                         << e->albumPosition
                ;
            }
        }
    }
    qDebug() << SB_DEBUG_INFO;

    ///////////////////////////////////////////////////////////////////////////////////
    ///	Section D:	Validation and selection of the big three:
    /// 	-	Performers,
    /// 	-	Albums,
    /// 	-	Songs.
    ///////////////////////////////////////////////////////////////////////////////////

    //	1.	Validate performers
    QMap<QString,SBIDPerformerPtr> name2PerformerMap;
    SBIDPerformerPtr selectedPerformerPtr;
    feIT.toFront();
    while(feIT.hasNext())
    {
        MLentityPtr entityPtr=feIT.next();

        if(entityPtr->isValid())
        {
            if(!name2PerformerMap.contains(entityPtr->songPerformerName))
            {
                Common::sb_parameters tobeMatched;
                tobeMatched.performerName=entityPtr->songPerformerName;
                tobeMatched.performerID=-1;
                selectedPerformerPtr=pemgr->userMatch(tobeMatched,SBIDPerformerPtr());
                if(!selectedPerformerPtr)
                {
                    qDebug() << SB_DEBUG_INFO << "none selected -- exit from import";
                    dal->restore(databaseRestorePoint);
                    return;
                }
                name2PerformerMap[entityPtr->songPerformerName]=selectedPerformerPtr;
            }
            else
            {
                selectedPerformerPtr=name2PerformerMap[entityPtr->songPerformerName];
            }
            entityPtr->songPerformerID=selectedPerformerPtr->performerID();
            entityPtr->albumPerformerID=selectedPerformerPtr->performerID();
        }
    }

    {	//	DEBUG
        QVectorIterator<MLentityPtr> eIT(foundEntities);
        qDebug() << SB_DEBUG_INFO;
        while(eIT.hasNext())
        {
            MLentityPtr e=eIT.next();
            if(e->errorFlag()==0)
            {
                qDebug() << SB_DEBUG_INFO
                         << e->filePath
                         << e->songPerformerName
                         << e->songPerformerID
                         << e->albumPerformerName
                         << e->albumPerformerID
                ;
            }
        }
    }

    //	2.	Validate albums

    //	a.	If music library is organized <performer>/<album>, we'll need to detect
    //		self made albums. For these albums:
    //		-	assign parent directory name as the album title
    //		-	renumber album positions
    if(properties->configValue(Properties::sb_performer_album_directory_structure_flag)=="1")
    {
        QMap<QString,MLalbumPathPtr> directory2AlbumPathMap;

        //	Collect unique album titles for each parent directory
        feIT.toFront();
        while(feIT.hasNext())
        {
            MLentityPtr entityPtr=feIT.next();

            if(entityPtr->isValid())
            {
                const QString key=entityPtr->parentDirectoryPath;
                if(!directory2AlbumPathMap.contains(key))
                {
                    MLalbumPath albumPath;
                    albumPath.uniqueAlbumTitles.append(entityPtr->albumTitle);

                    directory2AlbumPathMap[key]=std::make_shared<MLalbumPath>(albumPath);
                }
                else
                {
                    MLalbumPathPtr albumPathPtr=directory2AlbumPathMap[key];

                    if(!albumPathPtr->uniqueAlbumTitles.contains(entityPtr->albumTitle))
                    {
                        albumPathPtr->uniqueAlbumTitles.append(entityPtr->albumTitle);
                    }
                }
            }
        }

        //	If there is more than 1 entry, rename album title, renumber positions
        feIT.toFront();
        while(feIT.hasNext())
        {
            MLentityPtr entityPtr=feIT.next();

            if(entityPtr->isValid())
            {
                MLalbumPathPtr albumPathPtr=directory2AlbumPathMap[entityPtr->parentDirectoryPath];
                if(albumPathPtr->uniqueAlbumTitles.count()>1)
                {
                    entityPtr->albumTitle=entityPtr->parentDirectoryName;
                    entityPtr->albumPosition=++(albumPathPtr->maxPosition);
                    entityPtr->albumPerformerID=variousPerformerPtr->performerID();
                    entityPtr->createArtificialAlbumFlag=1;
                }
            }
        }
    }

    //	b.	CWIP: go through all entities within one album and if there is more than one
    //	performer, set albumPerformerID to variousPerformers

    {	//	DEBUG
        QVectorIterator<MLentityPtr> eIT(foundEntities);
        qDebug() << SB_DEBUG_INFO;
        while(eIT.hasNext())
        {
            MLentityPtr e=eIT.next();
            if(e->errorFlag()==0)
            {
                qDebug() << SB_DEBUG_INFO
                         << e->filePath
                         << e->songPerformerName
                         << e->songPerformerID
                         << e->albumTitle
                         << e->albumPosition
                         << e->createArtificialAlbumFlag
                ;
            }
        }
    }

    //	c.	Actual user validation of albums
    feIT.toFront();
    while(feIT.hasNext())
    {
        MLentityPtr entityPtr=feIT.next();
        QMap<QString,int> albumTitle2albumIDMap;	//	key: <album title>:<album performer id>
        SBIDAlbumPtr selectedAlbumPtr;

        if(entityPtr->isValid())
        {
            const QString key=QString("%1:%2").arg(entityPtr->albumTitle).arg(entityPtr->albumPerformerID);
            if(!albumTitle2albumIDMap.contains(key))
            {
                if(entityPtr->createArtificialAlbumFlag)
                {
                    selectedAlbumPtr=amgr->createInDB();
                    selectedAlbumPtr->setAlbumTitle(entityPtr->albumTitle);
                    selectedAlbumPtr->setAlbumPerformerID(entityPtr->albumPerformerID);
                    selectedAlbumPtr->setYear(entityPtr->year);
                    selectedAlbumPtr->setGenre(entityPtr->genre);

                    amgr->commit(selectedAlbumPtr,dal,0);
                }
                else
                {
                    //	Let user select
                    Common::sb_parameters parameters;
                    parameters.albumTitle=entityPtr->albumTitle;
                    parameters.performerID=entityPtr->albumPerformerID;
                    parameters.year=entityPtr->year;
                    parameters.genre=entityPtr->genre;

                    selectedAlbumPtr=amgr->userMatch(parameters,SBIDAlbumPtr());
                    if(!selectedAlbumPtr)
                    {
                        qDebug() << SB_DEBUG_INFO << "none selected -- exit from import";
                        dal->restore(databaseRestorePoint);
                        return;
                    }
                    qDebug() << SB_DEBUG_INFO << selectedAlbumPtr->key() << selectedAlbumPtr->genericDescription();
                }
                albumTitle2albumIDMap[key]=selectedAlbumPtr->albumID();
            }
            else
            {
                selectedAlbumPtr=amgr->retrieve(SBIDAlbum::createKey(albumTitle2albumIDMap[key]));
            }
            entityPtr->albumID=selectedAlbumPtr->albumID();
            entityPtr->albumPerformerID=selectedAlbumPtr->albumPerformerID();
        }
    }

    {	//	DEBUG
        QVectorIterator<MLentityPtr> eIT(foundEntities);
        qDebug() << SB_DEBUG_INFO;
        while(eIT.hasNext())
        {
            MLentityPtr e=eIT.next();
            if(e->errorFlag()==0)
            {
                qDebug() << SB_DEBUG_INFO
                         << e->filePath
                         << e->albumTitle
                         << e->createArtificialAlbumFlag
                         << e->albumID
                         << e->albumPerformerID
                ;
            }
        }
    }

    //	3.	Validate songs
    feIT.toFront();
    while(feIT.hasNext())
    {
        MLentityPtr entityPtr=feIT.next();
        QMap<QString,int> songTitle2songIDMap;	//	key: <song title>:<song performer id>
        SBIDSongPtr selectedSongPtr;

        if(entityPtr->isValid())
        {
            const QString key=QString("%1:%2").arg(entityPtr->songTitle).arg(entityPtr->songPerformerID);
            if(!songTitle2songIDMap.contains(key))
            {
                Common::sb_parameters parameters;
                parameters.songTitle=entityPtr->songTitle;
                parameters.performerID=entityPtr->songPerformerID;
                parameters.performerName=entityPtr->songPerformerName;
                parameters.year=entityPtr->year;
                parameters.notes=entityPtr->notes;

                selectedSongPtr=smgr->userMatch(parameters,SBIDSongPtr());
                if(!selectedSongPtr)
                {
                    qDebug() << SB_DEBUG_INFO << "none selected -- exit from import";
                    dal->restore(databaseRestorePoint);
                    return;
                }

                songTitle2songIDMap[key]=selectedSongPtr->songID();
            }
            else
            {
                selectedSongPtr=smgr->retrieve(SBIDSong::createKey(songTitle2songIDMap[key]));
            }

            //	Populate entity with attributes from selected song
            entityPtr->songID=selectedSongPtr->songID();

            //	Add performance
            if(entityPtr->songPerformerID!=selectedSongPtr->songPerformerID())
            {
                selectedSongPtr->addSongPerformance(entityPtr->songPerformerID,entityPtr->year,entityPtr->notes);
                if(smgr->commit(selectedSongPtr,dal,0)==0)
                {
                    //	something happened -- error out
                    qDebug() << SB_DEBUG_ERROR << "No go. Error out";
                    dal->restore(databaseRestorePoint);
                    return;
                }
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////
    ///	Section D:	Populate database.
    /// At this point, songs, performers, performances and albums are populated.
    /// Actual album and album_performance tables need to be populated.
    ///////////////////////////////////////////////////////////////////////////////////

    //	1.	Sanity check
    qDebug() << SB_DEBUG_INFO << "SANITYCHECK";
    feIT.toFront();
    while(feIT.hasNext())
    {
        MLentityPtr entityPtr=feIT.next();

        if(entityPtr->isValid())
        {
            if(entityPtr->songID==-1)
            {
                entityPtr->errorMsg="songID not populated";
            }
            if(entityPtr->songPerformerID==-1)
            {
                entityPtr->errorMsg="songPerformerID not populated";
            }
            if(entityPtr->albumID==-1)
            {
                entityPtr->errorMsg="albumID not populated";
            }
            if(entityPtr->albumPosition==-1)
            {
                entityPtr->errorMsg="albumPosition not populated";
            }
        }
    }

    //	2.	Save to database
    qDebug() << SB_DEBUG_INFO << "SANITYCHECK";
    feIT.toFront();
    while(feIT.hasNext())
    {
        MLentityPtr entityPtr=feIT.next();

        if(entityPtr->isValid())
        {
            SBIDAlbumPtr albumPtr=amgr->retrieve(SBIDAlbum::createKey(entityPtr->albumID),SBIDAlbumMgr::open_flag_foredit);
            albumPtr->addAlbumPerformance(
                        entityPtr->songID,
                        entityPtr->songPerformerID,
                        entityPtr->albumPosition,
                        entityPtr->year,
                        entityPtr->filePath,
                        entityPtr->duration,
                        entityPtr->notes);
        }
    }
    qDebug() << SB_DEBUG_INFO;
    if(amgr->commitAll1(dal)==0)
    {
        dal->restore(databaseRestorePoint);
    }

    //	Refresh caches
    Context::instance()->getController()->refreshModels();
    qDebug() << SB_DEBUG_INFO;
    Context::instance()->getController()->preloadAllSongs();

    qDebug() << SB_DEBUG_INFO << "Finished";
    return;
}
