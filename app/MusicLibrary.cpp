#include <QDir>
#include <QProgressDialog>

#include "MusicLibrary.h"

#include "AudioDecoderFactory.h"
#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "MetaData.h"
#include "MusicImportResult.h"
#include "ProgressDialog.h"
#include "SBIDAlbum.h"
#include "SBIDOnlinePerformance.h"
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
    const QString databaseRestorePoint=dal->createRestorePoint();

    const QString schema=dal->schema();

    //	Important lists: all have `path' as key (in lowercase)
    QVector<MLentityPtr> foundEntities;
    QHash<QString,MLperformancePtr> pathToSong;	//	existing songs as known in database

    //	SBIDManager
    SBIDAlbumMgr* aMgr=Context::instance()->getAlbumMgr();
    SBIDAlbumPerformanceMgr* apMgr=Context::instance()->getAlbumPerformanceMgr();
    SBIDOnlinePerformanceMgr* opMgr=Context::instance()->getOnlinePerformanceMgr();
    SBIDSongMgr* sMgr=Context::instance()->getSongMgr();
    SBIDSongPerformanceMgr* spMgr=Context::instance()->getSongPerformanceMgr();

    //	Init
    ProgressDialog::instance()->show("Starting","MusicLibrary::rescanMusicLibrary",4);

    const int numOnlinePerformances=SBIDOnlinePerformance::totalNumberOnlinePerformances()+100;
    int progressCurrentValue=0;
    int progressMaxValue=numOnlinePerformances;
    ProgressDialog::instance()->update("MusicLibrary::rescanMusicLibrary_scan",progressCurrentValue,progressMaxValue);

    const QString schemaRoot=
        Context::instance()->getProperties()->musicLibraryDirectory()
        +"/"
        +schema
        +(schema.length()?"/":"");


    ///////////////////////////////////////////////////////////////////////////////////
    ///	Section A:	Retrieve paths found in directory
    ///////////////////////////////////////////////////////////////////////////////////
    int numFiles=0;
    QDirIterator it(schemaRoot,
                    QDir::AllDirs | QDir::AllEntries | QDir::Files | QDir::NoSymLinks | QDir::Readable,
                    QDirIterator::Subdirectories);
    QCoreApplication::processEvents();
    int ID=9999;
    QTime time; time.start();
    while (it.hasNext())
    {
        it.next();
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

            //	Update dialogbox every half a second or so
            if(time.elapsed()>700)
            {
                QString label=e.parentDirectoryName;
                if(label.length()>40)
                {
                    label=label.left(30)+"...";
                }
                label="Scanning album "+label;
                ProgressDialog::instance()->setLabelText(label);
                if(progressCurrentValue+1>progressMaxValue)
                {
                    progressMaxValue=progressCurrentValue+1;
                }
                ProgressDialog::instance()->update("MusicLibrary::rescanMusicLibrary_scan",progressCurrentValue,progressMaxValue);
                time.restart();
                qDebug() << SB_DEBUG_INFO << progressCurrentValue;
            }
            progressCurrentValue++;
        }
    }
    ProgressDialog::instance()->finishStep("MusicLibrary::rescanMusicLibrary_scan");

    if(0)
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

    SBSqlQueryModel* sqm=SBIDOnlinePerformance::retrieveAllOnlinePerformancesExtended();

    progressCurrentValue=0;
    progressMaxValue=sqm->rowCount();
    ProgressDialog::instance()->setLabelText("Retrieving existing songs...");
    ProgressDialog::instance()->update("MusicLibrary::rescanMusicLibrary_retrieve",progressCurrentValue,progressMaxValue);
    time.restart();

    QHash<QString,bool> existingPath;
    for(int i=0;i<sqm->rowCount();i++)
    {
        MLperformance performance;
        performance.songID             =sqm->data(sqm->index(i,0)).toInt();
        performance.songPerformerID    =sqm->data(sqm->index(i,1)).toInt();
        performance.songPerformanceID  =sqm->data(sqm->index(i,2)).toInt();
        performance.albumID            =sqm->data(sqm->index(i,3)).toInt();
        performance.albumPerformerID   =sqm->data(sqm->index(i,4)).toInt();
        performance.albumPosition      =sqm->data(sqm->index(i,5)).toInt();
        performance.albumPerformanceID =sqm->data(sqm->index(i,6)).toInt();
        performance.onlinePerformanceID=sqm->data(sqm->index(i,7)).toInt();
        performance.path=sqm->data(sqm->index(i,8)).toString().replace("\\","");

        QString pathToSongKey=performance.path.toLower();
        pathToSong[pathToSongKey]=std::make_shared<MLperformance>(performance);
        existingPath[pathToSongKey]=0;

        if(time.elapsed()>700)
        {
            QString label=performance.path;
            if(label.length()>40)
            {
                label="..."+label.right(30);
            }
            label="Scanning song "+label;
            ProgressDialog::instance()->setLabelText(label);
            ProgressDialog::instance()->update("MusicLibrary::rescanMusicLibrary_retrieve",progressCurrentValue,progressMaxValue);
            time.restart();
        }
        progressCurrentValue++;
    }
    ProgressDialog::instance()->finishStep("MusicLibrary::rescanMusicLibrary_retrieve");
    qDebug() << SB_DEBUG_INFO;

    ///////////////////////////////////////////////////////////////////////////////////
    ///	Section C:	Determine new songs and retrieve meta data for these
    ///////////////////////////////////////////////////////////////////////////////////
    QMutableVectorIterator<MLentityPtr> feIT(foundEntities);
    int numNewSongs=0;

    progressCurrentValue=0;
    progressMaxValue=foundEntities.count();
    ProgressDialog::instance()->setLabelText("Retrieving meta data...");
    ProgressDialog::instance()->update("MusicLibrary::rescanMusicLibrary_metadata",progressCurrentValue,progressMaxValue);
    time.restart();

    qDebug() << SB_DEBUG_INFO;
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
            qDebug() << SB_DEBUG_INFO << key;

            //	Populate meta data
            MetaData md(schemaRoot+entityPtr->filePath);

            //	Primary meta data
            entityPtr->albumPosition=md.albumPosition();
            entityPtr->albumTitle=md.albumTitle();
            entityPtr->songPerformerName=_retrieveCorrectPerformerName(dal,md.songPerformerName());
            entityPtr->songTitle=md.songTitle();

            //	Secondary meta data
            entityPtr->albumPerformerName=entityPtr->songPerformerName; // for now, default to <>
            entityPtr->duration=md.duration();
            entityPtr->genre=md.genre();
            entityPtr->notes=md.notes();
            entityPtr->year=md.year();

            //	Misc. data
            entityPtr->key=key;	//	used for debugging purposes only

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
            else
            {
                numNewSongs++;
            }
        }

        if(time.elapsed()>700)
        {
            const QString title=QString("Retrieving meta data (%1/%2)").arg(progressCurrentValue).arg(progressMaxValue);
            ProgressDialog::instance()->setLabelText(title);
            ProgressDialog::instance()->update("MusicLibrary::rescanMusicLibrary_metadata",progressCurrentValue,progressMaxValue);
            time.restart();
        }
        progressCurrentValue++;
    }
    ProgressDialog::instance()->finishStep("MusicLibrary::rescanMusicLibrary_metadata");

    if(0)
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
    if(validateEntityList(foundEntities)==0)
    {
        dal->restore(databaseRestorePoint);
        //	CWIP: add dialog box here.
        ProgressDialog::instance()->setLabelText("Canceling");
    }
    else
    {

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
        apMgr->debugShow("BEFORE ADDALBUMPERFORMANCE");
        feIT.toFront();
        while(feIT.hasNext())
        {
            MLentityPtr entityPtr=feIT.next();

            if(!entityPtr->errorFlag())
            {
                qDebug() << SB_DEBUG_INFO;
                SBIDAlbumPtr albumPtr=aMgr->retrieve(SBIDAlbum::createKey(entityPtr->albumID),SBIDAlbumMgr::open_flag_foredit);
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

        bool resultFlag=1;
        qDebug() << SB_DEBUG_INFO;
        if(resultFlag)
        {
            resultFlag=aMgr->commitAll(dal);
        }

        qDebug() << SB_DEBUG_INFO;
        apMgr->debugShow("BEFORE COMMIT");
        if(resultFlag)
        {
            resultFlag=apMgr->commitAll(dal);
        }

        qDebug() << SB_DEBUG_INFO;
        if(resultFlag)
        {
            resultFlag=opMgr->commitAll(dal);
        }

        qDebug() << SB_DEBUG_INFO;
        if(resultFlag)
        {
            resultFlag=sMgr->commitAll(dal);
        }

        qDebug() << SB_DEBUG_INFO;
        if(resultFlag)
        {
            resultFlag=spMgr->commitAll(dal);
        }

        qDebug() << SB_DEBUG_INFO;
        if(resultFlag==0)
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
    }

    //	Refresh caches
    Context::instance()->getController()->refreshModels();
    Context::instance()->getController()->preloadAllSongs();

    ProgressDialog::instance()->finishStep("MusicLibrary::rescanMusicLibrary");
    ProgressDialog::instance()->hide();

    qDebug() << SB_DEBUG_INFO << "Finished";
    return;
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

    qDebug() << SB_DEBUG_INFO;
    {	//	DEBUG
        qDebug() << SB_DEBUG_INFO << "START OF VALIDATION" << list.count();
        QVectorIterator<MusicLibrary::MLentityPtr> eIT(list);
        while(eIT.hasNext())
        {
            MusicLibrary::MLentityPtr e=eIT.next();
            if(e && e->errorFlag()==0)
            {
                qDebug() << SB_DEBUG_INFO
                         << e->songTitle
                         << e->songPerformerName
                         << e->albumTitle
                         << e->albumPosition
                         << e->albumPerformerName
                         << e->mergedToAlbumPosition
                         << e->removedFlag
                ;
            }
            else
            {
                qDebug() << SB_DEBUG_INFO
                         << "NOT DEFINED"
                ;
            }
        }
        qDebug() << SB_DEBUG_INFO << "END";
    }

    //	1.	Validate performers
    QMutableVectorIterator<MLentityPtr> feIT(list);
    QHash<QString,int> name2PerformerIDMap;
    QHash<int,QString> performerID2CorrectNameMap;
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
                qDebug() << SB_DEBUG_INFO
                         << entityPtr->filePath
                ;
                int performerID=-1;
                if(!name2PerformerIDMap.contains(entityPtr->songPerformerName))
                {
                    SBIDPerformerPtr selectedPerformerPtr;
                    Common::sb_parameters p;
                    p.performerName=entityPtr->songPerformerName;
                    p.performerID=-1;
                    Common::result result=pemgr->userMatch(p,SBIDPerformerPtr(),selectedPerformerPtr);
                    if(result==Common::result_canceled)
                    {
                        qDebug() << SB_DEBUG_INFO << "none selected -- exit from import";
                        return 0;
                    }
                    if(result==Common::result_missing)
                    {
                        selectedPerformerPtr=pemgr->createInDB(p);
                    }
                    qDebug() << SB_DEBUG_INFO << selectedPerformerPtr->performerName();
                    performerID=selectedPerformerPtr->performerID();
                    name2PerformerIDMap[entityPtr->songPerformerName]=performerID;
                    _addAlternativePerformerName(dal,entityPtr->songPerformerName,selectedPerformerPtr->performerName());
                    performerID2CorrectNameMap[performerID]=selectedPerformerPtr->performerName();
                }
                else
                {
                    performerID=name2PerformerIDMap[entityPtr->songPerformerName];
                }
                entityPtr->songPerformerID=performerID;
                entityPtr->albumPerformerID=performerID;
            }
        }
        else
        {
            qDebug() << SB_DEBUG_WARNING << "entityPtr NOT DEFINED";
        }
        ProgressDialog::instance()->update("MusicLibrary::validateEntityList",progressCurrentValue++,progressMaxValue);
    }

    qDebug() << SB_DEBUG_INFO;
    QHashIterator<int,QString> it(performerID2CorrectNameMap);
    while(it.hasNext())
    {
        it.next();
        qDebug() << SB_DEBUG_INFO << it.key() << it.value();
    }

    //	Go through list and reset performer name
    qDebug() << SB_DEBUG_INFO << list.count();
    QVectorIterator<MLentityPtr> eIT(list);
    while(eIT.hasNext())
    {
        MLentityPtr e=eIT.next();
        if(e)
        {
            e->albumPerformerName=performerID2CorrectNameMap[e->albumPerformerID];
            e->songPerformerName=performerID2CorrectNameMap[e->songPerformerID];
        }
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
                         << e->albumID
                         << e->albumTitle
                         << e->albumPosition
                ;
            }
        }
    }

    //	2.	Validate albums

    //	a.	Handle abums with multiple artists
