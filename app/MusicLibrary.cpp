#include <QDir>

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
#include "SqlQuery.h"

///	Public methods
MusicLibrary::MusicLibrary(QObject *parent) : QObject(parent)
{
}

void
MusicLibrary::rescanMusicLibrary()
{
    QString dialogStep;
    const PropertiesPtr properties=Context::instance()->properties();
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    const QString databaseRestorePoint=dal->createRestorePoint();
    const bool suppressDialogsFlag=Context::instance()->properties()->configValue(Configuration::sb_smart_import).toInt();

    //	Important lists: all have `path' as key (in lowercase)
    QHash<QString,MLalbumPathPtr> directory2AlbumPathMap;	//	album path map

    //	SBIDManager
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumMgr* aMgr=cm->albumMgr();

    //	Init
    ProgressDialog::instance()->startDialog(__SB_PRETTY_FUNCTION__,"Scan Music Library",7);
    ProgressDialog::instance()->setOwnerOnly(__SB_PRETTY_FUNCTION__);

    _numNewSongs=0;
    _numNewPerformers=0;
    _numNewAlbums=0;

    const qsizetype numOnlinePerformances=SBIDOnlinePerformance::totalNumberOnlinePerformances()+100;
    qsizetype progressCurrentValue=0;
    qsizetype progressMaxValue=numOnlinePerformances;

    QElapsedTimer time; time.start();
    QVector<MLentityPtr> foundEntities=_retrievePaths(__SB_PRETTY_FUNCTION__,time,progressMaxValue);    //  dlg:#1

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
    QHash<QString,MLperformancePtr> pathToSong=_retrieveExistingData(__SB_PRETTY_FUNCTION__,time);  //  dlg:#2
    QHash<QString,bool> existingPath=_initializeExistingPath(pathToSong);

    if(1)
    {
        QString search1a="genesis";
        QString search1b="foxtrot";
        QString search2="sunday bloody sunday";
        qDebug() << SB_DEBUG_INFO << "Partial song list, looking for (" << search1a << " and " << search1b << ") or " << search2;
        QHashIterator<QString,MLperformancePtr> ptsIT(pathToSong);

        while(ptsIT.hasNext())
        {
            ptsIT.next();
            QString key=ptsIT.key();

            if((key.contains(search1a) && key.contains(search1b)) ||key.contains(search2))
            {
                qDebug() << SB_DEBUG_INFO << key;
            }
        }
        qDebug() << SB_DEBUG_INFO << "Finish";
    }

    ///////////////////////////////////////////////////////////////////////////////////
    ///	Section C:	Determine new songs and retrieve meta data for these
    ///////////////////////////////////////////////////////////////////////////////////
    QMutableVectorIterator<MLentityPtr> feIT(foundEntities);
    QVector<MLentityPtr> newEntities;

    progressCurrentValue=0;
    progressMaxValue=foundEntities.count();
    dialogStep="step3:retrieveMetadata";
    ProgressDialog::instance()->setLabelText(__SB_PRETTY_FUNCTION__,"Retrieving music data");           //  dlg:#3
    ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,dialogStep,progressCurrentValue,progressMaxValue);
    time.restart();

    while(feIT.hasNext())
    {
        MLentityPtr ePtr=feIT.next();

        if(pathToSong.contains(ePtr->key))
        {
            pathToSong[ePtr->key]->pathExists=1;
        }
        else if(ePtr->errorFlag()==0)
        {
            newEntities.append(ePtr);

            //	Populate meta data
            MetaData md(_getSchemaRoot()+ePtr->filePath);

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
            if(ePtr->errorMsg.length())
            {
                qDebug() << SB_DEBUG_ERROR << ePtr->errorMsg;
            }
        }

        if(time.elapsed()>700)
        {
            ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,dialogStep,progressCurrentValue,progressMaxValue);
            time.restart();
        }
        progressCurrentValue++;
    }
    foundEntities=newEntities;
    ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,dialogStep);

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
                         << e->key
                         << e->filePath
                         << e->songTitle
                         << e->songPerformerName
                         << e->albumTitle
                         << e->albumPosition
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
    if(validateEntityList(__SB_PRETTY_FUNCTION__,foundEntities,directory2AlbumPathMap,MusicLibrary::validation_type_album,suppressDialogsFlag)==0)
                                                                                                        //  dlg:4-6
    {
        cancelFlag=1;
        ProgressDialog::instance()->startDialog(__SB_PRETTY_FUNCTION__,"Canceling",1);
        ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,"stepX:cancel");
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
        progressCurrentValue=0;
        progressMaxValue=foundEntities.count();
        ProgressDialog::instance()->setLabelText(__SB_PRETTY_FUNCTION__,"Saving album data");           //  dlg:7
        dialogStep="saveData";

        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,dialogStep,progressCurrentValue,progressMaxValue);
        feIT.toFront();
        while(feIT.hasNext())
        {
            MLentityPtr ePtr=feIT.next();

            if(!ePtr->errorFlag())
            {
                const QString title=QString("Compiling album '%1'").arg(ePtr->albumTitle);
                ProgressDialog::instance()->setLabelText(__SB_PRETTY_FUNCTION__,title);

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
        ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,dialogStep);

        bool resultFlag=cm->saveChanges(__SB_PRETTY_FUNCTION__,"Saving Changes");

        if(resultFlag==0)
        {
            dal->restore(databaseRestorePoint);
        }
    }

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
        else if(entityPtr->isImported==0)
        {
            errors[entityPtr->filePath]="This song has NOT been imported. Like reason is this song being a duplicate";
        }
    }
    qDebug() << SB_DEBUG_ERROR << errors;

    if(errors.count())
    {
        MusicImportResult mir(errors);
        mir.exec();
    }

    //	Refresh caches
    Context::instance()->controller()->refreshModels();
    Context::instance()->controller()->preloadAllSongs();

    ProgressDialog::instance()->finishDialog(__SB_PRETTY_FUNCTION__);
    ProgressDialog::instance()->hide();
    ProgressDialog::instance()->stats();

    QString resultTxt=QString("<P>%4 %1 songs<BR>%4 %2 performers<BR>%4 %3 albums").arg(_numNewSongs).arg(_numNewPerformers).arg(_numNewAlbums).arg(QChar(8226));
    SBMessageBox::createSBMessageBox(QString("<center>Added:</center>"),resultTxt,QMessageBox::Information,QMessageBox::Ok,QMessageBox::Ok,QMessageBox::Ok,1);

    return;
}

