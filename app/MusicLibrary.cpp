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

    const int numOnlinePerformances=SBIDOnlinePerformance::totalNumberOnlinePerformances()+100;
    int progressCurrentValue=0;
    int progressMaxValue=numOnlinePerformances;
    ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step1:retrieveFiles",progressCurrentValue,progressMaxValue);

    const QString schemaRoot=
        Context::instance()->properties()->musicLibraryDirectory()
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
            e.absoluteParentDirectoryPath=fi.absoluteDir().absolutePath();
            e.parentDirectoryPath=e.absoluteParentDirectoryPath.mid(schemaRoot.length());
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
            ProgressDialog::instance()->setLabelText(__SB_PRETTY_FUNCTION__,label);
            ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step2:retrieveExisting",progressCurrentValue,progressMaxValue);
            time.restart();
        }
        progressCurrentValue++;
    }
    delete sqm;
    qDebug() << SB_DEBUG_INFO;
    ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,"step2:retrieveExisting");

    ///////////////////////////////////////////////////////////////////////////////////
    ///	Section C:	Determine new songs and retrieve meta data for these
    ///////////////////////////////////////////////////////////////////////////////////
    QMutableVectorIterator<MLentityPtr> feIT(foundEntities);
    int numNewSongs=0;

    progressCurrentValue=0;
    progressMaxValue=foundEntities.count();
    ProgressDialog::instance()->startDialog(__SB_PRETTY_FUNCTION__,"Getting meta data",1);
    ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step3:retrieveMetaData",progressCurrentValue,progressMaxValue);
    time.restart();

    qDebug() << SB_DEBUG_INFO;
    while(feIT.hasNext())
    {
        MLentityPtr ePtr=feIT.next();

        const QString key=ePtr->filePath.toLower();

        if(pathToSong.contains(key))
        {
            pathToSong[key]->pathExists=1;
            feIT.remove(); //	Gone, gone, gone. Not needed. Exists. Pleitte.
        }
        else if(ePtr->errorFlag()==0)
        {
            qDebug() << SB_DEBUG_INFO << key;

            //	Populate meta data
            MetaData md(schemaRoot+ePtr->filePath);

            //	Primary meta data
            ePtr->albumPosition=md.albumPosition();
            ePtr->albumTitle=md.albumTitle();
            ePtr->songPerformerName=_retrieveCorrectPerformerName(dal,md.songPerformerName());
            ePtr->songTitle=md.songTitle();

            //	Secondary meta data
            ePtr->albumPerformerName=ePtr->songPerformerName; // for now, default to <>
            ePtr->duration=md.duration();
            ePtr->genre=md.genre();
            ePtr->notes=md.notes();
            ePtr->year=md.year();

            //	Misc. data
            ePtr->searchKey=key;

            //	Check on album title, take parent directory name if not exists
            if(ePtr->albumTitle.length()==0 || (properties->configValue(Properties::sb_performer_album_directory_structure_flag)=="1"))
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
                qDebug() << SB_DEBUG_INFO << albumPath.albumTitle << albumPath.albumPerformerName;

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
                qDebug() << SB_DEBUG_INFO << albumPathPtr->uniqueAlbumTitles.count() << ePtr->albumTitle;
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
    ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,"step3:retrieveMetaData");
    ProgressDialog::instance()->finishDialog(__SB_PRETTY_FUNCTION__,1);	//	Not done yet

    if(1)
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
    bool cancelFlag=0;
    if(validateEntityList(foundEntities,directory2AlbumPathMap)==0)
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

            const QString title=QString("Compiling album '%1'").arg(ePtr->albumTitle);
            ProgressDialog::instance()->setLabelText(__SB_PRETTY_FUNCTION__,title);
            ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step4:save",progressCurrentValue++,progressMaxValue);

            if(!ePtr->errorFlag())
            {
                qDebug() << SB_DEBUG_INFO;
                SBIDAlbumPtr albumPtr=aMgr->retrieve(SBIDAlbum::createKey(ePtr->albumID));
                albumPtr->addAlbumPerformance(
                            ePtr->songID,
                            ePtr->songPerformerID,
                            ePtr->albumPosition,
                            ePtr->year,
                            ePtr->filePath,
                            ePtr->duration,
                            ePtr->notes);
                //	CWIP: do progressbox
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
    Context::instance()->controller()->refreshModels();
    Context::instance()->controller()->preloadAllSongs();

    ProgressDialog::instance()->hide();
    ProgressDialog::instance()->stats();

    qDebug() << SB_DEBUG_INFO;
    Common::sleep(5);
    qDebug() << SB_DEBUG_INFO;
    QString resultTxt=QString("<P>%4 %1 songs<BR>%4 %2 performers<BR>%4 %3 albums").arg(_numNewSongs).arg(_numNewPerformers).arg(_numNewAlbums).arg(QChar(8226));
    SBMessageBox::createSBMessageBox(QString("<center>Added:</center>"),resultTxt,QMessageBox::Information,QMessageBox::Ok,QMessageBox::Ok,QMessageBox::Ok,1);

    qDebug() << SB_DEBUG_INFO << "Finished";
    return;
}

bool
MusicLibrary::validateEntityList(QVector<MLentityPtr>& list, QHash<QString,MLalbumPathPtr>& directory2AlbumPathMap)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumMgr* amgr=cm->albumMgr();
    CachePerformerMgr* pemgr=cm->performerMgr();
    CacheSongMgr* smgr=cm->songMgr();
    SBIDPerformerPtr variousPerformerPtr=SBIDPerformer::retrieveVariousPerformers();
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    int progressCurrentValue=0;
    int progressMaxValue=0;
    QHashIterator<QString,MLalbumPathPtr> albumIT(directory2AlbumPathMap);

    _numNewSongs=0;
    _numNewPerformers=0;
    _numNewAlbums=0;

    qDebug() << SB_DEBUG_INFO;
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

        qDebug() << SB_DEBUG_INFO << performerName;
        int performerID=-1;
        if(!name2PerformerIDMap.contains(performerName))
        {
            SBIDPerformerPtr selectedPerformerPtr;
            Common::sb_parameters p;
            p.performerName=performerName;
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
                _numNewPerformers++;
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

    //		c.	Go through list and set performerID's accordingly.
    listIT=QMutableVectorIterator<MLentityPtr>(list);
    while(listIT.hasNext())
    {
        MLentityPtr ePtr=listIT.next();

        //	Set the correct performer ID's.
        ePtr->albumPerformerID=name2PerformerIDMap[ePtr->albumPerformerName];
        ePtr->songPerformerID=name2PerformerIDMap[ePtr->songPerformerName];

        //	Now correct the performer name themselves.
        ePtr->albumPerformerName=performerID2CorrectNameMap[ePtr->albumPerformerID];
        ePtr->songPerformerName=performerID2CorrectNameMap[ePtr->songPerformerID];
    }
    if(1)
    {
        qDebug() << SB_DEBUG_INFO;
        QHashIterator<QString,int> n2pIT(name2PerformerIDMap);
        while(n2pIT.hasNext())
        {
            n2pIT.next();
            qDebug() << SB_DEBUG_INFO << n2pIT.key() << n2pIT.value();
        }

    }
    albumIT=QHashIterator<QString,MLalbumPathPtr>(directory2AlbumPathMap);
    while(albumIT.hasNext())
    {
        albumIT.next();
        MLalbumPathPtr apPtr=albumIT.value();
        qDebug() << SB_DEBUG_INFO << apPtr->albumTitle << apPtr->albumPerformerID << apPtr->albumPerformerName;
        qDebug() << SB_DEBUG_INFO << apPtr->albumTitle << apPtr->variousPerformerFlag << apPtr->uniqueAlbumTitles.count() << apPtr->uniqueSongPerformerNames.count() << apPtr->multipleEntriesFlag();
        if(apPtr->uniqueAlbumTitles.count()>1)
        {
            qDebug() << SB_DEBUG_INFO << apPtr->uniqueAlbumTitles;
        }
        if(apPtr->multipleEntriesFlag()==1)
        {
            apPtr->albumPerformerID=variousPerformerPtr->performerID();
            apPtr->albumPerformerName=variousPerformerPtr->performerName();
        }
        else
        {
            apPtr->albumPerformerID=name2PerformerIDMap[apPtr->albumPerformerName];
            apPtr->albumPerformerName=performerID2CorrectNameMap[apPtr->albumPerformerID];
        }
    }


    {	//	DEBUG
        QVectorIterator<MLentityPtr> eIT(list);
        qDebug() << SB_DEBUG_INFO << "AFTER PERFORMER VALIDATION" << list.count();
        while(eIT.hasNext())
        {
            MLentityPtr ePtr=eIT.next();
            if(ePtr && ePtr->errorFlag()==0)
            {
                qDebug() << SB_DEBUG_INFO
                     << ePtr->albumID
                     << ePtr->albumPosition
                     << ePtr->songTitle
                     << ePtr->songPerformerName
                     << ePtr->mergedToAlbumPosition
                     << ePtr->albumTitle
                     << ePtr->albumID
                     << ePtr->albumPerformerName
                     << ePtr->albumPerformerID
                     << ePtr->removedFlag
                ;
            }
        }
        qDebug() << SB_DEBUG_INFO << "END";
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
    if(properties->configValue(Properties::sb_performer_album_directory_structure_flag)=="1")
    {
        albumIT=QHashIterator<QString,MLalbumPathPtr>(directory2AlbumPathMap);
        while(albumIT.hasNext())
        {
            albumIT.next();
            qDebug() << SB_DEBUG_INFO << albumIT.key();
            MLalbumPathPtr apPtr=albumIT.value();
            qDebug() << SB_DEBUG_INFO << apPtr->albumTitle << apPtr->path << apPtr->albumPerformerName << apPtr->albumPerformerID;
            SBIDAlbumPtr aPtr=SBIDAlbum::retrieveAlbumByPath(apPtr->path);
            if(aPtr)
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
            ;

            if(apPtr->multipleEntriesFlag())
            {
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

                //	Set up excluding album:
                //	-	if ePtr.albumID=-1 then empty,
                //	-	otherwise lookup existing album
                const SBIDAlbumPtr originalAlbumPtr=(apPtr->albumID==-1?SBIDAlbumPtr():SBIDAlbum::retrieveAlbum(apPtr->albumID));

                qDebug() << SB_DEBUG_INFO
                         << p.albumID
                         << p.albumTitle
                         << p.performerID
                         << p.performerName
                         << p.year
                         << p.genre
                ;
                Common::result result=amgr->userMatch(p,originalAlbumPtr,selectedAlbumPtr);
                qDebug() << SB_DEBUG_INFO << result;
                if(selectedAlbumPtr)
                {
                    qDebug() << SB_DEBUG_INFO << selectedAlbumPtr->text();
                }
                if(result==Common::result_canceled)
                {
                    qDebug() << SB_DEBUG_INFO << "none selected -- exit from import";
                    return 0;
                }
                if(result==Common::result_missing)
                {
                    qDebug() << SB_DEBUG_INFO;
                    //	If we work based on an existing album this means that the title has
                    //	changed. Therefore, the selectedAlbumPtr
                    if(originalAlbumPtr)
                    {
                        qDebug() << SB_DEBUG_INFO;
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
    while(listIT.hasNext())
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
                     << ePtr->removedFlag;

                MLalbumPathPtr apPtr=directory2AlbumPathMap[ePtr->absoluteParentDirectoryPath];
                if(apPtr)
                {
                    selectedAlbumPtr=amgr->retrieve(SBIDAlbum::createKey(apPtr->albumID));
                    if(selectedAlbumPtr)
                    {
                        ePtr->albumID=selectedAlbumPtr->albumID();
                        ePtr->albumPerformerID=selectedAlbumPtr->albumPerformerID();

                        if(apPtr->multipleEntriesFlag())
                        {
                            //	Overwrite and assign new album position if collection album.
                            ePtr->albumPosition=++(apPtr->maxPosition);
                        }
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
        qDebug() << SB_DEBUG_INFO << "BEFORE SONG VALIDATION" << list.count();
        while(eIT.hasNext())
        {
            MLentityPtr ePtr=eIT.next();
            if(ePtr && !ePtr->errorFlag())
            {
                qDebug() << SB_DEBUG_INFO
                     << ePtr->albumID
                     << ePtr->songID
                     << ePtr->albumPosition
                     << ePtr->songTitle
                     << ePtr->songPerformerName
                     << ePtr->mergedToAlbumPosition
                     << ePtr->removedFlag
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
                     << apPtr->multipleEntriesFlag()
            ;
        }
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

                    qDebug() << SB_DEBUG_INFO << p.songID << p.performerName;

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
