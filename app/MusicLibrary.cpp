#include <QDir>
#include <QProgressDialog>

#include "MusicLibrary.h"

#include "AudioDecoderFactory.h"
#include "CacheManager.h"
#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "DataAccessLayer.h"
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
    PropertiesPtr properties=Context::instance()->properties();
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    const QString databaseRestorePoint=dal->createRestorePoint();
    bool suppressDialogsFlag=Context::instance()->properties()->configValue(Configuration::sb_smart_import).toInt();


    const QString schema=properties->currentDatabaseSchema();

    //	Important lists: all have `path' as key (in lowercase)
    QVector<MLentityPtr> foundEntities;
    QHash<QString,MLperformancePtr> pathToSong;	//	existing songs as known in database
    QHash<QString,MLalbumPathPtr> directory2AlbumPathMap;	//	album path map

    //	SBIDManager
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumMgr* aMgr=cm->albumMgr();
    CacheAlbumPerformanceMgr* apMgr=cm->albumPerformanceMgr();

    //	Init
    ProgressDialog::instance()->startDialog(__SB_PRETTY_FUNCTION__,"Starting",1);
    _numNewSongs=0;
    _numNewPerformers=0;
    _numNewAlbums=0;

    const int numOnlinePerformances=SBIDOnlinePerformance::totalNumberOnlinePerformances()+100;
    int progressCurrentValue=0;
    int progressMaxValue=numOnlinePerformances;
    ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step1:retrieveFiles",progressCurrentValue,progressMaxValue);

    const QString schemaRoot=
        Context::instance()->properties()->musicLibraryDirectory()
        +"/"
        +schema
        +(schema.length()?"/":"");

    const QString mtaf=QString("j/john lennon/imagine/02 - crippled inside.flac");
    const QString mtaf_song=QString("crippled");
    const QString mtaf_performer=QString("john lennon");
    const int mtaf_songID=24848;
    QString keyFromFS;
    QString keyFromDB;

    ///////////////////////////////////////////////////////////////////////////////////
    ///	Section A:	Retrieve paths found in directory
    ///////////////////////////////////////////////////////////////////////////////////
    int numFiles=0;
    QDirIterator it(schemaRoot,
                    QDir::AllDirs | QDir::AllEntries | QDir::Files | QDir::NoSymLinks | QDir::Readable,
                    QDirIterator::Subdirectories);
    QCoreApplication::processEvents();
    int ID=9999;
    QElapsedTimer time; time.start();
    while (it.hasNext())
    {
        it.next();
        QFileInfo fi=it.fileInfo();
        QString path=it.filePath().toUtf8();
        if(fi.isFile())
        {
            path=path.mid(schemaRoot.length());
            MLentity e;
            e.ID=ID++;
            e.filePath=path;
            e.absoluteParentDirectoryPath=fi.absoluteDir().absolutePath();
            e.parentDirectoryPath=e.absoluteParentDirectoryPath.mid(schemaRoot.length());
            e.parentDirectoryName=fi.absoluteDir().dirName();
            e.extension=fi.completeSuffix();
            e.key=Common::removeAccents(e.filePath.toLower());

            if(e.key==mtaf)
            {
                qDebug() << SB_DEBUG_INFO << "*********************************************************************";
                qDebug() << SB_DEBUG_INFO << mtaf_songID << e.key;
                keyFromFS=e.key;
            }
            if(AudioDecoderFactory::fileSupportedFlag(fi,0))
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
                    label=label.left(30);
                }
                label="Scanning album "+label;
                ProgressDialog::instance()->setLabelText(__SB_PRETTY_FUNCTION__,label);
                if(progressCurrentValue+1>progressMaxValue)
                {
                    progressMaxValue=progressCurrentValue+1;
                }
                ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step1:retrieveFiles",progressCurrentValue,progressMaxValue);
                time.restart();
                qDebug() << SB_DEBUG_INFO << progressCurrentValue;
            }
            progressCurrentValue++;
        }
    }
    ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,"step1:retrieveFiles");
    qDebug() << SB_DEBUG_INFO << keyFromFS;

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
    ProgressDialog::instance()->startDialog(__SB_PRETTY_FUNCTION__,"Retrieving existing songs",1);
    ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step2:retrieveExisting",progressCurrentValue,progressMaxValue);
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
        performance.key=Common::removeAccents(performance.path.toLower());

        pathToSong[performance.key]=std::make_shared<MLperformance>(performance);
        existingPath[performance.key]=0;

        if(performance.songID==mtaf_songID)
        {
            qDebug() << SB_DEBUG_INFO << "*********************************************************************";
            qDebug() << SB_DEBUG_INFO << performance.songID << performance.key << performance.path;
            qDebug() << SB_DEBUG_INFO << performance.key;

            if(performance.songID==mtaf_songID && performance.key.contains(mtaf_song) && performance.key.contains(mtaf_performer))
            {
                keyFromDB=performance.key;
                qDebug() << SB_DEBUG_INFO << keyFromDB;
            }

//            QString b=Common::removeAccents(p,1);
//            qDebug() << SB_DEBUG_INFO << b;

//            QString t=pathToSongKey;
//            for(int i=0;i<t.length();i++)
//            {
//                qDebug() << SB_DEBUG_INFO << i << t.at(i) << t.at(i).unicode();

//            }
        }

        if(time.elapsed()>700)
        {
            QString label=performance.path;
            if(label.length()>40)
            {
                label="..."+label.right(30);
            }
            label="Scanning song "+label;
            ProgressDialog::instance()->setLabelText(__SB_PRETTY_FUNCTION__,label);
            ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step2:retrieveExisting",progressCurrentValue,progressMaxValue);
            time.restart();
        }
        progressCurrentValue++;
    }
    delete sqm;
    ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,"step2:retrieveExisting");

    qDebug() << SB_DEBUG_INFO << "db:" << keyFromDB;
    qDebug() << SB_DEBUG_INFO << "fs:" << keyFromFS;

    if(keyFromDB==keyFromFS)
    {
        qDebug() << SB_DEBUG_INFO << "equal";
    }
    else
    {
        qDebug() << SB_DEBUG_INFO << "NOT equal";
    }

    ///////////////////////////////////////////////////////////////////////////////////
    ///	Section C:	Determine new songs and retrieve meta data for these
    ///////////////////////////////////////////////////////////////////////////////////
    QMutableVectorIterator<MLentityPtr> feIT(foundEntities);
    QVector<MLentityPtr> newEntities;
    int numNewSongs=0;

    progressCurrentValue=0;
    progressMaxValue=foundEntities.count();
    ProgressDialog::instance()->startDialog(__SB_PRETTY_FUNCTION__,"Getting meta data",1);
    ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step3:retrieveMetaData",progressCurrentValue,progressMaxValue);
    time.restart();

    qDebug() << SB_DEBUG_INFO << foundEntities.count();
    while(feIT.hasNext())
    {
        MLentityPtr ePtr=feIT.next();
        bool showDebugFlag=0;

        if(ePtr->key==mtaf)
        {
            showDebugFlag=1;
            qDebug() << SB_DEBUG_INFO << "*********************************************************************";
            qDebug() << SB_DEBUG_INFO << mtaf_songID << ePtr->key;
        }

        if(pathToSong.contains(ePtr->key))
        {
            pathToSong[ePtr->key]->pathExists=1;
            if(showDebugFlag)
            {
                qDebug() << SB_DEBUG_INFO << "FOUND!";
            }
        }
        else if(ePtr->errorFlag()==0)
        {
            if(showDebugFlag)
            {
                qDebug() << SB_DEBUG_INFO << "NOT FOUND!";
            }
            newEntities.append(ePtr);
            qDebug() << SB_DEBUG_INFO << "new" << ePtr->key;

            //	Populate meta data
            MetaData md(schemaRoot+ePtr->filePath);

            //	Primary meta data
            ePtr->albumPosition=md.albumPosition();
            ePtr->albumTitle=md.albumTitle();
            ePtr->songPerformerName=_retrieveCorrectPerformerName(dal,md.songPerformerName()).trimmed();
            ePtr->songTitle=md.songTitle();

            //	Secondary meta data
            ePtr->albumPerformerName=ePtr->songPerformerName.trimmed(); // for now, default to <>
            ePtr->duration=md.duration();
            ePtr->genre=md.genre();
            ePtr->notes=QString();	//	Don't import notes
            ePtr->year=md.year();

            if(ePtr->key==mtaf)
            {
                qDebug() << SB_DEBUG_INFO << "*********************************************************************";
                qDebug() << SB_DEBUG_INFO << "file found:" << ePtr->key;
            }
            //	Check on album title, take parent directory name if not exists
            if(ePtr->albumTitle.length()==0 || (properties->configValue(Configuration::sb_performer_album_directory_structure_flag)=="1"))
            {
                //	Take the name of the parent directory as the album title.
                ePtr->albumTitle=ePtr->parentDirectoryName;
            }

            //	Populate directory2AlbumPathMap
            MLalbumPathPtr albumPathPtr;
            if(!directory2AlbumPathMap.contains(ePtr->absoluteParentDirectoryPath))
            {
                MLalbumPath albumPath;

                albumPath.directoryName=ePtr->parentDirectoryName;
                albumPath.path=ePtr->parentDirectoryPath;
                albumPath.albumPerformerName=ePtr->albumPerformerName;
                albumPath.year=ePtr->year;

                albumPath.albumTitle=ePtr->albumTitle;

                albumPathPtr=std::make_shared<MLalbumPath>(albumPath);

                directory2AlbumPathMap[ePtr->absoluteParentDirectoryPath]=albumPathPtr;
            }
            else
            {
                albumPathPtr=directory2AlbumPathMap[ePtr->absoluteParentDirectoryPath];
            }

            if(!albumPathPtr->uniqueAlbumTitles.contains(ePtr->albumTitle.toLower()))
            {
                albumPathPtr->uniqueAlbumTitles.append(ePtr->albumTitle.toLower());
            }
            if(!albumPathPtr->uniqueSongPerformerNames.contains(ePtr->songPerformerName))
            {
                albumPathPtr->uniqueSongPerformerNames.append(ePtr->songPerformerName);
            }

            //	Check if all primary meta data attributes are populated
            if(ePtr->albumPosition<0)
            {
                ePtr->errorMsg="Missing album position in meta data";
            }
            else if(ePtr->albumTitle.length()==0)
            {
                ePtr->errorMsg="Missing album title in meta data";
            }
            else if(ePtr->songPerformerName.length()==0)
            {
                ePtr->errorMsg="Missing performer name in meta data";
            }
            else if(ePtr->songTitle.length()==0)
            {
                ePtr->errorMsg="Missing song title in meta data";
            }
            else
            {
                numNewSongs++;
            }
            if(ePtr->errorMsg.length())
            {
                qDebug() << SB_DEBUG_ERROR << ePtr->errorMsg;
            }
        }

        if(time.elapsed()>700)
        {
            ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step3:retrieveMetaData",progressCurrentValue,progressMaxValue);
            time.restart();
        }
        progressCurrentValue++;
    }
    foundEntities=newEntities;
    ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,"step3:retrieveMetaData");
    ProgressDialog::instance()->finishDialog(__SB_PRETTY_FUNCTION__,1);	//	Not done yet

    if(1)
    {	//	DEBUG
        QVectorIterator<MLentityPtr> eIT(foundEntities);
        qDebug() << SB_DEBUG_INFO << "START LIST OF ALL NEW SONGS FOUND:";
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
                         << e->key
                ;
                ok++;
            }
        }
        qDebug() << SB_DEBUG_INFO << "END LIST OF ALL NEW SONGS FOUND:";

        int incorrect=0;
        eIT.toFront();
        qDebug() << SB_DEBUG_INFO << "START LIST OF SONGS THAT WILL NOT BE IMPORTED:";
        while(eIT.hasNext())
        {
            MLentityPtr e=eIT.next();
            if(e->errorFlag()!=0)
            {
                qDebug() << SB_DEBUG_INFO
                         << e->filePath
                         << e->songTitle
                         << e->songPerformerName
                         << e->albumTitle
                         << e->albumPosition
                ;
                incorrect++;
            }
        }
        qDebug() << SB_DEBUG_INFO << "END LIST OF SONGS THAT WILL NOT BE IMPORTED:";
        qDebug() << SB_DEBUG_INFO << ok << "VALID RECORDS";
        qDebug() << SB_DEBUG_INFO << incorrect << "INVALID RECORDS";
    }

    ///////////////////////////////////////////////////////////////////////////////////
    ///	Section D:	Validation and selection of the big three:
    /// 	-	Performers,
    /// 	-	Albums,
    /// 	-	Songs.
    ///////////////////////////////////////////////////////////////////////////////////
    bool cancelFlag=0;
    if(validateEntityList(foundEntities,directory2AlbumPathMap,MusicLibrary::validation_type_album,suppressDialogsFlag)==0)
    {
        cancelFlag=1;
        ProgressDialog::instance()->startDialog(__SB_PRETTY_FUNCTION__,"Canceling",1);
        ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,"stepX:cancel");
        Common::sleep(1);
        dal->restore(databaseRestorePoint);
        //	Dialogbox is closed at and of method
    }

    if(!cancelFlag)
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
            MLentityPtr ePtr=feIT.next();

            if(!ePtr->errorFlag())
            {
                if(ePtr->songID==-1)
                {
                    ePtr->errorMsg="songID not populated";
                }
                if(ePtr->songPerformerID==-1)
                {
                    ePtr->errorMsg="songPerformerID not populated";
                }
                if(ePtr->albumID==-1)
                {
                    ePtr->errorMsg="albumID not populated";
                }
                if(ePtr->albumPosition==-1)
                {
                    ePtr->errorMsg="albumPosition not populated";
                }
            }
        }

        //	2.	Save to database
        qDebug() << SB_DEBUG_INFO << "SANITYCHECK";
        apMgr->debugShow("BEFORE ADDALBUMPERFORMANCE");

        progressCurrentValue=0;
        progressMaxValue=foundEntities.count();
        ProgressDialog::instance()->startDialog(__SB_PRETTY_FUNCTION__,"Saving album data",1);
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step4:save",progressCurrentValue,progressMaxValue);
        feIT.toFront();
        while(feIT.hasNext())
        {
            MLentityPtr ePtr=feIT.next();

            if(!ePtr->errorFlag())
            {
                const QString title=QString("Compiling album '%1'").arg(ePtr->albumTitle);
                ProgressDialog::instance()->setLabelText(__SB_PRETTY_FUNCTION__,title);
                ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step4:save",progressCurrentValue++,progressMaxValue);

                qDebug() << SB_DEBUG_INFO << ePtr->filePath << ePtr->errorFlag();
                SBIDAlbumPtr albumPtr=aMgr->retrieve(SBIDAlbum::createKey(ePtr->albumID));
                SBIDAlbumPerformancePtr newAlbumPerformancePtr=albumPtr->addAlbumPerformance(
                            ePtr->songID,
                            ePtr->songPerformerID,
                            ePtr->albumPosition,
                            ePtr->year,
                            ePtr->filePath,
                            ePtr->duration,
                            ePtr->notes);

                //	CHG 20220928
                //if(newAlbumPerformancePtr->SBCreateStatus()==SBIDAlbumPerformance::sb_create_status_already_exists)
                //{
                    //ePtr->isImported=1;
                    //ePtr->errorMsg="Song already exists";	//	NEED TO FIGURE OUT WHY THIS IS A PROBLEM.
                //}
                //else
                //{
                    //ePtr->isImported=1;
                //}
                    ePtr->isImported=1;
            }
        }
        qDebug() << SB_DEBUG_INFO;
        ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,"step4:save");
        qDebug() << SB_DEBUG_INFO;
        ProgressDialog::instance()->finishDialog(__SB_PRETTY_FUNCTION__,1);
        qDebug() << SB_DEBUG_INFO;

        bool resultFlag=cm->saveChanges("Saving Changes");

        qDebug() << SB_DEBUG_INFO;
        if(resultFlag==0)
        {
            dal->restore(databaseRestorePoint);
        }
        qDebug() << SB_DEBUG_INFO;

    }

    //	Collect all errors
    QMap<QString,QString> errors;
    feIT.toFront();
    qDebug() << SB_DEBUG_INFO << "collecting errors";
    while(feIT.hasNext())
    {
        MLentityPtr entityPtr=feIT.next();

        if(entityPtr->errorFlag())
        {
            errors[entityPtr->filePath]=entityPtr->errorMsg;
            qDebug() << SB_DEBUG_INFO << errors.count() << entityPtr->filePath << entityPtr->errorMsg;
        }
        else if(entityPtr->isImported==0)
        {
            errors[entityPtr->filePath]="This song has NOT been imported. Like reason is this song being a duplicate";
        }
    }
    qDebug() << SB_DEBUG_INFO << errors;

    if(errors.count())
    {
        MusicImportResult mir(errors);
        mir.exec();
    }

    //	Refresh caches
    Context::instance()->controller()->refreshModels();
    Context::instance()->controller()->preloadAllSongs();

    ProgressDialog::instance()->hide();
    ProgressDialog::instance()->stats();

    QString resultTxt=QString("<P>%4 %1 songs<BR>%4 %2 performers<BR>%4 %3 albums").arg(_numNewSongs).arg(_numNewPerformers).arg(_numNewAlbums).arg(QChar(8226));
    SBMessageBox::createSBMessageBox(QString("<center>Added:</center>"),resultTxt,QMessageBox::Information,QMessageBox::Ok,QMessageBox::Ok,QMessageBox::Ok,1);

    qDebug() << SB_DEBUG_INFO << "Finished";
    return;
}