bool
MusicLibrary::validateEntityList(const QString& dialogOwner,QVector<MLentityPtr>& list, QHash<QString,MLalbumPathPtr>& directory2AlbumPathMap, const MusicLibrary::MLvalidationType validationType, bool suppressDialogsFlag)
{
    static const QString empty;
    QString dialogStep;
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();

    ProgressDialog::instance()->setLabelText(dialogOwner,"Preparing");

    if(1)
    {
        //  Step 1: Find matches on performer names and song titles.
        //          a:  Prepare database table
        if(1)   //  do match
        {
            QStringList sql;
            sql.append("TRUNCATE TABLE ___SB_SCHEMA_NAME___match;");

            QVectorIterator<MusicLibrary::MLentityPtr> eIT(list);
            while(eIT.hasNext())
            {
                MusicLibrary::MLentityPtr ePtr=eIT.next();
                if(ePtr && ePtr->errorFlag()==0)
                {
                    sql.append(QString("INSERT INTO ___SB_SCHEMA_NAME___match "
                        "("
                            "id, "

                            // "file_path, "
                            // "parent_directory_name, "
                            // "parent_directory_path, "
                            // "absolute_parent_directory_path, "
                            // "extension, "

                            // "record_title, "
                            // "record_position, "

                            "record_artist_name, "
                            "record_artist_soundex, "
                            "song_artist_name, "
                            "song_artist_soundex, "
                            "song_title, "
                            "song_soundex "
                        ")"
                        "VALUES"
                        "( "
                            "%1, "
                            // "'%2','%3','%4','%5','%6', "
                            // "'%7',%8,"
                            "'%2','%3','%4','%5','%6','%7' "
                        ");")
                                   .arg(ePtr->ID)

                                   // .arg(Common::escapeSingleQuotes(ePtr->filePath))
                                   // .arg(Common::escapeSingleQuotes(ePtr->parentDirectoryName))
                                   // .arg(Common::escapeSingleQuotes(ePtr->parentDirectoryPath))
                                   // .arg(Common::escapeSingleQuotes(ePtr->absoluteParentDirectoryPath))
                                   // .arg(Common::escapeSingleQuotes(ePtr->extension))

                                   // .arg(Common::escapeSingleQuotes(ePtr->albumTitle))
                                   // .arg(ePtr->albumPosition)

                                   .arg(Common::escapeSingleQuotes(ePtr->albumPerformerName))
                                   .arg(Common::soundex(ePtr->albumPerformerName))
                                   .arg(Common::escapeSingleQuotes(ePtr->songPerformerName))
                                   .arg(Common::soundex(ePtr->songPerformerName))
                                   .arg(Common::escapeSingleQuotes(ePtr->songTitle))
                                   .arg(Common::soundex(ePtr->songTitle))
                        )
                    ;
                }
            }

            //  Insert performers from directory2AlbumPathMap
            QHashIterator<QString,MLalbumPathPtr> albumIT(directory2AlbumPathMap);
            albumIT=QHashIterator<QString,MLalbumPathPtr>(directory2AlbumPathMap);
            qsizetype ID=list.size();
            while(albumIT.hasNext())
            {
                albumIT.next();
                MLalbumPathPtr apPtr=albumIT.value();
                if(apPtr->albumPerformerName.length())
                {
                    sql.append(QString("INSERT INTO ___SB_SCHEMA_NAME___match "
                        "("
                            "id, "

                            "record_artist_name, "
                            "record_artist_soundex "
                        ")"
                        "VALUES"
                        "( "
                            "%1, "
                            "'%2','%3' "
                        ");")
                                   .arg(ID++)

                                   .arg(Common::escapeSingleQuotes(apPtr->albumPerformerName))
                                   .arg(Common::soundex(apPtr->albumPerformerName))
                        )
                    ;
                }
            }

            dialogStep="stepA:populateMatchTable";
            dal->executeBatch(sql,dialogOwner,"Preparing",dialogStep,1,0,1);                                //  dlg:#1

            //      b: match performers
            sql.clear();

            QStringList matchSrcColumn;
            matchSrcColumn.append("a.name");
            matchSrcColumn.append("a.sort_name");
            matchSrcColumn.append("am.artist_alternative_name");

            QStringList matchDstColumn;
            matchDstColumn.append("%1_artist_name");
            matchDstColumn.append("%1_artist_name_rev");

            QStringList entityList;
            entityList.append("song");
            entityList.append("record");

            QStringList articles;
            articles.append(Common::articles());
            QHash<QString,QString> andWords;
            andWords["and"]="&";
            andWords["en"]="&";
            andWords["und"]="&";
            andWords["e"]="&";
            andWords["y"]="&";
            andWords["vs"]="&";
            andWords["vs."]="&";
            andWords["ft."]="&";
            andWords["feat."]="&";
            andWords["feat"]="&";

            for(const auto& entity: entityList)
            {
                for(int switchPerformerName=0;switchPerformerName<2;switchPerformerName++)
                {
                    //  Before we do the matching, populate the _cmp column with what we want to match to.
                    //  If 0, then original performer name,
                    //  If 1, attempt to switch artist1 & artist2 to artist2 & artist1
                    if(switchPerformerName==0)
                    {
                        sql.append
                            (
                                QString
                                    (
                                        "UPDATE "
                                            "___SB_SCHEMA_NAME___match "
                                        "SET "
                                            "%1_artist_name_cmp= %1_artist_name "
                                        "WHERE "
                                            "%1_artist_match_score IS NULL "
                                        ";"
                                    )
                                    .arg(entity)
                            );

                    }
                    else
                    {
                        sql.append
                            (
                                QString
                                    (
                                        "UPDATE "
                                            "___SB_SCHEMA_NAME___match "
                                        "SET "
                                            "%1_artist_name_cmp= "
                                                "SUBSTRING(%1_artist_name ,POSITION('& ' IN %1_artist_name)+1, LENGTH(%1_artist_name)) "
                                                "|| ' & ' "
                                                "|| SUBSTRING(%1_artist_name ,0 , POSITION(' &' IN %1_artist_name)) "
                                        "WHERE "
                                            "%1_artist_name ILIKE '%&%' AND "
                                            "%1_artist_match_score IS NULL "
                                        ";"
                                    )
                                    .arg(entity)
                            );
                    }

                    for(const auto& article: articles)
                    {
                        for(int matchSrcColumnType=0;matchSrcColumnType<matchSrcColumn.size();matchSrcColumnType++)
                        {
                            //  i=0:    match on match.<entity>_artist_name and artist.name
                            //  i=1:    match on match.<entity>_artist_name and artist.sort_name
                            //  i=2:    match on match.<entity>_artist_name and artist_match.artist_alternative_name

                            QString leftArg;
                            QString rightArg;
                            QString joinClause;

                            if(matchSrcColumnType==2)
                            {
                                joinClause=QString("JOIN ___SB_SCHEMA_NAME___artist_match am ON am.artist_correct_name=a.name");
                            }
                            else
                            {
                                joinClause=empty;
                            }

                            for(const auto& andWord: andWords.keys())
                            {
                                //  process article
                                leftArg=QString("TRIM(REGEXP_REPLACE(LOWER(%1_artist_name_cmp),'\\y%2\\y','','g'))").arg(entity).arg(article);
                                rightArg=QString("TRIM(REGEXP_REPLACE(LOWER(%1),'\\y%2\\y','','g'))").arg(matchSrcColumn[matchSrcColumnType]).arg(article);

                                //  process andWord
                                leftArg=QString("TRIM(REGEXP_REPLACE(%1,'\\y%2\\y','%3','g'))").arg(leftArg).arg(andWord).arg(andWords[andWord]);
                                rightArg=QString("TRIM(REGEXP_REPLACE(%1,'\\y%2\\y','%3','g'))").arg(rightArg).arg(andWord).arg(andWords[andWord]);

                                sql.append
                                (
                                    //  Match on name
                                    QString
                                    (
                                        "UPDATE "
                                            "___SB_SCHEMA_NAME___match m "
                                        "SET "
                                            "%1_artist_match_score=0, "
                                            "%1_artist_id=a.artist_id, "
                                            "%1_artist_match_method='%5:%6:%7:%8:%9'"
                                        "FROM "
                                            "___SB_SCHEMA_NAME___artist a "
                                                "%2 "
                                        "WHERE "
                                            "%3 = %4 AND "
                                            "COALESCE(%1_artist_name_cmp,'')!='' AND "
                                            "m.song_artist_match_score IS NULL "
                                            "-- %5:%6:%7:%8:%9"
                                    )
                                        .arg(entity)
                                        .arg(joinClause)
                                        .arg(leftArg)
                                        .arg(rightArg)

                                        .arg(entity)
                                        .arg(switchPerformerName)
                                        .arg(article)
                                        .arg(matchSrcColumnType)
                                        .arg(andWord)
                                )
                                ;

                                //  Match on diacritical removed alphanumerical lowercase characters only
                                leftArg=QString
                                        (
                                            "REGEXP_REPLACE"
                                            "("
                                                "TRANSLATE"
                                                    "("
                                                        "%1, "
                                                        "'\303\200\303\201\303\202\303\203\303\204\303\205\303\206\303\207\303\210\303\211\303\212\303\213\303\214\303\215\303\216\303\217\303\221\303\222\303\223\303\224\303\225\303\226\303\230\303\231\303\232\303\233\303\234\303\235\303\237\303\240\303\241\303\242\303\243\303\244\303\245\303\246\303\247\303\250\303\251\303\252\303\253\303\254\303\255\303\256\303\257\303\261\303\262\303\263\303\264\303\265\303\266\303\270\303\271\303\272\303\273\303\274\303\275\303\277',"
                                                        "'AAAAAAACEEEEIIIINOOOOOOUUUUYSaaaaaaaceeeeiiiinoooooouuuuyy'"
                                                    "),"
                                                "'[^a-zA-Z0-9]+',"
                                                "'',"
                                                "'g'"
                                            ") "
                                        )
                                            .arg(leftArg)
                                    ;
                                rightArg=QString
                                        (
                                            "REGEXP_REPLACE"
                                            "("
                                                "TRANSLATE"
                                                    "("
                                                        "%1, "
                                                        "'\303\200\303\201\303\202\303\203\303\204\303\205\303\206\303\207\303\210\303\211\303\212\303\213\303\214\303\215\303\216\303\217\303\221\303\222\303\223\303\224\303\225\303\226\303\230\303\231\303\232\303\233\303\234\303\235\303\237\303\240\303\241\303\242\303\243\303\244\303\245\303\246\303\247\303\250\303\251\303\252\303\253\303\254\303\255\303\256\303\257\303\261\303\262\303\263\303\264\303\265\303\266\303\270\303\271\303\272\303\273\303\274\303\275\303\277',"
                                                        "'AAAAAAACEEEEIIIINOOOOOOUUUUYSaaaaaaaceeeeiiiinoooooouuuuyy'"
                                                    "),"
                                                "'[^a-zA-Z0-9]+',"
                                                "'',"
                                                "'g'"
                                            ") "
                                        )
                                            .arg(rightArg)
                                    ;
                                sql.append
                                (
                                    QString
                                    (
                                        "UPDATE "
                                            "___SB_SCHEMA_NAME___match m "
                                        "SET "
                                            "%1_artist_match_score=0, "
                                            "%1_artist_id=a.artist_id, "
                                            "%1_artist_match_method='%5:%6:%7:%8:%9'"
                                        "FROM "
                                            "___SB_SCHEMA_NAME___artist a "
                                                "%2 "
                                        "WHERE "
                                            "%3 = %4 AND "
                                            "COALESCE(%1_artist_name_cmp,'')!='' AND "
                                            "m.song_artist_match_score IS NULL "
                                            "-- %5:%6:%7:%8:%9"
                                    )
                                        .arg(entity)
                                        .arg(joinClause)
                                        .arg(leftArg)
                                        .arg(rightArg)

                                        .arg(entity)
                                        .arg(switchPerformerName)
                                        .arg(article)
                                        .arg(matchSrcColumnType)
                                        .arg(andWord)
                                )
                                ;
                            }
                        }
                    }
                }
            }

            dialogStep="stepB:validatePerformers";
            ProgressDialog::instance()->setLabelText(dialogOwner,"Validating Performers");
            dal->executeBatch(sql,dialogOwner,"Matching",dialogStep,1,0,1);                                     //  dlg:#2

            //      c: match songs
            sql.clear();

            sql.append
            (
                QString
                (
                    "UPDATE  "
                        "___SB_SCHEMA_NAME___match m "
                    "SET  "
                        "song_match_score=0, "
                        "song_id=s.song_id "
                    "FROM "
                        "___SB_SCHEMA_NAME___song s  "
                            "LEFT JOIN ___SB_SCHEMA_NAME___performance p ON  "
                                "p.song_id=s.song_id  "
                    "WHERE "
                        "LOWER(m.song_title)=LOWER(s.title) AND "
                        "m.song_artist_id=p.artist_id AND "
                        "m.song_match_score IS NULL "
                    "; "
                )
            );

            sql.append
            (
                QString
                (
                    "UPDATE  "
                        "___SB_SCHEMA_NAME___match m "
                    "SET  "
                        "song_match_score=0, "
                        "song_id=s.song_id "
                    "FROM "
                        "___SB_SCHEMA_NAME___song s  "
                            "LEFT JOIN ___SB_SCHEMA_NAME___performance p ON  "
                                "p.song_id=s.song_id  "
                    "WHERE "
                        "LOWER(REGEXP_REPLACE(m.song_title, '[^a-zA-Z0-9]+', '', 'g')) = LOWER(REGEXP_REPLACE(s.title, '[^a-zA-Z0-9]+', '', 'g')) AND "
                        "m.song_artist_id=p.artist_id AND "
                        "m.song_match_score IS NULL "
                    "; "
                )
            );

            // sql.append
            // (
            //     QString
            //     (
            //         "UPDATE  "
            //             "___SB_SCHEMA_NAME___match m "
            //         "SET  "
            //             "song_match_score=1, "
            //             "song_id=s.song_id "
            //         "FROM "
            //             "___SB_SCHEMA_NAME___song s  "
            //                 "LEFT JOIN ___SB_SCHEMA_NAME___performance p ON  "
            //                     "p.song_id=s.song_id  "
            //         "WHERE "
            //             "LOWER(m.song_title)=LOWER(s.title) AND "
            //             "m.song_artist_id!=p.artist_id AND "
            //             "m.song_match_score IS NULL "
            //         "; "
            //     )
            // );

            // sql.append
            // (
            //     QString
            //     (
            //         "UPDATE  "
            //             "___SB_SCHEMA_NAME___match m "
            //         "SET  "
            //             "song_match_score=2, "
            //             "song_id=s.song_id "
            //         "FROM "
            //             "___SB_SCHEMA_NAME___song s  "
            //                 "LEFT JOIN ___SB_SCHEMA_NAME___performance p ON  "
            //                     "p.song_id=s.song_id  "
            //         "WHERE "
            //             "LOWER(LEFT(m.song_title,LEAST(LENGTH(m.song_title),LENGTH(s.title))))=LOWER(LEFT(s.title,LEAST(LENGTH(m.song_title),LENGTH(s.title)))) AND "
            //             "m.song_artist_id=p.artist_id AND "
            //             "m.song_match_score IS NULL "
            //         "; "
            //     )
            // );

            // sql.append
            // (
            //     QString
            //     (
            //         "UPDATE  "
            //             "___SB_SCHEMA_NAME___match m "
            //         "SET  "
            //             "song_match_score=3, "
            //             "song_id=s.song_id "
            //         "FROM "
            //             "___SB_SCHEMA_NAME___song s  "
            //                 "LEFT JOIN ___SB_SCHEMA_NAME___performance p ON  "
            //                     "p.song_id=s.song_id  "
            //         "WHERE "
            //             "LOWER(LEFT(m.song_title,LEAST(LENGTH(m.song_title),LENGTH(s.title))))=LOWER(LEFT(s.title,LEAST(LENGTH(m.song_title),LENGTH(s.title)))) AND "
            //             "m.song_artist_id!=p.artist_id AND "
            //             "m.song_match_score IS NULL "
            //         "; "
            //     )
            // );
            dialogStep="stepB:validateSongs";
            ProgressDialog::instance()->setLabelText(dialogOwner,"Validating Songs");
            dal->executeBatch(sql,dialogOwner,"Matching",dialogStep,1,0,1);                                     //  dlg:#3

            //          d: match albums: TBD
        }   // do match

        //          e: integrate results back to list vector.
        QString readFromMatchSQL=
            QString
            (
                "SELECT "
                    "id, "
                    "COALESCE(record_artist_id,-1)  AS record_artist_id, "
                    "COALESCE(song_artist_id,-1)    AS song_artist_id, "
                    "COALESCE(song_id,-1)           AS song_id "
                "FROM "
                    "___SB_SCHEMA_NAME___match "
                ";"
            )
            ;

        dal->customize(readFromMatchSQL);
        QHash<int,MLentity> ID2e;
        QSqlQuery sq=dal->runSqlQuery(readFromMatchSQL);
        while(sq.next())
        {
            MLentity e;

            e.ID=sq.value(sq.record().indexOf("id")).toInt();

            e.albumPerformerID=sq.value(sq.record().indexOf("record_artist_id")).toInt();
            e.songPerformerID=sq.value(sq.record().indexOf("song_artist_id")).toInt();
            e.songID=sq.value(sq.record().indexOf("song_id")).toInt();

            ID2e[e.ID]=e;
        }

        if(0)
        {
            QHashIterator<int,MusicLibrary::MLentity> ieIT(ID2e);
            while(ieIT.hasNext())
            {
                ieIT.next();
                MusicLibrary::MLentity e=ieIT.value();
                qDebug() << SB_DEBUG_INFO
                     << e.ID
                     << e.albumPerformerName
                     << e.albumPerformerID
                     << e.songTitle
                     << e.songID
                     << e.songPerformerName
                     << e.songPerformerID
                ;
            }
        }

        if(0)
        {
            QVectorIterator<MusicLibrary::MLentityPtr> eIT(list);
            while(eIT.hasNext())
            {
                MusicLibrary::MLentityPtr ePtr=eIT.next();
                qDebug() << SB_DEBUG_INFO<< "before:"
                     << ePtr->ID
                     << ePtr->albumPerformerName
                     << ePtr->albumPerformerID
                     << ePtr->songTitle
                     << ePtr->songID
                     << ePtr->songPerformerName
                     << ePtr->songPerformerID
                ;
            }
        }

        QMutableVectorIterator<MusicLibrary::MLentityPtr> eIT(list);
        while(eIT.hasNext())
        {
            MusicLibrary::MLentityPtr ePtrOrg=eIT.next();
            MLentity eNew=ID2e[ePtrOrg->ID];

            if(eNew.ID!=-1)
            {
                ePtrOrg->albumPerformerID=eNew.albumPerformerID;
                ePtrOrg->songPerformerID=eNew.songPerformerID;
                ePtrOrg->songID=eNew.songID;
            }
            else
            {
                qDebug() << SB_DEBUG_INFO << "Cannot find" << ePtrOrg->ID;
            }
        }

        if(1)
        {
            QVectorIterator<MusicLibrary::MLentityPtr> eIT(list);
            while(eIT.hasNext())
            {
                MusicLibrary::MLentityPtr ePtr=eIT.next();
                qDebug() << SB_DEBUG_INFO << "updated:"
                     << ePtr->ID
                     << ePtr->albumPerformerName
                     << ePtr->albumPerformerID
                     << ePtr->songTitle
                     << ePtr->songID
                     << ePtr->songPerformerName
                     << ePtr->songPerformerID
                ;
            }
        }

        //  Step 2: Find matches on remaining performer names
        //          a:  create list of performers
        QStringList allPerformers;
        QHash<QString,int> name2PerformerIDMap;
        QHash<int,QString> performerID2CorrectNameMap;

        QMutableVectorIterator<MLentityPtr> listIT(list);
        while(listIT.hasNext())
        {
            const MLentityPtr ePtr=listIT.next();

            if(ePtr && !ePtr->errorFlag() && !ePtr->removedFlag)
            {
                if(ePtr->albumPerformerName.length() && !allPerformers.contains(ePtr->albumPerformerName))
                {
                    allPerformers.append(ePtr->albumPerformerName);
                    if(ePtr->albumPerformerID!=-1)
                    {
                        name2PerformerIDMap[ePtr->albumPerformerName]=ePtr->albumPerformerID;
                        performerID2CorrectNameMap[ePtr->albumPerformerID]=ePtr->albumPerformerName;
                    }
                }
                if(ePtr->songPerformerName.length() && !allPerformers.contains(ePtr->songPerformerName))
                {
                    allPerformers.append(ePtr->songPerformerName);
                    if(ePtr->songPerformerID!=-1)
                    {
                        name2PerformerIDMap[ePtr->songPerformerName]=ePtr->songPerformerID;
                        performerID2CorrectNameMap[ePtr->songPerformerID]=ePtr->songPerformerName;
                    }
                }
            }
        }

        if(1)
        {
            QStringListIterator slIT(allPerformers);
            while(slIT.hasNext())
            {
                qDebug() << SB_DEBUG_INFO << slIT.next();
            }
        }

        qsizetype progressCurrentValue=0;
        qsizetype progressMaxValue=allPerformers.size();
        dialogStep="stepA:validatePerformers";
        ProgressDialog::instance()->setLabelText(dialogOwner,"Validate Performers");
        ProgressDialog::instance()->update(dialogOwner,dialogStep,progressCurrentValue,progressMaxValue);       //  dlg:#3

        CacheManager* cm=Context::instance()->cacheManager();
        CachePerformerMgr* pemgr=cm->performerMgr();

        //		    b.	Go through all collected performer names and validate these.
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
                p.suppressDialogsFlag=suppressDialogsFlag;

                Common::result result=Common::result_missing;
                result=pemgr->userMatch(p,SBIDPerformerPtr(),selectedPerformerPtr);

                if(result==Common::result_canceled)
                {
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
            }
            ProgressDialog::instance()->update(dialogOwner,dialogStep,progressCurrentValue++,progressMaxValue);
        }
        ProgressDialog::instance()->finishStep(dialogOwner,dialogStep);

        //		    c.	Go through list and set performerID's accordingly.
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

                qDebug() << SB_DEBUG_INFO << ePtr->ID << ePtr->key << ":path=" << ePtr->filePath << ":albumPerformerID=" << ePtr->albumPerformerID;
            }
            qDebug() << SB_DEBUG_INFO << "START: directory2AlbumPathMap";
            QHashIterator<QString,MLalbumPathPtr> albumIT(directory2AlbumPathMap);
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

        if(1)
        {
            QVectorIterator<MusicLibrary::MLentityPtr> eIT(list);
            while(eIT.hasNext())
            {
                MusicLibrary::MLentityPtr ePtr=eIT.next();
                qDebug() << SB_DEBUG_INFO << "updated:"
                     << ePtr->ID
                     << ePtr->albumPerformerName
                     << ePtr->albumPerformerID
                     << ePtr->songTitle
                     << ePtr->songID
                     << ePtr->songPerformerName
                     << ePtr->songPerformerID
                ;
            }
        }

        if(1)
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

        //  Step 3: Process albums
        //          a:  identify albums with multiple performers
        SBIDPerformerPtr variousPerformerPtr=SBIDPerformer::retrieveVariousPerformers();
        QString albumPerformerName;
        QHashIterator<QString,MLalbumPathPtr> albumIT(directory2AlbumPathMap);
        while(albumIT.hasNext())
        {
            albumIT.next();
            MLalbumPathPtr apPtr=albumIT.value();
            albumPerformerName=apPtr->albumPerformerName;

            qDebug() << SB_DEBUG_INFO
                     << albumIT.key()
                     << ":albumTitle="  << apPtr->albumTitle
                     << ":albumPerformerID="  << apPtr->albumPerformerID
                     << ":albumPerformerName=" << apPtr->albumPerformerName
            ;
            qDebug() << SB_DEBUG_INFO
                     << albumIT.key()
                     << ":variousPerformerFlag=" << apPtr->variousPerformerFlag
                     << ":uniqueAlbumTitles.count()=" << apPtr->uniqueAlbumTitles.count()
                     << ":uniqueSongPerformerNames.count()=" << apPtr->uniqueSongPerformerNames.count()
                     << ":mulipleEntriesFlag=" << apPtr->multipleEntriesFlag()
            ;
            if(apPtr->uniqueAlbumTitles.count()>1)
            {
                qDebug() << SB_DEBUG_INFO << apPtr->uniqueAlbumTitles;
            }
            if(apPtr->multipleEntriesFlag()==1)
            {
                qDebug() << SB_DEBUG_INFO << apPtr->multipleEntriesFlag();
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

            if(0)
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

        }

        //          b: Go thru each albumPtr and find out if album can be found based on path name
        //			   This can only be done if directories are structured.
        PropertiesPtr properties=Context::instance()->properties();
        if(properties->configValue(Configuration::sb_performer_album_directory_structure_flag)=="1" && directory2AlbumPathMap.count()>0)
        {
            albumIT=QHashIterator<QString,MLalbumPathPtr>(directory2AlbumPathMap);
            while(albumIT.hasNext())
            {
                albumIT.next();
                MLalbumPathPtr apPtr=albumIT.value();
                SBIDAlbumPtr aPtr=SBIDAlbum::retrieveAlbumByPath(apPtr->path);
                if(aPtr && apPtr->errorFlag()==0)
                {
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

        //		    c.	Actual user validation of albums
        CacheAlbumMgr* amgr=cm->albumMgr();
        albumIT=QHashIterator<QString,MLalbumPathPtr>(directory2AlbumPathMap);
        while(albumIT.hasNext())
        {
            albumIT.next();
            MLalbumPathPtr apPtr=albumIT.value();
            SBIDAlbumPtr selectedAlbumPtr;

            if(apPtr)
            {
                if(0)
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
                }

                if(apPtr->errorFlag()==0)
                {

                    //	Only create collection album if album is not found.
                    if(apPtr->multipleEntriesFlag() && apPtr->albumID==-1)
                    {
                        //	No validation needed -- create collection album
                        Common::sb_parameters p;
                        p.albumTitle=apPtr->albumTitle;
                        p.performerID=apPtr->albumPerformerID;
                        p.year=apPtr->year;
                        p.genre=apPtr->genre;
                        selectedAlbumPtr=amgr->createInDB(p);
                        _numNewAlbums++;

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

                        if(!originalAlbumPtr)
                        {
                            qDebug() << SB_DEBUG_WARNING << "EMPTY!";
                        }

                        Common::result result=Common::result_missing;
                        if(!originalAlbumPtr)
                        {
                            result=amgr->userMatch(p,originalAlbumPtr,selectedAlbumPtr);
                        }
                        if(result==Common::result_canceled)
                        {
                            qDebug() << SB_DEBUG_WARNING << "none selected -- exit from import";
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
                        qDebug()
                            << SB_DEBUG_WARNING
                            << "ASSIGNMENT VARIOUS PERFORMER:"
                            << apPtr->albumPerformerName;
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


        if(1)
        {
            QHashIterator<QString,MLalbumPathPtr> albumIT(directory2AlbumPathMap);
            while(albumIT.hasNext())
            {
                albumIT.next();
                MLalbumPathPtr apPtr=albumIT.value();
                qDebug() << SB_DEBUG_INFO
                         << "New albums"
                         << ":albumID=" << apPtr->albumID
                         << ":albumTitle=" << apPtr->albumTitle
                         << ":albumPermerID=" << apPtr->albumPerformerID
                         << ":albumPerformerName=" << apPtr->albumPerformerName
                         << ":path=" << apPtr->path
                         << "multipleEntriesFlag:" << apPtr->multipleEntriesFlag()
                ;
            }
        }

        //		d.	Assign album data to entity pointers
        progressCurrentValue=0;
        progressMaxValue=list.count();
        dialogStep="stepB:validateAlbums";
        ProgressDialog::instance()->setLabelText(dialogOwner,"Validating Albums");
        ProgressDialog::instance()->update(dialogOwner,dialogStep,progressCurrentValue,progressMaxValue);       //  dlg:#2

        listIT=QMutableVectorIterator<MLentityPtr>(list);
        while(listIT.hasNext() && directory2AlbumPathMap.count()>0)
        {
            MLentityPtr ePtr=listIT.next();
            SBIDAlbumPtr selectedAlbumPtr;

            if(ePtr)
            {
                if(!ePtr->errorFlag() && (!ePtr->removedFlag))
                {
                    if(0)
                    {
                        qDebug() << SB_DEBUG_INFO
                             << ePtr->albumID
                             << ePtr->albumPosition
                             << ePtr->mergedToAlbumPosition
                             << ePtr->albumTitle
                             << ePtr->albumPerformerID
                             << ePtr->removedFlag
                             << ePtr->errorFlag()
                        ;
                    }

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
            ProgressDialog::instance()->update(dialogOwner,dialogStep,progressCurrentValue++,progressMaxValue);
        }
        ProgressDialog::instance()->finishStep(dialogOwner,dialogStep);

        if(1)
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

        //  Step 3: Validate songs
        progressCurrentValue=0;
        progressMaxValue=list.count();
        dialogStep="stepC:validateSongs";
        ProgressDialog::instance()->setLabelText(dialogOwner,"Validating Songs");
        ProgressDialog::instance()->update(dialogOwner,dialogStep,progressCurrentValue,progressMaxValue);       //  dlg:#3
        listIT=QMutableVectorIterator<MLentityPtr>(list);
        CacheSongMgr* smgr=cm->songMgr();

        QHash<QString,int> songTitle2songIDMap;	//	key: <song title>:<song performer id>
        songTitle2songIDMap.reserve(list.count());
        while(listIT.hasNext())
        {
            MLentityPtr entityPtr=listIT.next();

            if(entityPtr && entityPtr->songID==-1)
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
                            qDebug() << SB_DEBUG_WARNING << "none selected -- exit from import";
                            return 0;
                        }
                        qDebug() << SB_DEBUG_INFO << result;

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
                        selectedSongPtr->addSongPerformance(entityPtr->songPerformerID,entityPtr->year,entityPtr->notes);
                    }
                }
            }
            ProgressDialog::instance()->update(dialogOwner,dialogStep,progressCurrentValue++,progressMaxValue);
        }
        ProgressDialog::instance()->finishStep(dialogOwner,dialogStep);

        return 1;
    }
    else
    {
        //  OLD CODE
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
        //  DataAccessLayer* dal=Context::instance()->dataAccessLayer();
        int progressCurrentValue=0;
        int progressMaxValue=0;
        QHashIterator<QString,MLalbumPathPtr> albumIT(directory2AlbumPathMap);

        if(0)
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
            if(apPtr->albumPerformerName.length() && !allPerformers.contains(apPtr->albumPerformerName))
            {
                allPerformers.append(apPtr->albumPerformerName);
            }
        }

        QHash<QString,int> name2PerformerIDMap;
        QHash<int,QString> performerID2CorrectNameMap;

        QStringList ignoreClasses;
        ignoreClasses << "CacheTemplate" << "Preloader";
        progressCurrentValue=0;
        progressMaxValue=allPerformers.count();
        //QString dialogStep;
        dialogStep="stepA:validatePerformers";
        ProgressDialog::instance()->setLabelText(dialogOwner,"Validate Performers");
        ProgressDialog::instance()->update(dialogOwner,dialogStep,progressCurrentValue,progressMaxValue);       //  dlg:#1

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
                p.suppressDialogsFlag=suppressDialogsFlag;

                Common::result result=Common::result_missing;
                result=pemgr->userMatch(p,SBIDPerformerPtr(),selectedPerformerPtr);

                if(result==Common::result_canceled)
                {
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
            }
            ProgressDialog::instance()->update(dialogOwner,dialogStep,progressCurrentValue++,progressMaxValue);
        }
        ProgressDialog::instance()->finishStep(dialogOwner,dialogStep);

        if(0)
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

        if(0)
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

        //      d:  identify albums with multiple performers
        QString albumPerformerName;
        albumIT=QHashIterator<QString,MLalbumPathPtr>(directory2AlbumPathMap);
        while(albumIT.hasNext())
        {
            albumIT.next();
            MLalbumPathPtr apPtr=albumIT.value();
            albumPerformerName=apPtr->albumPerformerName;

            qDebug() << SB_DEBUG_INFO
                     << albumIT.key()
                     << ":albumTitle="  << apPtr->albumTitle
                     << ":albumPerformerID="  << apPtr->albumPerformerID
                     << ":albumPerformerName=" << apPtr->albumPerformerName
            ;
            qDebug() << SB_DEBUG_INFO
                     << albumIT.key()
                     << ":variousPerformerFlag=" << apPtr->variousPerformerFlag
                     << ":uniqueAlbumTitles.count()=" << apPtr->uniqueAlbumTitles.count()
                     << ":uniqueSongPerformerNames.count()=" << apPtr->uniqueSongPerformerNames.count()
                     << ":mulipleEntriesFlag=" << apPtr->multipleEntriesFlag()
            ;
            if(apPtr->uniqueAlbumTitles.count()>1)
            {
                qDebug() << SB_DEBUG_INFO << apPtr->uniqueAlbumTitles;
            }
            if(apPtr->multipleEntriesFlag()==1)
            {
                qDebug() << SB_DEBUG_INFO << apPtr->multipleEntriesFlag();
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


        //  Mark records without albumPerformerID as invalid
        if(0)
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
        if(0)
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
                MLalbumPathPtr apPtr=albumIT.value();
                SBIDAlbumPtr aPtr=SBIDAlbum::retrieveAlbumByPath(apPtr->path);
                if(aPtr && apPtr->errorFlag()==0)
                {
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
                         << "New albums"
                         << ":albumID=" << apPtr->albumID
                         << ":albumTitle=" << apPtr->albumTitle
                         << ":albumPermerID=" << apPtr->albumPerformerID
                         << ":albumPerformerName=" << apPtr->albumPerformerName
                         << ":path=" << apPtr->path
                         << "multipleEntriesFlag:" << apPtr->multipleEntriesFlag()
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
                if(0)
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
                }

                if(apPtr->errorFlag()==0)
                {

                    //	Only create collection album if album is not found.
                    if(apPtr->multipleEntriesFlag() && apPtr->albumID==-1)
                    {
                        //	No validation needed -- create collection album
                        Common::sb_parameters p;
                        p.albumTitle=apPtr->albumTitle;
                        p.performerID=apPtr->albumPerformerID;
                        p.year=apPtr->year;
                        p.genre=apPtr->genre;
                        selectedAlbumPtr=amgr->createInDB(p);
                        _numNewAlbums++;

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

                        if(!originalAlbumPtr)
                        {
                            qDebug() << SB_DEBUG_WARNING << "EMPTY!";
                        }

                        Common::result result=Common::result_missing;
                        if(!originalAlbumPtr)
                        {
                            result=amgr->userMatch(p,originalAlbumPtr,selectedAlbumPtr);
                        }
                        if(result==Common::result_canceled)
                        {
                            qDebug() << SB_DEBUG_WARNING << "none selected -- exit from import";
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
                        qDebug()
                            << SB_DEBUG_WARNING
                            << "ASSIGNMENT VARIOUS PERFORMER:"
                            << apPtr->albumPerformerName;
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
        dialogStep="stepB:validateAlbums";
        ProgressDialog::instance()->setLabelText(dialogOwner,"Validating Albums");
        ProgressDialog::instance()->update(dialogOwner,dialogStep,progressCurrentValue,progressMaxValue);       //  dlg:#2

        listIT=QMutableVectorIterator<MLentityPtr>(list);
        while(listIT.hasNext() && directory2AlbumPathMap.count()>0)
        {
            MLentityPtr ePtr=listIT.next();
            SBIDAlbumPtr selectedAlbumPtr;

            if(ePtr)
            {
                if(!ePtr->errorFlag() && (!ePtr->removedFlag))
                {
                    if(0)
                    {
                        qDebug() << SB_DEBUG_INFO
                             << ePtr->albumID
                             << ePtr->albumPosition
                             << ePtr->mergedToAlbumPosition
                             << ePtr->albumTitle
                             << ePtr->albumPerformerID
                             << ePtr->removedFlag
                             << ePtr->errorFlag()
                        ;
                    }

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
            ProgressDialog::instance()->update(dialogOwner,dialogStep,progressCurrentValue++,progressMaxValue);
        }
        ProgressDialog::instance()->finishStep(dialogOwner,dialogStep);

        if(0)
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
        dialogStep="stepC:validateSongs";
        ProgressDialog::instance()->setLabelText(dialogOwner,"Validating Songs");
        ProgressDialog::instance()->update(dialogOwner,dialogStep,progressCurrentValue,progressMaxValue);       //  dlg:#3
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
                            qDebug() << SB_DEBUG_WARNING << "none selected -- exit from import";
                            return 0;
                        }
                        qDebug() << SB_DEBUG_INFO << result;

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
                        selectedSongPtr->addSongPerformance(entityPtr->songPerformerID,entityPtr->year,entityPtr->notes);
                    }
                }
            }
            ProgressDialog::instance()->update(dialogOwner,dialogStep,progressCurrentValue++,progressMaxValue);
        }
        ProgressDialog::instance()->finishStep(dialogOwner,dialogStep);

        if(0)
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
        }
    }
    return 1;
}

void
MusicLibrary::consistencyCheck() const
{
    const Qt::CaseSensitivity caseSensitive=_fileSystemCaseSensitive();
    qDebug() << SB_DEBUG_INFO << "File system case sensitive:" << caseSensitive;

    ProgressDialog::instance()->startDialog(__SB_PRETTY_FUNCTION__,"Consistency Check",3);

    const int numOnlinePerformances=SBIDOnlinePerformance::totalNumberOnlinePerformances()+100;
    qsizetype progressMaxValue=numOnlinePerformances;
    QElapsedTimer time; time.start();
    QVector<MLentityPtr> foundEntities=_retrievePaths(__SB_PRETTY_FUNCTION__,time,progressMaxValue);    //  dlg:#1

    QHash<QString,MLperformancePtr> pathToSong=_retrieveExistingData(__SB_PRETTY_FUNCTION__,time);      //  dlg:#2

    //  For testing purposes only
    bool doTest=0;
    QString search("Getting In Tune");

    qsizetype currentIndex=0;
    qsizetype maxIndex=foundEntities.size()+pathToSong.size()+std::max(foundEntities.size(),pathToSong.size());
    time.restart();
    const static QString dialogStep="step3:comparingData";
    ProgressDialog::instance()->setLabelText(__SB_PRETTY_FUNCTION__,"Comparing Data");                  //  dlg:#3

    //  Convert to vectors: db
    QVector<QString> dbPath;
    QHashIterator<QString,MLperformancePtr> itPTS(pathToSong);
    while(itPTS.hasNext())
    {
        itPTS.next();
        MLperformancePtr mpPtr=itPTS.value();
        if(mpPtr->path.size()>0)
        {
            dbPath.append(mpPtr->path);
            if(doTest && mpPtr->path.contains(search))
            {
                qDebug() << SB_DEBUG_INFO << "*****DB" << mpPtr->path;
            }
            if(time.elapsed()>700)
            {
                ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,dialogStep,currentIndex,maxIndex);
                time.restart();
            }
            currentIndex++;

            if(currentIndex % 2500==0)
            {
                QThread::msleep(10);
            }
        }
    }
    std::sort(dbPath.begin(),dbPath.end());

    //  Convert to vectors: storage
    QVector<QString> hdPath;
    QVectorIterator<MLentityPtr> itFE(foundEntities);
    while(itFE.hasNext())
    {
        MLentityPtr mePtr=itFE.next();
        if(mePtr->filePath.size())
        {
            hdPath.append(mePtr->filePath);
            if(doTest && mePtr->filePath.contains(search))
            {
                qDebug() << SB_DEBUG_INFO << "*****HD" << mePtr->filePath;
            }
            if(time.elapsed()>700)
            {
                ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,dialogStep,currentIndex,maxIndex);
                time.restart();
            }
            currentIndex++;

            if(currentIndex % 2500==0)
            {
                QThread::msleep(10);
            }
        }
    }
    std::sort(hdPath.begin(),hdPath.end());

    qDebug() << SB_DEBUG_INFO << "dbPath:" << dbPath.size();
    qDebug() << SB_DEBUG_INFO << "hdPath:" << hdPath.size();

    QVectorIterator<QString> itDP(dbPath);
    QVectorIterator<QString> itHP(hdPath);
    QString d;
    QString h;
    bool finish=0;
    qsizetype total=0;
    qsizetype matching=0;
    if(itDP.hasNext() && !d.size())
    {
        d=itDP.next();
    }
    if(itHP.hasNext() && !h.size())
    {
        h=itHP.next();
    }

    QVector<QString> dbMissing;
    QVector<QString> hdMissing;
    do
    {
        bool incD=0;
        bool incH=0;

        if(!d.size() || !h.size())
        {
            finish=1;
        }
        else
        {
            //  qDebug() << SB_DEBUG_INFO << "d" << d << "h" << h;

            //  if(d==h)
            if(d.compare(h,caseSensitive)==0)
            {
                //  match
                matching++;
                incD=1;
                incH=1;
                //  qDebug() << SB_DEBUG_INFO << "\tsame";
            }
            else if(d<h)
            {
                //      D:  1   2   3   5   6
                //      H:  1   2   5   6
                //
                //      D   H
                //      0   0
                //      1   1
                //      2   2
                //      3
                //          4
                //      5   5
                //      6
                //      7   7
                qDebug() << SB_DEBUG_INFO << "\tMissing on hd:" << d;
                incD=1;
                hdMissing.append(d);
            }
            else if(d>h)
            {
                qDebug() << SB_DEBUG_INFO << "\tMissing in db:" << h;
                incH=1;
                dbMissing.append(h);
            }

            if(incD)
            {
                if(itDP.hasNext())
                {
                    //  qDebug() << SB_DEBUG_INFO << "\tincrease d";
                    d=itDP.next();
                }
                else
                {
                    qDebug() << SB_DEBUG_INFO << "\tnothing left in d";
                    finish=1;
                }
            }
            if(incH)
            {
                if(itHP.hasNext())
                {
                    //  qDebug() << SB_DEBUG_INFO << "\tincrease h";
                    h=itHP.next();
                }
                else
                {
                    qDebug() << SB_DEBUG_INFO << "\tnothing left in h";
                    finish=1;
                }
            }
        }

        if(time.elapsed()>700)
        {
            ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,dialogStep,currentIndex,maxIndex);
            time.restart();
        }
        currentIndex++;

            if(currentIndex % 2500==0)
            {
                QThread::msleep(10);
            }
    }
    while(!finish);

    qDebug() << SB_DEBUG_INFO << "Total:" << total << "songs";
    qDebug() << SB_DEBUG_INFO << "Matching:" << matching << "songs";
    qDebug() << SB_DEBUG_INFO << "Missing in db:" << dbMissing.size();
    //  Collect errors
    QMap<QString,QString> errors;
    for(auto result : dbMissing)
    {
        errors[result]="does not exists in the database";
        qDebug() << SB_DEBUG_INFO << "not in db" << result;
    }
    qDebug() << SB_DEBUG_INFO << "Missing on hd:" << hdMissing.size();
    for(auto result : hdMissing)
    {
        qDebug() << SB_DEBUG_INFO << "not on hd" << result;
        errors[result]="is missing on hard drive";
    }

    ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,dialogStep);
    ProgressDialog::instance()->finishDialog(__SB_PRETTY_FUNCTION__);
    ProgressDialog::instance()->hide();
    ProgressDialog::instance()->stats();

    if(errors.count())
    {
        MusicImportResult mir(errors);
        mir.exec();
    }
    else
    {
        SBMessageBox::createSBMessageBox(QString("<center>Consistency Check Results:</center>"),"No errors detected.",QMessageBox::Information,QMessageBox::Ok,QMessageBox::Ok,QMessageBox::Ok,1);
    }

}

