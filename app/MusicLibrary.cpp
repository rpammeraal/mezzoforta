#include <QDir>
#include <QProgressDialog>

#include "MusicLibrary.h"

#include "AudioDecoderFactory.h"
#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "MetaData.h"
#include "MusicImportResult.h"
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
    /*
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    const QString databaseRestorePoint=dal->createRestorePoint();
    const QString schema=dal->schema();

    //	Important lists: all have `path' as key (in lowercase)
    QVector<MLentityPtr> foundEntities;
    QHash<QString,MLperformancePtr> pathToSong;	//	existing songs as known in database

    //	SBIDManager
    SBIDAlbumMgr* amgr=Context::instance()->getAlbumMgr();

    //	Init
    int progressValue=0;
    const QString schemaRoot=
        Context::instance()->getProperties()->musicLibraryDirectory()
        +"/"
        +schema
        +(schema.length()?"/":"");

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
    int ID=9999;
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
            e.ID=ID++;
            e.filePath=path;
            e.parentDirectoryPath=fi.absoluteDir().absolutePath();
            e.parentDirectoryName=fi.absoluteDir().dirName();
            e.extension=fi.completeSuffix();

            if(AudioDecoderFactory::fileSupportedFlag(fi))
            {
                if(fi.size()>10*1024)
                {
                    numFiles++;
                }
                else
                {
                    e.errorMsg=QString("File size too short: %1 bytes").arg(fi.size());
                }
                foundEntities.append(std::make_shared<MLentity>(e));
            }
        }
    }

    c->updateStatusBarText(QString("Found %1 files").arg(numFiles));
    QCoreApplication::processEvents();

    {	//	DEBUG
        QVectorIterator<MLentityPtr> eIT(foundEntities);
        qDebug() << SB_DEBUG_INFO << "SECTION A";
        while(eIT.hasNext())
        {
            MLentityPtr e=eIT.next();
            if(e->errorFlag()==0)
            {
                qDebug() << SB_DEBUG_INFO
                         << e->ID
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
            if(e->errorFlag())
            {
                qDebug() << SB_DEBUG_INFO << "NOT IMPORTED"
                         << e->ID
                         << e->errorMsg.length()
                         << e->filePath
                         << e->extension
                         << e->errorMsg
                ;
            }
        }
    }

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

    SBSqlQueryModel* sqm=SBIDOnlinePerformance::retrieveAllOnlinePerformances();
    importProgressDialog.setMaximum(sqm->rowCount());

    importProgressDialog.setValue(0);
    importProgressDialog.setMaximum(maxValue);
    importProgressDialog.setLabelText("Retrieving existing songs...");
    QCoreApplication::processEvents();

    QHash<QString,bool> existingPath;
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
    importProgressDialog.setLabelText("Retrieve meta data");
    progressValue=0;

    QMutableVectorIterator<MLentityPtr> feIT(foundEntities);
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

            //	Check if all primary meta data attributes are populated
            if(entityPtr->albumPosition<0 && entityPtr->createArtificialAlbumFlag!=0)
            {
                entityPtr->errorMsg="Missing album position in meta data";
            }
            else if(entityPtr->albumTitle.length()==0)
            {
                entityPtr->errorMsg="Missing album title in meta data";
            }
            else if(entityPtr->songPerformerName.length()==0)
            {
                entityPtr->errorMsg="Missing performer name in meta data";
            }
            else if(entityPtr->songTitle.length()==0)
            {
                entityPtr->errorMsg="Missing song title in meta data";
            }
        }
        if((progressValue%100)==0)
        {
            importProgressDialog.setValue(progressValue);
            QCoreApplication::processEvents();
        }
        progressValue++;
    }
    importProgressDialog.setValue(maxValue);

    {	//	DEBUG
        QVectorIterator<MLentityPtr> eIT(foundEntities);
        qDebug() << SB_DEBUG_INFO << "SECTION C";
        int ok=0;
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
                ok++;
            }

        }

        int incorrect=0;
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
                incorrect++;
            }
        }
        qDebug() << SB_DEBUG_INFO << ok << "correct records";
        qDebug() << SB_DEBUG_INFO << incorrect << "incorrect records";
    }

    ///////////////////////////////////////////////////////////////////////////////////
    ///	Section D:	Validation and selection of the big three:
    /// 	-	Performers,
    /// 	-	Albums,
    /// 	-	Songs.
    ///////////////////////////////////////////////////////////////////////////////////
    if(validateEntityList(foundEntities,&importProgressDialog)==0)
    {
        dal->restore(databaseRestorePoint);
    }

    ///////////////////////////////////////////////////////////////////////////////////
    ///	Section E:	Populate database.
    /// At this point, songs, performers, performances and albums are populated.
    /// Actual album and album_performance tables need to be populated.
    ///////////////////////////////////////////////////////////////////////////////////

    //	1.	Sanity check
    qDebug() << SB_DEBUG_INFO << "SANITYCHECK";
    feIT.toFront();
    while(feIT.hasNext())
    {
        MLentityPtr entityPtr=feIT.next();

        if(!entityPtr->errorFlag())
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

        if(!entityPtr->errorFlag())
        {
            qDebug() << SB_DEBUG_INFO;
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
    if(amgr->commitAll(dal,"Updating Database")==0)
    {
        dal->restore(databaseRestorePoint);
    }
    qDebug() << SB_DEBUG_INFO;

    //	Collect all errors
    QMap<QString,QString> errors;
    feIT.toFront();
    while(feIT.hasNext())
    {
        MLentityPtr entityPtr=feIT.next();

        if(entityPtr->errorFlag())
        {
            errors[entityPtr->filePath]=entityPtr->errorMsg;
        }
    }
    if(errors.count())
    {
        MusicImportResult mir(errors);
        mir.exec();
    }

    //	Refresh caches
    Context::instance()->getController()->refreshModels();
    Context::instance()->getController()->preloadAllSongs();

    qDebug() << SB_DEBUG_INFO << "Finished";
    return;
    */
}