qDebug() << SB_DEBUG_INFO;
    if(properties->configValue(Properties::sb_performer_album_directory_structure_flag)=="1")
    {
qDebug() << SB_DEBUG_INFO;
        //	If music library is organized <performer>/<album>, we'll always assign parent directory name as the album title.
        //	For self made/collection albums, renumber album positions.

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

            if(entityPtr && entityPtr->albumID==-1 && !entityPtr->errorFlag() && !(entityPtr->removedFlag))
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
            ProgressDialog::instance()->update("MusicLibrary::validateEntityList",progressCurrentValue++,progressMaxValue);
        }

        progressCurrentValue=0;
        progressMaxValue=list.count();
        ProgressDialog::instance()->update("MusicLibrary::validateEntityList",progressCurrentValue,progressMaxValue);

        feIT.toFront();
        while(feIT.hasNext())
        {
            MLentityPtr entityPtr=feIT.next();

            if(entityPtr && entityPtr->albumID==-1 && !entityPtr->errorFlag() && !(entityPtr->removedFlag))
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
            MLentityPtr ePtr=eIT.next();
            if(ePtr && ePtr->errorFlag()==0)
            {
                {
                    qDebug() << SB_DEBUG_INFO
                             << ePtr->filePath
                             << ePtr->songPerformerID
                             << ePtr->songPerformerName
                             << ePtr->albumPerformerID
                             << ePtr->albumPerformerName
                             << ePtr->albumID
                             << ePtr->albumTitle
                             << ePtr->createArtificialAlbumFlag
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
        MLentityPtr ePtr=feIT.next();
        SBIDAlbumPtr selectedAlbumPtr;

        if(ePtr && ePtr->albumID==-1 && !ePtr->errorFlag() && (!ePtr->removedFlag))
        {
            //	Only validate albums if the albumID==-1.
            const QString key=QString("%1:%2").arg(ePtr->albumTitle).arg(ePtr->albumPerformerID);
            if(!albumTitle2albumIDMap.contains(key))
            {
                if(ePtr->createArtificialAlbumFlag || greatestHitsAlbums.contains(ePtr->albumTitle))
                {
                    //	Create greatest hits album
                    Common::sb_parameters p;
                    p.albumTitle=ePtr->albumTitle;
                    p.performerID=ePtr->albumPerformerID;
                    p.year=ePtr->year;
                    p.genre=ePtr->genre;
                    selectedAlbumPtr=amgr->createInDB(p);

                    qDebug() << SB_DEBUG_INFO << "CRT ALBUM:"
                             << selectedAlbumPtr->albumTitle()
                             << selectedAlbumPtr->albumPerformerID()
                    ;
                }
                else
                {
                    //	Let user select
                    Common::sb_parameters p;
                    p.albumID=ePtr->albumID;
                    p.albumTitle=ePtr->albumTitle;
                    p.performerID=ePtr->albumPerformerID;
                    p.performerName=ePtr->albumPerformerName;
                    p.year=ePtr->year;
                    p.genre=ePtr->genre;

                    qDebug() << SB_DEBUG_INFO
                             << ePtr->filePath
                             << p.albumID
                             << p.albumTitle
                             << p.performerID
                             << p.performerName
                             << p.year
                             << p.genre
                    ;
                    Common::result result=amgr->userMatch(p,SBIDAlbumPtr(),selectedAlbumPtr);
                    if(result==Common::result_canceled)
                    {
                        qDebug() << SB_DEBUG_INFO << "none selected -- exit from import";
                        return 0;
                    }
                    if(result==Common::result_missing)
                    {
                        qDebug() << SB_DEBUG_INFO
                             << ePtr->filePath
                             << p.albumTitle
                             << p.performerID
                             << p.performerName
                             << p.year
                             << p.genre
                    ;
                        //	albumPerformerID not set
                        selectedAlbumPtr=amgr->createInDB(p);
                    }
                }
                albumTitle2albumIDMap[key]=selectedAlbumPtr->albumID();
            }
            else
            {
                selectedAlbumPtr=amgr->retrieve(SBIDAlbum::createKey(albumTitle2albumIDMap[key]));
            }
            ePtr->albumID=selectedAlbumPtr->albumID();
            ePtr->albumPerformerID=selectedAlbumPtr->albumPerformerID();
        }
        ProgressDialog::instance()->update("MusicLibrary::validateEntityList",progressCurrentValue++,progressMaxValue);
    }
    qDebug() << SB_DEBUG_INFO;

    {	//	DEBUG
        QVectorIterator<MLentityPtr> eIT(list);
        qDebug() << SB_DEBUG_INFO << "SECTION D2c" << list.count();
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
                         << e->albumPosition
                ;
            }
        }
    }

    //	3.	Validate songs
    progressCurrentValue=0;
    progressMaxValue=list.count();
    ProgressDialog::instance()->update("MusicLibrary::validateEntityList",progressCurrentValue,progressMaxValue);
    feIT.toFront();

    QHash<QString,int> songTitle2songIDMap;	//	key: <song title>:<song performer id>
    songTitle2songIDMap.reserve(list.count());
    while(feIT.hasNext())
    {
        MLentityPtr entityPtr=feIT.next();

        if(entityPtr)
        {
            if(!entityPtr->errorFlag() && !(entityPtr->removedFlag))
            {
                SBIDSongPtr selectedSongPtr;

                const QString key=QString("%1:%2").arg(entityPtr->songTitle).arg(entityPtr->songPerformerID);
                if(!songTitle2songIDMap.contains(key))
                {
                    Common::sb_parameters p;
                    p.songTitle=entityPtr->songTitle;
                    p.performerID=entityPtr->songPerformerID;
                    p.performerName=entityPtr->songPerformerName;
                    p.year=entityPtr->year;
                    p.notes=entityPtr->notes;

                    qDebug() << SB_DEBUG_INFO << p.songID;

                    Common::result result=smgr->userMatch(p,SBIDSongPtr(),selectedSongPtr);
                    if(result==Common::result_canceled)
                    {
                        qDebug() << SB_DEBUG_INFO << "none selected -- exit from import";
                        return 0;
                    }

                    //	Create song
                    if(result==Common::result_missing)
                    {
                        selectedSongPtr=smgr->createInDB(p);
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
                    if(smgr->commit(selectedSongPtr,dal)==0)
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

    {	//	DEBUG
        QVectorIterator<MLentityPtr> eIT(list);
        qDebug() << SB_DEBUG_INFO << "SECTION D22" << list.count();
        while(eIT.hasNext())
        {
            MLentityPtr e=eIT.next();
            if(e && !e->errorFlag())
            {
                qDebug() << SB_DEBUG_INFO
                         << e->filePath
                         << e->songID
                         << e->songPerformanceID
                         << e->songTitle
                ;
            }
        }
    }
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

///	Private methods
QString
MusicLibrary::_retrieveCorrectPerformerName(DataAccessLayer* dal, const QString &altPerformerName)
{
    if(_alternativePerformerName2CorrectPerformerName.count()==0)
    {
        QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
        QString q="SELECT artist_alternative_name, artist_correct_name FROM ___SQL_SCHEMA_NAME___artist_match";
        dal->customize(q);
        QSqlQuery queryList(q,db);
        while(queryList.next())
        {
            _alternativePerformerName2CorrectPerformerName[queryList.value(0).toString()]=queryList.value(1).toString();
        }
    }
    QString newAltPerformerName=Common::correctArticle(altPerformerName);

    return _alternativePerformerName2CorrectPerformerName.contains(newAltPerformerName)?_alternativePerformerName2CorrectPerformerName[newAltPerformerName]:newAltPerformerName;
}

void
MusicLibrary::_addAlternativePerformerName(DataAccessLayer *dal, const QString& altPerformerName, const QString& correctPerformerName)
{
    if(!_alternativePerformerName2CorrectPerformerName.contains(altPerformerName))
    {
        QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
        QString q=
            "INSERT INTO ___SQL_SCHEMA_NAME___artist_match "
            "( "
                "artist_alternative_name, "
                "artist_correct_name "
            ") "
            "VALUES "
            "( "
                "'%1', "
                "'%2' "
            ") ";
        dal->customize(q);
        QSqlQuery insert(q,db);
        Q_UNUSED(insert);

        _alternativePerformerName2CorrectPerformerName[altPerformerName]=correctPerformerName;
    }
}