QStringList
MusicLibrary::_greatestHitsAlbums() const
{
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QStringList greatestHitsAlbums;
    QString q="SELECT title FROM greatest_hits_record";
    SqlQuery qID(q,db);
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
        QString q="SELECT artist_alternative_name, artist_correct_name FROM ___SB_SCHEMA_NAME___artist_match";
        dal->customize(q);
        SqlQuery queryList(q,db);
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
            "INSERT INTO ___SB_SCHEMA_NAME___artist_match "
            "( "
                "artist_alternative_name, "
                "artist_correct_name "
            ") "
            "SELECT "
                "'%1', "
                "'%2' "
            "WHERE "
                "NOT EXISTS "
                "("
                    "SELECT "
                        "NULL "
                    "FROM "
                        "___SB_SCHEMA_NAME___artist_match "
                    "WHERE "
                        "artist_alternative_name= '%1' AND "
                        "artist_correct_name= '%2' "
                ") ";
        dal->customize(q);
        SqlQuery insert(q,db);
        Q_UNUSED(insert);

        _alternativePerformerName2CorrectPerformerName[altPerformerName]=correctPerformerName;
    }
}

Qt::CaseSensitivity
MusicLibrary::_fileSystemCaseSensitive() const
{
    QTemporaryDir tmpDir;
    if(tmpDir.isValid())
    {
        //  Create `foo' in tmpDir
        QString fn=QString("%1/foo").arg(tmpDir.path());
        {
            //  Put in separate block, to make `foo' go out of scope
            QFile foo(fn);
            if (!foo.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                return Qt::CaseSensitive;
            }

            QTextStream out(&foo);
            out << "\n";
        }

        //  Check if we can read with upper case filename
        QString fnUC=fn.toUpper();

        QFile FOO(fnUC);
        if (!FOO.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return Qt::CaseSensitive;
        }

        //  At this point the file is available in uppercase.
        //  Now remove file.
        QFile::remove(fn);
        return Qt::CaseInsensitive;
    }
    return Qt::CaseSensitive;
}