bool
MusicLibrary::validateEntityList(QVector<MLentityPtr>& list)
{
    SBIDAlbumMgr* amgr=Context::instance()->getAlbumMgr();
    SBIDPerformerMgr* pemgr=Context::instance()->getPerformerMgr();
    SBIDSongMgr* smgr=Context::instance()->getSongMgr();
    Properties* properties=Context::instance()->getProperties();
    SBIDPerformerPtr variousPerformerPtr=SBIDPerformer::retrieveVariousPerformers();
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    int progressCurrentValue=0;
    int progressMaxValue=0;


    //	1.	Validate performers
    QMutableVectorIterator<MLentityPtr> feIT(list);
    QHash<QString,int> name2PerformerIDMap;
    feIT.toFront();

    progressCurrentValue=0;
    progressMaxValue=list.count();
    ProgressDialog::instance()->update("MusicLibrary::validateEntityList",progressCurrentValue,progressMaxValue);

    while(feIT.hasNext())
    {
        MLentityPtr entityPtr=feIT.next();

        if(entityPtr)
        {
            qDebug() << SB_DEBUG_INFO
                     << entityPtr->songTitle
                     << entityPtr->songPerformerName
            ;
            if(!(entityPtr->errorFlag()) && !(entityPtr->removedFlag))
            {
                int performerID=-1;
                if(!name2PerformerIDMap.contains(entityPtr->songPerformerName))
                {
                    SBIDPerformerPtr selectedPerformerPtr;
                    Common::sb_parameters tobeMatched;
                    tobeMatched.performerName=entityPtr->songPerformerName;
                    tobeMatched.performerID=-1;
                    selectedPerformerPtr=pemgr->userMatch(tobeMatched,SBIDPerformerPtr());
                    if(!selectedPerformerPtr)
                    {
                        qDebug() << SB_DEBUG_INFO << "none selected -- exit from import";
                        return 0;
                    }
                    performerID=selectedPerformerPtr->performerID();
                    name2PerformerIDMap[entityPtr->songPerformerName]=performerID;
                }
                else
                {
                    performerID=name2PerformerIDMap[entityPtr->songPerformerName];
                }
                entityPtr->songPerformerID=performerID;
                entityPtr->albumPerformerID=performerID;
            }
        }
        ProgressDialog::instance()->update("MusicLibrary::validateEntityList",progressCurrentValue++,progressMaxValue);
    }

    {	//	DEBUG
        QVectorIterator<MLentityPtr> eIT(list);
        qDebug() << SB_DEBUG_INFO << "SECTION D1";
        while(eIT.hasNext())
        {
            MLentityPtr e=eIT.next();
            if(e && e->errorFlag()==0)
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

    //	a.	Handle abums with multiple artists
    if(properties->configValue(Properties::sb_performer_album_directory_structure_flag)=="1")
    {
        //	If music library is organized <performer>/<album>, we'll need to detect
        //	self made/collection albums. For these albums:
        //	-	assign parent directory name as the album title
        //	-	renumber album positions

        //	This maps a parent directory path to an MLalbumPtr
        QHash<QString,MLalbumPathPtr> directory2AlbumPathMap;
        directory2AlbumPathMap.reserve(list.count());

        progressCurrentValue=0;
        progressMaxValue=list.count();
        ProgressDialog::instance()->update("MusicLibrary::validateEntityList",progressCurrentValue,progressMaxValue);

        //	Collect unique album titles for each parent directory
        feIT.toFront();
        while(feIT.hasNext())
        {
            MLentityPtr entityPtr=feIT.next();

            if(entityPtr)
            {
                if(!entityPtr->errorFlag() && !(entityPtr->removedFlag))
                {
                    const QString key=entityPtr->parentDirectoryPath;

                    if(!directory2AlbumPathMap.contains(key))
                    {
                        MLalbumPath albumPath;
                        albumPath.uniqueAlbumTitles.append(entityPtr->albumTitle);
                        albumPath.uniqueSongPerformerIDs.append(entityPtr->songPerformerID);

                        directory2AlbumPathMap[key]=std::make_shared<MLalbumPath>(albumPath);
                    }
                    else
                    {
                        MLalbumPathPtr albumPathPtr=directory2AlbumPathMap[key];

                        if(!albumPathPtr->uniqueAlbumTitles.contains(entityPtr->albumTitle))
                        {
                            albumPathPtr->uniqueAlbumTitles.append(entityPtr->albumTitle);
                        }
                        if(!albumPathPtr->uniqueSongPerformerIDs.contains(entityPtr->songPerformerID))
                        {
                            albumPathPtr->uniqueSongPerformerIDs.append(entityPtr->songPerformerID);
                        }
                    }
                }
            }
            ProgressDialog::instance()->update("MusicLibrary::validateEntityList",progressCurrentValue++,progressMaxValue);
        }

        progressCurrentValue=0;
        progressMaxValue=list.count();
        ProgressDialog::instance()->update("MusicLibrary::validateEntityList",progressCurrentValue,progressMaxValue);

        feIT.toFront();
        while(feIT.hasNext())
        {
            MLentityPtr entityPtr=feIT.next();

            if(entityPtr)
            {
                if(!entityPtr->errorFlag() && !(entityPtr->removedFlag))
                {
                    MLalbumPathPtr albumPathPtr=directory2AlbumPathMap[entityPtr->parentDirectoryPath];
                    if(albumPathPtr->multipleEntriesFlag())
                    {
                        entityPtr->albumTitle=entityPtr->parentDirectoryName;
                        entityPtr->albumPosition=++(albumPathPtr->maxPosition);
                        entityPtr->albumPerformerID=variousPerformerPtr->performerID();
                        entityPtr->albumPerformerName=variousPerformerPtr->performerName();
                        entityPtr->createArtificialAlbumFlag=1;
                    }
                }
            }
            ProgressDialog::instance()->update("MusicLibrary::validateEntityList",progressCurrentValue++,progressMaxValue);
        }
    }
    //	CWIP
//    else
//    {
//        //	b.	For unorganized libraries, we'll need to depend on the album title in the meta data.
//        //		This will mean that if we encounter two different performers with the same album name,
//        //		that all entries will be all merged into one big album.
//        feIT.toFront();
//        while(feIT.hasNext())
//        {
//            MLentityPtr entityPtr=feIT.next();
//            QMap<QString,int> albumTitle2albumIDMap;	//	key: <album title>:<album performer id>
//            SBIDAlbumPtr selectedAlbumPtr;

//            if(!entityPtr->errorFlag())
//            {

//            }
//        }
//    }


    {	//	DEBUG
        QVectorIterator<MLentityPtr> eIT(list);
        qDebug() << SB_DEBUG_INFO << "SECTION D2b";
        while(eIT.hasNext())
        {
            MLentityPtr e=eIT.next();
            if(e && e->errorFlag()==0)
            {
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
    }

    //	c.	Actual user validation of albums
    progressCurrentValue=0;
    progressMaxValue=list.count();
    ProgressDialog::instance()->update("MusicLibrary::validateEntityList",progressCurrentValue,progressMaxValue);

    QStringList greatestHitsAlbums=_greatestHitsAlbums();

    feIT.toFront();
    QHash<QString,int> albumTitle2albumIDMap;	//	key: <album title>:<album performer id>
    while(feIT.hasNext())
    {
        MLentityPtr entityPtr=feIT.next();
        SBIDAlbumPtr selectedAlbumPtr;

        if(entityPtr && !entityPtr->errorFlag() && (!entityPtr->removedFlag))
        {
            const QString key=QString("%1:%2").arg(entityPtr->albumTitle).arg(entityPtr->albumPerformerID);
            if(!albumTitle2albumIDMap.contains(key))
            {
                if(entityPtr->createArtificialAlbumFlag || greatestHitsAlbums.contains(entityPtr->albumTitle))
                {
                    selectedAlbumPtr=amgr->createInDB();
                    selectedAlbumPtr->setAlbumTitle(entityPtr->albumTitle);
                    selectedAlbumPtr->setAlbumPerformerID(entityPtr->albumPerformerID);
                    selectedAlbumPtr->setYear(entityPtr->year);
                    selectedAlbumPtr->setGenre(entityPtr->genre);

                    qDebug() << SB_DEBUG_INFO << "CRT ALBUM:" << selectedAlbumPtr->albumTitle() << selectedAlbumPtr->albumPerformerID();
                    amgr->commit(selectedAlbumPtr,dal,0);
                }
                else
                {
                    //	Let user select
                    Common::sb_parameters parameters;
                    parameters.albumTitle=entityPtr->albumTitle;
                    parameters.performerName=entityPtr->albumPerformerName;
                    parameters.performerID=entityPtr->albumPerformerID;
                    parameters.year=entityPtr->year;
                    parameters.genre=entityPtr->genre;

                    qDebug() << SB_DEBUG_INFO
                             << parameters.albumTitle
                             << parameters.performerName
                             << parameters.performerID
                             << parameters.year
                             << parameters.genre
                    ;
                    selectedAlbumPtr=amgr->userMatch(parameters,SBIDAlbumPtr());
                    qDebug() << SB_DEBUG_INFO;
                    if(!selectedAlbumPtr)
                    {
                        qDebug() << SB_DEBUG_INFO << "none selected -- exit from import";
                        return 0;
                    }
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
        ProgressDialog::instance()->update("MusicLibrary::validateEntityList",progressCurrentValue++,progressMaxValue);
    }

    {	//	DEBUG
        QVectorIterator<MLentityPtr> eIT(list);
        qDebug() << SB_DEBUG_INFO << "SECTION D2c";
        while(eIT.hasNext())
        {
            MLentityPtr e=eIT.next();
            if(e && !e->errorFlag())
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
    progressCurrentValue=0;
    progressMaxValue=list.count();
    ProgressDialog::instance()->update("MusicLibrary::validateEntityList",progressCurrentValue,progressMaxValue);
    feIT.toFront();

qDebug() << SB_DEBUG_INFO;
    QHash<QString,int> songTitle2songIDMap;	//	key: <song title>:<song performer id>
    songTitle2songIDMap.reserve(list.count());
    while(feIT.hasNext())
    {
qDebug() << SB_DEBUG_INFO;
        MLentityPtr entityPtr=feIT.next();

        if(entityPtr)
        {
qDebug() << SB_DEBUG_INFO;
            if(!entityPtr->errorFlag() && !(entityPtr->removedFlag))
            {
qDebug() << SB_DEBUG_INFO;
                SBIDSongPtr selectedSongPtr;

                const QString key=QString("%1:%2").arg(entityPtr->songTitle).arg(entityPtr->songPerformerID);
                if(!songTitle2songIDMap.contains(key))
                {
qDebug() << SB_DEBUG_INFO;
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
                        return 0;
                    }

                    songTitle2songIDMap[key]=selectedSongPtr->songID();
                }
                else
                {
                    selectedSongPtr=smgr->retrieve(SBIDSong::createKey(songTitle2songIDMap[key]));
                }

                //	Populate entity with attributes from selected song
qDebug() << SB_DEBUG_INFO;
                entityPtr->songID=selectedSongPtr->songID();

                //	Add performance
                if(entityPtr->songPerformerID!=selectedSongPtr->songOriginalPerformerID())
                {
                    selectedSongPtr->addSongPerformance(entityPtr->songPerformerID,entityPtr->year,entityPtr->notes);
                    qDebug() << SB_DEBUG_INFO;
                    if(smgr->commit(selectedSongPtr,dal,0)==0)
                    {
                        //	something happened -- error out
                        qDebug() << SB_DEBUG_ERROR << "No go. Error out";
                        return 0;
                    }
                }
            }
        }
        ProgressDialog::instance()->update("MusicLibrary::validateEntityList",progressCurrentValue++,progressMaxValue);
    }
    ProgressDialog::instance()->finishStep("MusicLibrary::validateEntityList");
    return 1;
}

QStringList
MusicLibrary::_greatestHitsAlbums() const
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QStringList greatestHitsAlbums;
    QString q="SELECT title FROM greatest_hits_record";
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery qID(q,db);
    while(qID.next())
    {
        greatestHitsAlbums.append(qID.value(0).toString());
    }
    return greatestHitsAlbums;
}