bool
MusicLibrary::validateEntityList(QVector<MLentityPtr>& list, QHash<QString,MLalbumPathPtr>& directory2AlbumPathMap, const MusicLibrary::MLvalidationType validationType, bool suppressDialogsFlag)
{
    if(validationType==MusicLibrary::validation_type_none)
    {
        SBMessageBox::createSBMessageBox("Internal error in validateEntityList",
                                         "Type of validation not specified",
                                         QMessageBox::Critical,
                                         QMessageBox::Close,
                                         QMessageBox::Close,
                                         QMessageBox::Close);

    }
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumMgr* amgr=cm->albumMgr();
    CachePerformerMgr* pemgr=cm->performerMgr();
    CacheSongMgr* smgr=cm->songMgr();
    SBIDPerformerPtr variousPerformerPtr=SBIDPerformer::retrieveVariousPerformers();
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    int progressCurrentValue=0;
    int progressMaxValue=0;
    QHashIterator<QString,MLalbumPathPtr> albumIT(directory2AlbumPathMap);

    qDebug() << SB_DEBUG_INFO;
    if(1)
    {	//	DEBUG
        qDebug() << SB_DEBUG_INFO << "START OF VALIDATION" << list.count();
        QVectorIterator<MusicLibrary::MLentityPtr> eIT(list);
        while(eIT.hasNext())
        {
            MusicLibrary::MLentityPtr ePtr=eIT.next();
            if(ePtr && ePtr->errorFlag()==0)
            {
                qDebug() << SB_DEBUG_INFO
                     << ePtr->albumID
                     << ePtr->albumPosition
                     << ePtr->songTitle
                     << ePtr->songPerformerName
                     << ePtr->mergedToAlbumPosition
                     << ePtr->removedFlag
                ;
            }
        }
        qDebug() << SB_DEBUG_INFO << "END";
        QHashIterator<QString,MLalbumPathPtr> albumIT(directory2AlbumPathMap);
        while(albumIT.hasNext())
        {
            albumIT.next();
            MLalbumPathPtr apPtr=albumIT.value();
            qDebug() << SB_DEBUG_INFO
                     << apPtr->albumID
                     << apPtr->albumTitle
                     << apPtr->albumPerformerID
                     << apPtr->albumPerformerName
                     << apPtr->path
                     << apPtr->variousPerformerFlag
                     << apPtr->uniqueAlbumTitles.count()
                     << apPtr->uniqueSongPerformerNames.count()
                     << apPtr->multipleEntriesFlag()
            ;
        }
    }

    //	1.	Validate performers
    QMutableVectorIterator<MLentityPtr> listIT(list);

    //		a.	Collect performer names from songPerformer and albumPerformer in list.
    QStringList allPerformers;
    while(listIT.hasNext())
    {
        const MLentityPtr ePtr=listIT.next();

        if(ePtr && !ePtr->errorFlag() && !ePtr->removedFlag)
        {
            if(ePtr->albumPerformerName.length() && !allPerformers.contains(ePtr->albumPerformerName))
            {
                allPerformers.append(ePtr->albumPerformerName);
            }
            if(ePtr->songPerformerName.length() && !allPerformers.contains(ePtr->songPerformerName))
            {
                allPerformers.append(ePtr->songPerformerName);
            }
        }
    }

    albumIT=QHashIterator<QString,MLalbumPathPtr>(directory2AlbumPathMap);
    while(albumIT.hasNext())
    {
        albumIT.next();
        MLalbumPathPtr apPtr=albumIT.value();
        qDebug() << SB_DEBUG_INFO << apPtr->albumTitle << apPtr->path << apPtr->albumPerformerName << apPtr->albumPerformerID;
        if(apPtr->albumPerformerName.length() && !allPerformers.contains(apPtr->albumPerformerName))
        {
            allPerformers.append(apPtr->albumPerformerName);
        }
    }

    QHash<QString,int> name2PerformerIDMap;
    QHash<int,QString> performerID2CorrectNameMap;

    qDebug() << SB_DEBUG_INFO;
    QStringList ignoreClasses;
    ignoreClasses << "CacheTemplate" << "Preloader";
    progressCurrentValue=0;
    progressMaxValue=allPerformers.count();
    ProgressDialog::instance()->startDialog(__SB_PRETTY_FUNCTION__,"Validating Performers",1);
    ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step:validatePerformers",progressCurrentValue,progressMaxValue);

    //		b.	Go through all collected performer names and validate these.
    listIT=QMutableVectorIterator<MLentityPtr>(list);
    QStringListIterator allPerformersIT(allPerformers);
    while(allPerformersIT.hasNext())
    {
        QString performerName=allPerformersIT.next();

        int performerID=-1;
        if(!name2PerformerIDMap.contains(performerName))
        {
            SBIDPerformerPtr selectedPerformerPtr;
            Common::sb_parameters p;
            p.performerName=performerName;
            p.performerID=-1;
            p.suppressDialogsFlag=suppressDialogsFlag;

            Common::result result=Common::result_missing;
            result=pemgr->userMatch(p,SBIDPerformerPtr(),selectedPerformerPtr);

            if(result==Common::result_canceled)
            {
                qDebug() << SB_DEBUG_INFO << "none selected -- exit from import";
                return 0;
            }
            else if(result==Common::result_missing)
            {
                selectedPerformerPtr=pemgr->createInDB(p);
                _numNewPerformers++;
            }
            else if(result==Common::result_exists_user_selected)
            {
                if(p.performerName != selectedPerformerPtr->performerName())
                {
                    selectedPerformerPtr->addAlternativePerformerName(p.performerName);
                }
            }
            performerID=selectedPerformerPtr->performerID();
            name2PerformerIDMap[performerName]=performerID;
            _addAlternativePerformerName(dal,performerName,selectedPerformerPtr->performerName());
            performerID2CorrectNameMap[performerID]=selectedPerformerPtr->performerName();
            qDebug() << SB_DEBUG_INFO << performerID << selectedPerformerPtr->performerName();
        }
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step:validatePerformers",progressCurrentValue++,progressMaxValue);
    }
    ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,"step:validatePerformers");

    {
        qDebug() << SB_DEBUG_INFO << "START: entityList";
        while(listIT.hasNext())
        {
            MLentityPtr ePtr=listIT.next();

            qDebug() << SB_DEBUG_INFO << ePtr->key << ":path=" << ePtr->filePath << ":albumPerformerID=" << ePtr->albumPerformerID;
        }
        qDebug() << SB_DEBUG_INFO << "START: directory2AlbumPathMap";
        albumIT.toFront();
        while(albumIT.hasNext())
        {
            albumIT.next();
            MLalbumPathPtr apPtr=albumIT.value();
            qDebug() << SB_DEBUG_INFO
                     << "albumID=" << apPtr->albumID
                     << ":title=" << apPtr->albumTitle
                     << ":performerID=" << apPtr->albumPerformerID
                     << ":performerName=" << apPtr->albumPerformerName
                     << ":path=" << apPtr->path
                     << ":variousPerformerFlag=" << apPtr->variousPerformerFlag
                     << apPtr->uniqueAlbumTitles.count()
                     << apPtr->uniqueSongPerformerNames.count()
                     << apPtr->multipleEntriesFlag()
            ;
        }
        qDebug() << SB_DEBUG_INFO << "END: ALL";
    }

    //		c.	Go through list and set performerID's accordingly.
    listIT=QMutableVectorIterator<MLentityPtr>(list);
    {
        QString albumPerformerName;
        QString songPerformerName;
        while(listIT.hasNext())
        {
            MLentityPtr ePtr=listIT.next();
            albumPerformerName=ePtr->albumPerformerName;
            songPerformerName=ePtr->songPerformerName;

            //	Set the correct performer ID's.

            qDebug() << SB_DEBUG_INFO << ePtr->key << ":path=" << ePtr->filePath << ":albumPerformerName=" << ePtr->albumPerformerName << ":lookup=" << name2PerformerIDMap[ePtr->albumPerformerName];

            if(validationType==MusicLibrary::validation_type_album)
            {
                if(albumPerformerName!="")
                {
                    ePtr->albumPerformerID=name2PerformerIDMap[albumPerformerName];
                    ePtr->albumPerformerName=performerID2CorrectNameMap[ePtr->albumPerformerID];
                }
                else
                {
                    ePtr->isImported=0;
                    qDebug() << SB_DEBUG_ERROR << ePtr->key << ":marking song as invalid: missing album performer name in meta data";
                    ePtr->errorMsg="Album performer name not populated for this song (Missing album performer name in meta data)";
                }
            }
            if(songPerformerName!="")
            {
                ePtr->songPerformerID=name2PerformerIDMap[ePtr->songPerformerName];
                ePtr->songPerformerName=performerID2CorrectNameMap[ePtr->songPerformerID];
            }
            else
            {
                ePtr->isImported=0;
                qDebug() << SB_DEBUG_ERROR << ePtr->key << ":marking song as invalid: missing song performer name in meta data";
                ePtr->errorMsg="Song Performer name not populated for this song (Missing song performer name in meta data)";
            }
        }
    }

    if(1)
    {
        qDebug() << SB_DEBUG_INFO;
        QHashIterator<QString,int> n2pIT(name2PerformerIDMap);
        qDebug() << SB_DEBUG_INFO << "START: name2PerformerIDMap";
        while(n2pIT.hasNext())
        {
            n2pIT.next();
            qDebug() << SB_DEBUG_INFO << n2pIT.key() << n2pIT.value();
        }
        qDebug() << SB_DEBUG_INFO << "START: entityList";
        listIT.toFront();
        while(listIT.hasNext())
        {
            MLentityPtr ePtr=listIT.next();

            qDebug() << SB_DEBUG_INFO << ePtr->key << ":path=" << ePtr->filePath << ":albumPerformerID=" << ePtr->albumPerformerID;
        }
        qDebug() << SB_DEBUG_INFO << "START: directory2AlbumPathMap";
        albumIT.toFront();
        while(albumIT.hasNext())
        {
            albumIT.next();
            MLalbumPathPtr apPtr=albumIT.value();
            qDebug() << SB_DEBUG_INFO
                     << "albumID=" << apPtr->albumID
                     << ":title=" << apPtr->albumTitle
                     << ":performerID=" << apPtr->albumPerformerID
                     << ":performerName=" << apPtr->albumPerformerName
                     << ":path=" << apPtr->path
                     << ":variousPerformerFlag=" << apPtr->variousPerformerFlag
                     << apPtr->uniqueAlbumTitles.count()
                     << apPtr->uniqueSongPerformerNames.count()
                     << apPtr->multipleEntriesFlag()
            ;
        }
        qDebug() << SB_DEBUG_INFO << "END: ALL";

    }
    {
        QString albumPerformerName;
        albumIT=QHashIterator<QString,MLalbumPathPtr>(directory2AlbumPathMap);
        while(albumIT.hasNext())
        {
            albumIT.next();
            MLalbumPathPtr apPtr=albumIT.value();
            albumPerformerName=apPtr->albumPerformerName;

            qDebug() << SB_DEBUG_INFO << albumIT.key() << ":albumTitle=" << apPtr->albumTitle << ":albumPerformerID=" << apPtr->albumPerformerID << ":albumPerformerName=" << apPtr->albumPerformerName;
            qDebug() << SB_DEBUG_INFO << albumIT.key() << apPtr->variousPerformerFlag << apPtr->uniqueAlbumTitles.count() << apPtr->uniqueSongPerformerNames.count() << apPtr->multipleEntriesFlag();
            if(apPtr->uniqueAlbumTitles.count()>1)
            {
                qDebug() << SB_DEBUG_INFO << apPtr->uniqueAlbumTitles;
            }
            if(apPtr->multipleEntriesFlag()==1)
            {
                apPtr->albumPerformerID=variousPerformerPtr->performerID();
                apPtr->albumPerformerName=variousPerformerPtr->performerName();
            }
            else if(albumPerformerName!="")
            {
                apPtr->albumPerformerID=name2PerformerIDMap[albumPerformerName];
                apPtr->albumPerformerName=performerID2CorrectNameMap[apPtr->albumPerformerID];
            }
            else
            {
                apPtr->errorMsg="Empty performer name"; //  not being displayed.
            }
            qDebug() << SB_DEBUG_INFO << albumIT.key() << ":albumPerformerID=" << apPtr->albumPerformerID << ":albumPerformerName=" << apPtr->albumPerformerName;
        }
    }


    //  Mark records without albumPerformerID as invalid



    {	//	DEBUG
        QVectorIterator<MLentityPtr> eIT(list);
        qDebug() << SB_DEBUG_INFO << "AFTER PERFORMER VALIDATION" << list.count();
        qDebug() << SB_DEBUG_INFO << "START: entityList";
        while(eIT.hasNext())
        {
            MLentityPtr ePtr=eIT.next();
            if(ePtr && ePtr->errorFlag()==0)
            {
                qDebug() << SB_DEBUG_INFO
                     << ePtr->albumID
                     << ePtr->albumPosition
                     << ePtr->songTitle
                     << ePtr->songPerformerID
                     << ePtr->songPerformerName
                     << ePtr->mergedToAlbumPosition
                     << ePtr->albumTitle
                     << ePtr->albumID
                     << ePtr->albumPerformerName
                     << ePtr->albumPerformerID
                     << ePtr->chartPosition
                     << ePtr->removedFlag
                ;
            }
        }
        QHashIterator<QString,MLalbumPathPtr> albumIT(directory2AlbumPathMap);
        qDebug() << SB_DEBUG_INFO << "START: directory2AlbumPathMap";
        while(albumIT.hasNext())
        {
            albumIT.next();
            MLalbumPathPtr apPtr=albumIT.value();
            qDebug() << SB_DEBUG_INFO
                     << "albumID=" << apPtr->albumID
                     << ":title=" << apPtr->albumTitle
                     << ":performerID=" << apPtr->albumPerformerID
                     << ":performerName=" << apPtr->albumPerformerName
                     << ":path=" << apPtr->path
                     << ":variousPerformerFlag=" << apPtr->variousPerformerFlag
                     << apPtr->uniqueAlbumTitles.count()
                     << apPtr->uniqueSongPerformerNames.count()
                     << apPtr->multipleEntriesFlag()
            ;
        }
        qDebug() << SB_DEBUG_INFO << "END: ALL";
    }

    //	2.	Validate albums

    {	//	DEBUG
        QVectorIterator<MLentityPtr> eIT(list);
        qDebug() << SB_DEBUG_INFO << "BEFORE ALBUM VALIDATION" << list.count();
        while(eIT.hasNext())
        {
            MLentityPtr ePtr=eIT.next();
            if(ePtr && ePtr->errorFlag()==0)
            {
                qDebug() << SB_DEBUG_INFO
                     << ePtr->albumID
                     << ePtr->albumTitle
                     << ePtr->albumPerformerName
                     << ePtr->albumPerformerID
                     << ePtr->albumPosition
                     << ePtr->songTitle
                     << ePtr->songPerformerName
                     << ePtr->mergedToAlbumPosition
                     << ePtr->albumTitle
                     << ePtr->albumID
                     << ePtr->albumPerformerName
                     << ePtr->albumPerformerID
                     << ePtr->removedFlag
                     << ePtr->filePath
                ;
            }
        }

        QHashIterator<QString,MLalbumPathPtr> albumIT(directory2AlbumPathMap);
        while(albumIT.hasNext())
        {
            albumIT.next();
            MLalbumPathPtr apPtr=albumIT.value();
            qDebug() << SB_DEBUG_INFO
                     << apPtr->albumID
                     << apPtr->albumTitle
                     << apPtr->albumPerformerID
                     << apPtr->albumPerformerName
                     << apPtr->path
                     << apPtr->variousPerformerFlag
                     << apPtr->uniqueAlbumTitles.count()
                     << apPtr->uniqueSongPerformerNames.count()
                     << apPtr->multipleEntriesFlag()
            ;
        }
    }

    //		a.	Go thru each albumPtr and find out if album can be found based on path name
    //			This can only be done if directories are structured.
    PropertiesPtr properties=Context::instance()->properties();
    if(properties->configValue(Configuration::sb_performer_album_directory_structure_flag)=="1" && directory2AlbumPathMap.count()>0)
    {
        albumIT=QHashIterator<QString,MLalbumPathPtr>(directory2AlbumPathMap);
        while(albumIT.hasNext())
        {
            albumIT.next();
            qDebug() << SB_DEBUG_INFO << albumIT.key();
            MLalbumPathPtr apPtr=albumIT.value();
            qDebug() << SB_DEBUG_INFO << apPtr->albumID << apPtr->albumTitle << apPtr->path << apPtr->albumPerformerName << apPtr->albumPerformerID;
            SBIDAlbumPtr aPtr=SBIDAlbum::retrieveAlbumByPath(apPtr->path);
            if(aPtr && apPtr->errorFlag()==0)
            {
                qDebug() << SB_DEBUG_INFO;
                apPtr->albumID=aPtr->albumID();
                apPtr->albumPerformerID=aPtr->albumPerformerID();
                apPtr->albumPerformerName=aPtr->albumPerformerName();
                apPtr->maxPosition=aPtr->numPerformances();

                if(aPtr->albumPerformerID()==variousPerformerPtr->performerID())
                {
                    apPtr->variousPerformerFlag=1;
                }
            }
        }
    }

    if(1)
    {
        QHashIterator<QString,MLalbumPathPtr> albumIT(directory2AlbumPathMap);
        while(albumIT.hasNext())
        {
            albumIT.next();
            MLalbumPathPtr apPtr=albumIT.value();
            qDebug() << SB_DEBUG_INFO
                     << apPtr->albumID
                     << apPtr->albumTitle
                     << apPtr->albumPerformerID
                     << apPtr->albumPerformerName
                     << apPtr->path
                     << apPtr->multipleEntriesFlag()
            ;
        }
    }

    //		b.	Actual user validation of albums
    albumIT=QHashIterator<QString,MLalbumPathPtr>(directory2AlbumPathMap);
    while(albumIT.hasNext())
    {
        albumIT.next();
        MLalbumPathPtr apPtr=albumIT.value();
        SBIDAlbumPtr selectedAlbumPtr;

        if(apPtr)
        {
            qDebug() << SB_DEBUG_INFO
                     << apPtr->albumID
                     << apPtr->albumTitle
                     << apPtr->albumPerformerID
                     << apPtr->albumPerformerName
                     <<	apPtr->maxPosition
                     << apPtr->errorFlag()
            ;
            qDebug() << SB_DEBUG_INFO
                     << apPtr->variousPerformerFlag
                     << apPtr->uniqueAlbumTitles.count()
                     << apPtr->uniqueSongPerformerNames.count()
                     << apPtr->maxPosition
                ;

            if(apPtr->errorFlag()==0)
            {
                //	Only create collection album if album is not found.
                if(apPtr->multipleEntriesFlag() && apPtr->albumID==-1)
                {
                    qDebug() << SB_DEBUG_INFO
                             << apPtr->multipleEntriesFlag()
                             << apPtr->albumID
                    ;

                    //	No validation needed -- create collection album
                    Common::sb_parameters p;
                    p.albumTitle=apPtr->albumTitle;
                    p.performerID=apPtr->albumPerformerID;
                    p.year=apPtr->year;
                    p.genre=apPtr->genre;
                    selectedAlbumPtr=amgr->createInDB(p);
                    _numNewAlbums++;

                    qDebug() << SB_DEBUG_INFO << "Create collection album"
                             << selectedAlbumPtr->albumTitle()
                             << selectedAlbumPtr->albumPerformerID()
                    ;
                }
                else
                {
                    //	Let user select
                    Common::sb_parameters p;
                    p.albumID=apPtr->albumID;
                    p.albumTitle=apPtr->albumTitle;
                    p.performerID=apPtr->albumPerformerID;
                    p.performerName=apPtr->albumPerformerName;
                    p.year=apPtr->year;
                    p.genre=apPtr->genre;
                    p.suppressDialogsFlag=suppressDialogsFlag;

                    //	Set up excluding album:
                    //	-	if ePtr.albumID=-1 then empty,
                    //	-	otherwise lookup existing album
                    //  const SBIDAlbumPtr originalAlbumPtr=(apPtr->albumID==-1?SBIDAlbumPtr():SBIDAlbum::retrieveAlbum(apPtr->albumID));
                    SBIDAlbumPtr originalAlbumPtr;

                    if(apPtr->albumID==-1)
                    {
                        originalAlbumPtr=SBIDAlbumPtr();
                    }
                    else
                    {
                        originalAlbumPtr=SBIDAlbum::retrieveAlbum(apPtr->albumID);
                    }

                    qDebug() << SB_DEBUG_INFO
                             << p.albumID
                             << p.albumTitle
                             << p.performerID
                             << p.performerName
                             << p.year
                             << p.genre
                    ;
                    if(originalAlbumPtr)
                    {
                        qDebug() << SB_DEBUG_INFO
                                 << originalAlbumPtr->albumID()
                                 << originalAlbumPtr->albumTitle()
                                 << originalAlbumPtr->albumPerformerID()
                                 << originalAlbumPtr->albumPerformerName()
                        ;
                    }
                    else
                    {
                        qDebug() << SB_DEBUG_INFO << "EMPTY!";
                    }
                    qDebug() << SB_DEBUG_INFO;

                    Common::result result=Common::result_missing;
                    if(!originalAlbumPtr)
                    {
                        result=amgr->userMatch(p,originalAlbumPtr,selectedAlbumPtr);
                    }
                    if(result==Common::result_canceled)
                    {
                        qDebug() << SB_DEBUG_INFO << "none selected -- exit from import";
                        return 0;
                    }
                    if(result==Common::result_missing)
                    {
                        //	If we work based on an existing album this means that the title has
                        //	changed. Therefore, the selectedAlbumPtr
                        if(originalAlbumPtr)
                        {
                            selectedAlbumPtr=originalAlbumPtr;
                        }
                        else
                        {
                            //	Only create if truly new album
                            qDebug() << SB_DEBUG_INFO
                                 << p.albumTitle
                                 << p.performerID
                                 << p.performerName
                                 << p.year
                                 << p.genre
                            ;

                            //	albumPerformerID not set
                            selectedAlbumPtr=amgr->createInDB(p);
                            _numNewAlbums++;
                            apPtr->albumID=selectedAlbumPtr->albumID();
                            apPtr->albumPerformerID=selectedAlbumPtr->albumPerformerID();
                        }
                    }
                }

                //	Assign ID's back to apPtr
                if(apPtr->albumID==-1 || selectedAlbumPtr->albumID()!=apPtr->albumID)
                {
                    //	Only assign maxPosition if new album or merged album.
                    apPtr->maxPosition=selectedAlbumPtr->numPerformances();
                }
                else
                {
                    apPtr->maxPosition=0;
                }
                apPtr->albumID=selectedAlbumPtr->albumID();

                //	CWIP: need to be tested in case of merging from performer specific album to various performer album or the other way around.
                if(apPtr->albumPerformerName==selectedAlbumPtr->albumPerformerName())
                {
                    apPtr->albumPerformerID=selectedAlbumPtr->albumPerformerID();
                }

                //	Set multiple entries flag if collection album
                if(apPtr->albumPerformerID==variousPerformerPtr->performerID())
                {
                    qDebug() << SB_DEBUG_INFO << "ASSIGNMENT VARIOUS PERFORMER";
                    apPtr->variousPerformerFlag=1;
                }
            }
            else
            {
                qDebug() << SB_DEBUG_ERROR << "Skipped:has error:" << apPtr->errorMsg;

            }
        }
        else
        {
            qDebug() << SB_DEBUG_ERROR << "apPtr not defined. Aborting.";
            return 0;
        }
    }

    //		c.	Assign album data to entity pointers
    progressCurrentValue=0;
    progressMaxValue=list.count();
    ProgressDialog::instance()->startDialog(__SB_PRETTY_FUNCTION__,"Validating Albums",1);
    ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step:validateAlbums",progressCurrentValue,progressMaxValue);
    qDebug() << SB_DEBUG_INFO << progressMaxValue;

    listIT=QMutableVectorIterator<MLentityPtr>(list);
    while(listIT.hasNext() && directory2AlbumPathMap.count()>0)
    {
        MLentityPtr ePtr=listIT.next();
        SBIDAlbumPtr selectedAlbumPtr;

        if(ePtr)
        {
            if(!ePtr->errorFlag() && (!ePtr->removedFlag))
            {
                qDebug() << SB_DEBUG_INFO
                     << ePtr->albumID
                     << ePtr->albumPosition
                     << ePtr->mergedToAlbumPosition
                     << ePtr->albumTitle
                     << ePtr->albumPerformerID
                     << ePtr->removedFlag
                     << ePtr->errorFlag();

                MLalbumPathPtr apPtr=directory2AlbumPathMap[ePtr->absoluteParentDirectoryPath];
                if(apPtr)
                {
                    selectedAlbumPtr=amgr->retrieve(SBIDAlbum::createKey(apPtr->albumID));
                    if(selectedAlbumPtr)
                    {
                        ePtr->albumID=selectedAlbumPtr->albumID();
                        ePtr->albumPerformerID=selectedAlbumPtr->albumPerformerID();

                        //	CHG 20220928: do not overwrite albumPosition
                        //if(apPtr->multipleEntriesFlag())
                        //{
                            //	Overwrite and assign new album position if collection album.
                            //ePtr->albumPosition=++(apPtr->maxPosition);
                        //}
                    }
                    else
                    {
                        qDebug() << SB_DEBUG_ERROR << "selectedAlbumPtr not defined. Aborting.";
                        return 0;
                    }
                }
                else
                {
                    qDebug() << SB_DEBUG_ERROR << "apPtr not defined. Aborting.";
                    return 0;
                }
            }
            else
            {
                qDebug() << SB_DEBUG_ERROR << ePtr->errorFlag() << ePtr->removedFlag;
            }
        }
        else
        {
            qDebug() << SB_DEBUG_ERROR << "ePtr NULL pointer. Aborting.";
            return 0;
        }
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step:validateAlbums",progressCurrentValue++,progressMaxValue);
    }
    ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,"step:validateAlbums");
    qDebug() << SB_DEBUG_INFO;

    {	//	DEBUG
        QVectorIterator<MLentityPtr> eIT(list);
        qDebug() << SB_DEBUG_INFO << "START: entityList" << list.count();
        while(eIT.hasNext())
        {
            MLentityPtr ePtr=eIT.next();
            if(ePtr && !ePtr->errorFlag())
            {
                qDebug() << SB_DEBUG_INFO
                     << "albumID=" << ePtr->albumID
                     << ":albumPerformerID=" << ePtr->albumPerformerID
                     << ":songID=" << ePtr->songID
                     << ":performerID=" << ePtr->songPerformerID
                     << ":albumPosition=" << ePtr->albumPosition
                     << ":songTitle=" << ePtr->songTitle
                     << ":songPerformerName=" << ePtr->songPerformerName
                     << ":mrg2albumPos=" << ePtr->mergedToAlbumPosition
                     << ":removedFlag=" << ePtr->removedFlag
                ;
            }
        }

        qDebug() << SB_DEBUG_INFO << "START: directory2AlbumPathMap";
        QHashIterator<QString,MLalbumPathPtr> albumIT(directory2AlbumPathMap);
        while(albumIT.hasNext())
        {
            albumIT.next();
            MLalbumPathPtr apPtr=albumIT.value();
            qDebug() << SB_DEBUG_INFO
                     << apPtr->albumID
                     << apPtr->albumTitle
                     << apPtr->albumPerformerID
                     << apPtr->albumPerformerName
                     << apPtr->multipleEntriesFlag()
            ;
        }
        qDebug() << SB_DEBUG_INFO << "ALL: END";
    }

    //	3.	Validate songs
    progressCurrentValue=0;
    progressMaxValue=list.count();
    ProgressDialog::instance()->startDialog(__SB_PRETTY_FUNCTION__,"Validating Songs",1);
    ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step:validateSongs",progressCurrentValue,progressMaxValue);
    listIT=QMutableVectorIterator<MLentityPtr>(list);

    QHash<QString,int> songTitle2songIDMap;	//	key: <song title>:<song performer id>
    songTitle2songIDMap.reserve(list.count());
    while(listIT.hasNext())
    {
        MLentityPtr entityPtr=listIT.next();

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
                    p.songID=entityPtr->songID;
                    p.suppressDialogsFlag=suppressDialogsFlag;

                    Common::result result=Common::result_missing;
                    result=smgr->userMatch(p,SBIDSongPtr(),selectedSongPtr);
                    if(result==Common::result_canceled)
                    {
                        qDebug() << SB_DEBUG_INFO << "none selected -- exit from import";
                        return 0;
                    }

                    //	Create song
                    if(result==Common::result_missing)
                    {
                        selectedSongPtr=smgr->createInDB(p);
                        _numNewSongs++;
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
                if(entityPtr->songPerformerID!=selectedSongPtr->songOriginalPerformerID())
                {
                    qDebug() << SB_DEBUG_INFO << entityPtr->songPerformerID;
                    qDebug() << SB_DEBUG_INFO << selectedSongPtr->songOriginalPerformerID();
                    selectedSongPtr->addSongPerformance(entityPtr->songPerformerID,entityPtr->year,entityPtr->notes);
                }
            }
        }
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step:validateSongs",progressCurrentValue++,progressMaxValue);
    }
    ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,"step:validateSongs");
    qDebug() << SB_DEBUG_INFO;
    ProgressDialog::instance()->finishDialog(__SB_PRETTY_FUNCTION__,0);
    ProgressDialog::instance()->stats();

    {	//	DEBUG
        QVectorIterator<MLentityPtr> eIT(list);
        qDebug() << SB_DEBUG_INFO << "START: entityList" << list.count();
        while(eIT.hasNext())
        {
            MLentityPtr ePtr=eIT.next();
            if(ePtr && !ePtr->errorFlag())
            {
                qDebug() << SB_DEBUG_INFO
                     << "albumID=" << ePtr->albumID
                     << ":albumPerformerID=" << ePtr->albumPerformerID
                     << ":songID=" << ePtr->songID
                     << ":performerID=" << ePtr->songPerformerID
                     << ":albumPosition=" << ePtr->albumPosition
                     << ":songTitle=" << ePtr->songTitle
                     << ":songPerformerName=" << ePtr->songPerformerName
                     << ":mrg2albumPos=" << ePtr->mergedToAlbumPosition
                     << ":removedFlag=" << ePtr->removedFlag
                ;
            }
        }
        qDebug() << SB_DEBUG_INFO << "START: directory2AlbumPathMap";
        QHashIterator<QString,MLalbumPathPtr> albumIT(directory2AlbumPathMap);
        while(albumIT.hasNext())
        {
            albumIT.next();
            MLalbumPathPtr apPtr=albumIT.value();
            qDebug() << SB_DEBUG_INFO
                     << apPtr->albumID
                     << apPtr->albumTitle
                     << apPtr->albumPerformerID
                     << apPtr->albumPerformerName
                     << apPtr->errorFlag()
                     << apPtr->multipleEntriesFlag()
            ;
        }
        qDebug() << SB_DEBUG_INFO << "ALL: END";
    }
    return 1;
}

QStringList
MusicLibrary::_greatestHitsAlbums() const
{
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
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