QString
MusicLibrary::_getSchemaRoot() const
{
    const PropertiesPtr properties=Context::instance()->properties();
    const QString schema=properties->currentDatabaseSchema();

    return QString("%1/%2%3")
               .arg(Context::instance()->properties()->musicLibraryDirectory())
               .arg(schema)
               .arg((schema.length()?"/":""));
}

QHash<QString,bool>
MusicLibrary::_initializeExistingPath(const QHash<QString,MusicLibrary::MLperformancePtr>& pathToSong) const
{
    QHash<QString,bool> existingPath;
    QHashIterator<QString,MusicLibrary::MLperformancePtr> it(pathToSong);

    while(it.hasNext())
    {
        it.next();
        existingPath[it.key()]=0;
    }
    return existingPath;
}

QHash<QString,MusicLibrary::MLperformancePtr>
MusicLibrary::_retrieveExistingData(const QString& dialogOwner,QElapsedTimer& time) const
{
    static const QString dialogStep("step2:retrieveExistingData");
    QHash<QString,MLperformancePtr> pathToSong;

    SBSqlQueryModel* sqm=SBIDOnlinePerformance::retrieveAllOnlinePerformancesExtended();

    //	For the next sections, set up a progress bar.
    qsizetype progressCurrentValue=0;
    qsizetype progressMaxValue=sqm->rowCount();
    ProgressDialog::instance()->update(dialogOwner,dialogStep,progressCurrentValue,progressMaxValue);
    time.restart();

    //  For testing purposes only
    bool doTest=0;
    QString search("Puik Idee Ballade.ogg");

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

        if(doTest && performance.path.contains(search))
        {
            qDebug() << SB_DEBUG_INFO << "*****"
                     << performance.songPerformanceID
                     << performance.albumPerformanceID
                     << performance.onlinePerformanceID
                     << performance.path
                ;
        }

        if(time.elapsed()>700)
        {
            QString label=performance.path;
            if(label.length()>40)
            {
                label="..."+label.right(30);
            }
            label="Scanning song "+label;
            ProgressDialog::instance()->setLabelText(dialogOwner,label);
            ProgressDialog::instance()->update(dialogOwner,dialogStep,progressCurrentValue,progressMaxValue);
            time.restart();
        }
        progressCurrentValue++;
    }
    ProgressDialog::instance()->finishStep(dialogOwner,dialogStep);
    delete sqm;sqm=NULL;
    return pathToSong;
}

QVector<MusicLibrary::MLentityPtr>
MusicLibrary::_retrievePaths(const QString& dialogOwner, QElapsedTimer& time, qsizetype progressMaxValue) const
{
    const QString schemaRoot=_getSchemaRoot();
    QVector<MusicLibrary::MLentityPtr> foundEntities;
    int progressCurrentValue=0;
    static const QString dialogStep("step1:retrieveFiles");
    ProgressDialog::instance()->update(dialogOwner,dialogStep,progressCurrentValue,progressMaxValue);
    time.restart();

    ///////////////////////////////////////////////////////////////////////////////////
    ///	Section A:	Retrieve paths found in directory
    ///////////////////////////////////////////////////////////////////////////////////
    QDirIterator it(schemaRoot,
                    QDir::AllDirs | QDir::AllEntries | QDir::Files | QDir::NoSymLinks | QDir::Readable,
                    QDirIterator::Subdirectories);
    QCoreApplication::processEvents();
    int ID=9999;
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

            if(AudioDecoderFactory::fileSupportedFlag(fi,0))
            {
                if(fi.size()<10*1024)
                {
                    e.errorMsg=QString("File size too short: %1 bytes").arg(fi.size());
                }
                else
                {
                    foundEntities.append(std::make_shared<MLentity>(e));
                }
            }

            //	Update dialogbox every half a second or so
            if(time.elapsed()>700)
            {
                QString label=e.parentDirectoryName;
                if(label.length()>40)
                {
                    label=label.left(30);
                }
                label="Scanning album: "+label;
                ProgressDialog::instance()->setLabelText(dialogOwner,label);
                if(progressCurrentValue+1>progressMaxValue)
                {
                    progressMaxValue=progressCurrentValue+1;
                }
                ProgressDialog::instance()->update(dialogOwner,dialogStep,progressCurrentValue,progressMaxValue);
                time.restart();
            }
            progressCurrentValue++;
        }
    }
    ProgressDialog::instance()->finishStep(dialogOwner,dialogStep);
    return foundEntities;
}
