#include <QMessageBox>

#include "SBTabSongEdit.h"

#include "CacheManager.h"
#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "SBDialogSelectItem.h"
#include "SBIDPerformer.h"

SBTabSongEdit::SBTabSongEdit(QWidget* parent) : SBTab(parent,1)
{
}

void
SBTabSongEdit::handleEnterKey()
{
    save();
}

bool
SBTabSongEdit::hasEdits() const
{
    const SBKey key=this->currentScreenItem().key();
    const MainWindow* mw=Context::instance()->mainWindow();

    SBIDSongPtr songPtr=SBIDSong::retrieveSong(key);
    SB_RETURN_IF_NULL(songPtr,0);

    if(songPtr->songTitle()!=mw->ui.songEditTitle->text() ||
        songPtr->songOriginalPerformerName()!=mw->ui.songEditPerformerName->text() ||
        songPtr->songOriginalYear()!=mw->ui.songEditYearOfRelease->text().toInt() ||
        songPtr->notes()!=mw->ui.songEditNotes->text() ||
        songPtr->lyrics()!=mw->ui.songEditLyrics->toPlainText()
    )
    {
        return 1;
    }

    return 0;
}

///	Public slots
void
SBTabSongEdit::save() const
{
    //	Test cases:
    //	A1.	[simple rename] Bad - U2: change to Badaa. Should be simple renames.
    //	A2.	[simple rename w/ case] Badaa - U2 to BadaA. Should take into account case change.
    //	A3.	[switch original performer to completely new performer] BadaA - U2: change performer to U22.
    //	A4.	[merge song (within performer)] Badaa - U22 to Acrobat - U2. Note that album listing for Acrobat should include 'Bad' albums.
    //	A5.	[remove performances without album performances] Edit (no changes) and save Acrobat - U2 one more time to remove the U22 entry.

    //	B.	[switch original performer to non-original performer] Dancing Barefoot: change from Patti Smith to U2 and back

    //	C.	[switch original performer to different existing performer] "C" Moon Cry Like A Baby: Simple Minds -> U2

    //	D.	[Assign song to different song and its non-original performer]  Assign <whatever> to Sunday Bloody Sunday by The Royal Philharmonic Orchestra

    //	Refresh database
    //	F2.	[merge song (within performer)] Bad - U2 to Acrobat. Note that album listing for Acrobat should include 'Bad' albums.

    //	G.	[merge to different performer] "C" Moon Cry Like A Baby/Simple Minds -> "40"/U2

    //	H.	[merge song with existing song by renaming original performer] Get Lucky - Daft Poonk => Get Lucky - Daft Poonk & Squirrel W.
    //	I.	[simple edits]: 'Elusive Dreams ' -> 'Elusive Dreams': should just save the new title.

    //	Algorithm:
    //	1.	determine current original performance (orgOpPtr).
    //	2.	detect year, notes change -> metaDataChangeFlag
    //	3.	find same song performance based on edit data 'as is' (altOpPtr). If found, merge -> save. [merge duplicate]
    //	4.	if performer has changed:
    //		4.1	lookup changed performer (userMatch)
    //			4.1.1	if not found:
    //				-	create new performer,
    //				-	create new song performance,
    //				-	set originalPerformanceID of song
    //				-	set other attributes, save song -- completely new performer means no other matches can be found.
    //			4.1.2	if found:
    //				-	find if song performance exist with edited song title & changed performer.
    //					-	If found, merge -> save.
    //					-	If not found, find song with matching edit song title.
    //						-	if found: merge
    //						-	if not found:
    //							-	create new song performance,
    //							-	set originalPerformanceID of song
    //							-	set other attributes, save song -- completely new performer means no other matches can be found.
    //		4.2	(else) if performer has not changed, title has changed:
    //			-	find performance with org performer and edit title (userMatch)
    //				-	if found: merge
    //				-	if not found:
    //					-	set other attributes, save new title and other attributes.
    //	5.	Consolidate song performances -- remove non-original performances not appearing on
    //		any album performance

    //	Data
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QString restorePoint=dal->createRestorePoint();

    //	Cache Mgrs
    CacheManager* cm=Context::instance()->cacheManager();
    CacheSongPerformanceMgr* spMgr=cm->songPerformanceMgr();
    CachePerformerMgr* peMgr=cm->performerMgr();
    CacheSongMgr* sMgr=cm->songMgr();

    ScreenItem currentScreenItem=this->currentScreenItem();
    if(currentScreenItem.editFlag()==0)
    {
        qDebug() << SB_DEBUG_ERROR << "isEditFlag flag not set. Aborting.";
        return;
    }

    //	1.	Pointers to original song.
    SBIDSongPtr orgSongPtr=SBIDSong::retrieveSong(this->currentScreenItem().key());
    SBIDSongPerformancePtr orgSpPtr=orgSongPtr->originalSongPerformancePtr();

    SB_RETURN_VOID_IF_NULL(orgSongPtr);
    SB_RETURN_VOID_IF_NULL(orgSpPtr);

    //	Pointer to new song, only if found
    SBIDSongPtr newSongPtr;

    //	Attributes
    const MainWindow* mw=Context::instance()->mainWindow();
    QString editTitle=mw->ui.songEditTitle->text();
    QString editPerformerName=mw->ui.songEditPerformerName->text();
    int editYearOfRelease=mw->ui.songEditYearOfRelease->text().toInt();
    QString editNotes=mw->ui.songEditNotes->text();
    QString editLyrics=mw->ui.songEditLyrics->toPlainText();

    //	Flags
    bool metaDataChangedFlag=0;
    bool songTitleChangedFlag=0;
    bool mergedFlag=0;
    bool setMetaDataFlag=0;	//	if set, update meta data

    //	2.	Determine metaDataChangedFlag
    if(editYearOfRelease!=orgSpPtr->year() || editNotes!=orgSongPtr->notes() || editLyrics!=orgSongPtr->lyrics())
    {
        metaDataChangedFlag=1;
    }

    //	3.	Look up if song already exists.
    qDebug() << SB_DEBUG_INFO;
    {
        //	Keep the scope of altSpPtr local
        SBIDSongPerformancePtr altSpPtr=SBIDSongPerformance::retrieveSongPerformanceByPerformer(
                    editTitle,
                    editPerformerName,
                    orgSpPtr->songPerformanceID());

        if(altSpPtr)
        {
            newSongPtr=altSpPtr->songPtr();
            SB_RETURN_VOID_IF_NULL(newSongPtr);
            qDebug() << SB_DEBUG_INFO << "merge" << orgSongPtr->key() << orgSongPtr->ID();
            qDebug() << SB_DEBUG_INFO << "to" << newSongPtr->key() << newSongPtr->ID();
            sMgr->merge(orgSongPtr,newSongPtr);
            mergedFlag=1;
        }
    }

    //	4.	if performer has changed
    if(!mergedFlag && editPerformerName!=orgSpPtr->songPerformerName())
    {
        SBIDPerformerPtr newPPtr;

        //		4.1	lookup changed performer (userMatch)
        Common::sb_parameters p;
        p.performerName=editPerformerName;
        p.performerID=-1;
        Common::result result=peMgr->userMatch(p,SBIDPerformerPtr(),newPPtr);

        if(result==Common::result_canceled)
        {
            qDebug() << SB_DEBUG_INFO << "none selected -- exit from import";
            return;
        }
        else if(result==Common::result_missing)
        {
            newSongPtr=orgSongPtr;

        qDebug() << SB_DEBUG_INFO;
            //	4.1.1	if not found:
            //	Create performer
            Common::sb_parameters performer;
            performer.performerName=p.performerName;
            newPPtr=peMgr->createInDB(performer);

            //	Create new song performance
            p.songID=newSongPtr->songID();
            p.performerID=newPPtr->performerID();
            p.year=orgSpPtr->year();
            SBIDSongPerformancePtr newSpPtr=spMgr->createInDB(p);
            newSongPtr->addSongPerformance(newSpPtr);

            //	set originalPerformanceID of song
            newSongPtr->setOriginalPerformanceID(newSpPtr->songPerformanceID());

            //	set other attributes, save song -- completely new performer means no other matches can be found.
            setMetaDataFlag=1;
        }
        else if(result==Common::result_exists)
        {
            //	4.1.2	if found:
            //		find if song performance exist with edited song title & changed performer.
            SBIDSongPerformancePtr altSpPtr=SBIDSongPerformance::retrieveSongPerformanceByPerformer(
                        editTitle,
                        newPPtr->performerName(),
                        orgSpPtr->songPerformanceID());

            if(altSpPtr)
            {
                //		If found, merge -> save.
                newSongPtr=altSpPtr->songPtr();
                SB_RETURN_VOID_IF_NULL(newSongPtr);
                qDebug() << SB_DEBUG_INFO << "merge" << orgSongPtr->key() << orgSongPtr->ID();
                qDebug() << SB_DEBUG_INFO << "to" << newSongPtr->key() << newSongPtr->ID();
                sMgr->merge(orgSongPtr,newSongPtr);
                mergedFlag=1;
            }
            else
            {
                //		If not found, find song with matching edit song title.
                Common::sb_parameters p;
                p.songTitle=editTitle;
                p.performerID=newPPtr->performerID();
                p.performerName=newPPtr->performerName();
                p.year=orgSpPtr->year();
                p.notes=orgSpPtr->notes();

                qDebug() << SB_DEBUG_INFO << p.songID;
                Common::result result=sMgr->userMatch(p,SBIDSongPtr(),newSongPtr);
                if(result==Common::result_canceled)
                {
                    qDebug() << SB_DEBUG_INFO << "none selected -- exit from import";
                    return;
                }
                if(result==Common::result_exists)
                {
                    //		if found: merge
                    SB_RETURN_VOID_IF_NULL(newSongPtr);
                    qDebug() << SB_DEBUG_INFO << "merge" << orgSongPtr->key() << orgSongPtr->ID();
                    qDebug() << SB_DEBUG_INFO << "to" << newSongPtr->key() << newSongPtr->ID();
                    sMgr->merge(orgSongPtr,newSongPtr);
                    mergedFlag=1;
                }
                if(result==Common::result_missing)
                {
                    //		if not found:
                    //	Create new song performance
                    p.songID=orgSongPtr->songID();
                    p.performerID=newPPtr->performerID();
                    p.year=editYearOfRelease;
                    SBIDSongPerformancePtr newSpPtr=spMgr->createInDB(p);
                    newSongPtr->addSongPerformance(newSpPtr);

                    //	set originalPerformanceID of song
                    newSongPtr->setOriginalPerformanceID(newSpPtr->songPerformanceID());

                    //	set other attributes, save song -- completely new performer means no other matches can be found.
                    setMetaDataFlag=1;
                }
            }
        }
    }
    else
    {
        //	4.2	(else) if performer has not changed, title has changed:
        Common::sb_parameters p;
        p.songTitle=editTitle;
        p.performerID=orgSpPtr->songPerformerID();
        p.performerName=orgSpPtr->songPerformerName();
        p.year=orgSpPtr->year();
        p.notes=orgSpPtr->notes();

        qDebug() << SB_DEBUG_INFO << p.songID;
        Common::result result=sMgr->userMatch(p,SBIDSongPtr(),newSongPtr);
        if(result==Common::result_canceled)
        {
            qDebug() << SB_DEBUG_INFO << "none selected -- exit from import";
            return;
        }
        else if(result==Common::result_exists)
        {
            SB_RETURN_VOID_IF_NULL(newSongPtr);
            qDebug() << SB_DEBUG_INFO << "merge" << orgSongPtr->key() << orgSongPtr->ID();
            qDebug() << SB_DEBUG_INFO << "to" << newSongPtr->key() << newSongPtr->ID();
            sMgr->merge(orgSongPtr,newSongPtr);
            mergedFlag=1;
        }
        else if(result==Common::result_missing)
        {
            newSongPtr=orgSongPtr;
            newSongPtr->setSongTitle(editTitle);
            setMetaDataFlag=1;
        }
    }

    if(setMetaDataFlag && metaDataChangedFlag)
    {
        SB_RETURN_VOID_IF_NULL(newSongPtr);
        newSongPtr->setNotes(editNotes);
        newSongPtr->setLyrics(editLyrics);
    }

    //	5.	Consolidate song performances with what is on any album.
    //		If a non-original performer does not exist on an album, remove this (there is no other way to remove this).
    qDebug() << SB_DEBUG_INFO << mergedFlag;
    if(!mergedFlag)
    {
        qDebug() << SB_DEBUG_INFO;
        QMapIterator<int,SBIDSongPerformancePtr> it(newSongPtr->songPerformances());
        while(it.hasNext())
        {
            it.next();
            SBIDSongPerformancePtr spPtr=it.value();
            int songPerformanceID=spPtr->songPerformanceID();	//	CWIP: new data collection to keep track of this by songPerformanceID

            qDebug() << SB_DEBUG_INFO << songPerformanceID;
            QVectorIterator<SBIDAlbumPerformancePtr> apIT(newSongPtr->allPerformances());
            bool foundFlag=0;
            while(apIT.hasNext() && !foundFlag)
            {
                SBIDAlbumPerformancePtr apPtr=apIT.next();
                if(apPtr->songPerformanceID()==songPerformanceID)
                {
                    foundFlag=1;
                }
            }

            if(!foundFlag && editPerformerName!=spPtr->songPerformerName())
            {
                qDebug() << SB_DEBUG_INFO << "Removing song performance by " << spPtr->songPerformerName() << "id=" << spPtr->songPerformanceID();
                newSongPtr->removeSongPerformance(spPtr);
            }
        }
    }

    /* SB_RETURN_VOID_IF_NULL(orgSongPtr);

    SBIDSongPtr newSongPtr=orgSongPtr;
    SBIDSongPerformancePtr newSpPtr;	//	only populated if a new original spPtr is created.
    SBIDPerformerPtr newPPtr;

    qDebug() << SB_DEBUG_INFO << "orgSong" << orgSongPtr->text();
    qDebug() << SB_DEBUG_INFO << "newSong" << newSongPtr->text();

    bool simpleTitleChangedFlag=0;
    bool songTitleChangedFlag=0;

    qDebug() << SB_DEBUG_INFO << editTitle << editPerformerName;
    qDebug() << SB_DEBUG_INFO << newSongPtr->key() << orgSongPtr->songTitle() << newSongPtr->songOriginalPerformerName();
    qDebug() << SB_DEBUG_INFO << newSongPtr->songID();
    qDebug() << SB_DEBUG_INFO << "orgSongPtr:sb_song_performer_id" << orgSongPtr->originalSongPerformanceID();
    qDebug() << SB_DEBUG_INFO << "orgSongPtr:sb_song_id" << orgSongPtr->songID();
    qDebug() << SB_DEBUG_INFO << "newSongPtr:sb_song_performer_id" << newSongPtr->originalSongPerformanceID();
    qDebug() << SB_DEBUG_INFO << "newSongPtr:sb_song_id" << newSongPtr->songID();

    if(
        !mergeFlag &&
        Common::simplified(editTitle)==Common::simplified(orgSongPtr->songTitle()) &&
        editPerformerName==newSongPtr->songOriginalPerformerName()
        )
    {
        if(editTitle!=orgSongPtr->songTitle())
        {
            qDebug() << SB_DEBUG_INFO << editTitle;
            qDebug() << SB_DEBUG_INFO << orgSongPtr->songTitle();
            //	Take care of simple case changes
            newSongPtr->setSongTitle(editTitle);
            simpleTitleChangedFlag=1;
            songTitleChangedFlag=1;
        }
    }

    if(!mergeFlag && editYearOfRelease!=orgSpPtr->year())
    {
        orgSpPtr->setYear(editYearOfRelease);
    }
    if(!mergeFlag &&
        (
            editNotes!=orgSongPtr->notes() ||
            editLyrics!=orgSongPtr->lyrics())
        )
    {
        newSongPtr->setNotes(editNotes);
        newSongPtr->setLyrics(editLyrics);
    }

    if(!mergeFlag && simpleTitleChangedFlag==0 && editTitle!=orgSongPtr->songTitle())
    {
        Common::toTitleCase(editTitle);
        Common::toTitleCase(editPerformerName);
        songTitleChangedFlag=1;
    }

    qDebug() << SB_DEBUG_INFO << metaDataChangedFlag << simpleTitleChangedFlag << songTitleChangedFlag;

    if(!mergeFlag)
    {
        //	Handle performer name edits
        if(editPerformerName!=newSongPtr->songOriginalPerformerName())
        {
            qDebug() << SB_DEBUG_INFO;
            Common::sb_parameters p;
            p.performerName=editPerformerName;
            p.performerID=-1;
            Common::result result=peMgr->userMatch(p,SBIDPerformerPtr(),newPPtr);
            if(result==Common::result_canceled)
            {
                qDebug() << SB_DEBUG_INFO << "none selected -- exit from import";
                return;
            }
            if(result==Common::result_missing)
            {
            qDebug() << SB_DEBUG_INFO;
                //	Create performer
                Common::sb_parameters performer;
                performer.performerName=p.performerName;
                newPPtr=peMgr->createInDB(performer);

                //	CWIP: make sure all attributes are populated
                p.songID=newSongPtr->songID();
                p.performerID=newPPtr->performerID();
                p.year=orgSpPtr->year();
                newSpPtr=spMgr->createInDB(p);
                newSongPtr->addSongPerformance(newSpPtr);
            }
            else if(songTitleChangedFlag==0)
            {
            qDebug() << SB_DEBUG_INFO;
                //	Find out if a songperformance already exists for the unchanged title and changed performer.
                //	If it not existing, create one. Set original_performance_id to this songPerformance.
                newSpPtr=SBIDSongPerformance::retrieveSongPerformanceByPerformerID(orgSongPtr->songID(),newPPtr->performerID());
            qDebug() << SB_DEBUG_INFO;
                if(!newSpPtr)
                {
                    //	Find out if a songperformance exists for a different song with the same name.
                    newSpPtr=SBIDSongPerformance::retrieveSongPerformanceByPerformer(editTitle,editPerformerName);
            qDebug() << SB_DEBUG_INFO;
                    if(!newSpPtr)
                    {
                        //	Not found -- create one
                        p.songID=newSongPtr->songID();
                        p.performerID=newPPtr->performerID();
                        p.year=orgSpPtr->year();
                        newSpPtr=spMgr->createInDB(p);
                        newSongPtr->addSongPerformance(newSpPtr);
            qDebug() << SB_DEBUG_INFO;
                    }
                }
            }

            if(newSpPtr)
            {
                newSongPtr->setOriginalPerformanceID(newSpPtr->songPerformanceID());
            }
        }
        else
        {
            qDebug() << SB_DEBUG_INFO;
            newPPtr=SBIDPerformer::retrievePerformer(newSongPtr->songOriginalPerformerID());
        }
    }

    SB_RETURN_VOID_IF_NULL(newPPtr);
    qDebug() << SB_DEBUG_INFO << newPPtr->performerID() << newPPtr->performerName();
    qDebug() << SB_DEBUG_INFO << "orgSongPtr:sb_song_performer_id" << orgSongPtr->songOriginalPerformerID();

//    qDebug() << SB_DEBUG_INFO << "orgSongPtr:sb_song_performer_id" << orgSongPtr.songPerformerID();
//    qDebug() << SB_DEBUG_INFO << "orgSongPtr:sb_song_id" << orgSongPtr.songID();
//    qDebug() << SB_DEBUG_INFO << "orgSongPtr:songPerformerName" << orgSongPtr.songPerformerName();
//    qDebug() << SB_DEBUG_INFO << "orgSongPtr:songTitle" << orgSongPtr.songTitle();
//    qDebug() << SB_DEBUG_INFO << "newSongPtr:sb_song_performer_id" << newSongPtr.songPerformerID();
//    qDebug() << SB_DEBUG_INFO << "newSongPtr:sb_song_id" << newSongPtr.songID();
//    qDebug() << SB_DEBUG_INFO << "newSongPtr:songPerformerName" << newSongPtr.songPerformerName();
//    qDebug() << SB_DEBUG_INFO << "newSongPtr:songTitle" << newSongPtr.songTitle();

    //	Handle song title edits
qDebug() << SB_DEBUG_INFO << songTitleChangedFlag;
qDebug() << SB_DEBUG_INFO << simpleTitleChangedFlag;
qDebug() << SB_DEBUG_INFO << newSpPtr.use_count();

    if(!mergeFlag && songTitleChangedFlag==1 && simpleTitleChangedFlag==0 && !newSpPtr)
    {
qDebug() << SB_DEBUG_INFO;
        Common::sb_parameters p;
        p.songTitle=editTitle;
        p.performerID=newPPtr->performerID();
        p.performerName=newPPtr->performerName();
        p.year=orgSpPtr->year();
        p.notes=orgSpPtr->notes();

        qDebug() << SB_DEBUG_INFO << p.songID;

        Common::result result=sMgr->userMatch(p,SBIDSongPtr(),newSongPtr);
        if(result==Common::result_canceled)
        {
            qDebug() << SB_DEBUG_INFO << "none selected -- exit from import";
            return;
        }
        if(result==Common::result_missing)
        {
qDebug() << SB_DEBUG_INFO;
            //	Back to updating title and performer on original song
            newSongPtr->setSongTitle(editTitle);
            newSpPtr=orgSpPtr;
            newSpPtr->setSongPerformerID(newPPtr->performerID());
        }
        if(result==Common::result_exists)
        {
qDebug() << SB_DEBUG_INFO;
            //	Merge to selected
            sMgr->merge(orgSongPtr,newSongPtr);
            mergeFlag=1;
        }
    }
qDebug() << SB_DEBUG_INFO;

    int newSongPerformanceID=(newSpPtr?newSpPtr->songPerformanceID():-1);
    qDebug() << SB_DEBUG_INFO << newSongPerformanceID;

    //	H.	Check if there is another song with exactly the same title and performer but with different song_id.
    //		Goal is to detect another way of merging songs.
    if(!mergeFlag)
    {
        SBIDSongPerformancePtr dupSpPtr=SBIDSongPerformance::retrieveSongPerformanceByPerformer(
                    newSongPtr->songTitle(),
                    newSongPtr->songOriginalPerformerName(),
                    newSongPerformanceID);
        if(dupSpPtr)
        {
            qDebug() << SB_DEBUG_INFO << "dupSong:songPerformanceID" << dupSpPtr->songPerformanceID();
            qDebug() << SB_DEBUG_INFO << "dupSong:songID" << dupSpPtr->songID();

            qDebug() << SB_DEBUG_INFO << "merge" << orgSongPtr->key() << orgSongPtr->ID();
            qDebug() << SB_DEBUG_INFO << "to" << newSongPtr->key() << newSongPtr->ID();
            sMgr->merge(orgSongPtr,newSongPtr);
            mergeFlag=1;
            qDebug() << SB_DEBUG_INFO << "Found exact song for:" << newSongPtr->text();
        }
    }

    //	5.	Consolidate song performances with what is on any album.
    //		If a non-original performer does not exist on an album, remove this (there is no other way to remove this).
    qDebug() << SB_DEBUG_INFO << mergeFlag;
    if(!mergeFlag)
    {
        qDebug() << SB_DEBUG_INFO;
        QMapIterator<int,SBIDSongPerformancePtr> it(newSongPtr->songPerformances());
        while(it.hasNext())
        {
            it.next();
            SBIDSongPerformancePtr spPtr=it.value();
            int songPerformanceID=spPtr->songPerformanceID();	//	CWIP: new data collection to keep track of this by songPerformanceID

            qDebug() << SB_DEBUG_INFO << songPerformanceID;
            QVectorIterator<SBIDAlbumPerformancePtr> apIT(newSongPtr->allPerformances());
            bool foundFlag=0;
            while(apIT.hasNext() && !foundFlag)
            {
                SBIDAlbumPerformancePtr apPtr=apIT.next();
                if(apPtr->songPerformanceID()==songPerformanceID)
                {
                    foundFlag=1;
                }
            }

            if(!foundFlag && editPerformerName!=spPtr->songPerformerName())
            {
                qDebug() << SB_DEBUG_INFO << "Removing song performance by " << spPtr->songPerformerName() << "id=" << spPtr->songPerformanceID();
                newSongPtr->removeSongPerformance(spPtr);
            }
        }
    }
    */

    cm->debugShowChanges("before save");
    const bool successFlag=cm->saveChanges();

    qDebug() << SB_DEBUG_INFO << mergedFlag;
    if(successFlag==1)
    {
        QString updateText=QString("Saved song %1%2%3.")
            .arg(QChar(96))      //	1
            .arg(newSongPtr->songTitle())   //	2
            .arg(QChar(180));    //	3

        //	Update screenstack
        currentScreenItem.setEditFlag(0);
        Context::instance()->controller()->updateStatusBarText(updateText);

        if(mergedFlag)
        {
            //	Refresh models -- since song got removed.

            ScreenStack* st=Context::instance()->screenStack();

            newSongPtr->refreshDependents(0,1);
            ScreenItem from(orgSongPtr->key());
            ScreenItem to(newSongPtr->key());
            qDebug() << SB_DEBUG_INFO << orgSongPtr->key() << newSongPtr->key();
            st->replace(from,to);
        }

        if(mergedFlag || songTitleChangedFlag)
        {
            mw->ui.tabAllSongs->preload();
        }
    }
    else
    {
        qDebug() << SB_DEBUG_INFO;
        dal->restore(restorePoint);
    }

    {
            ScreenStack* st=Context::instance()->screenStack();
            st->debugShow("end of save");
    }

    //	Close screen
    Context::instance()->navigator()->closeCurrentTab(1);
}

void
SBTabSongEdit::_init()
{
    if(_initDoneFlag==0)
    {
        _initDoneFlag=1;

        const MainWindow* mw=Context::instance()->mainWindow();

        //	Completers
        CompleterFactory* cf=Context::instance()->completerFactory();
        mw->ui.songEditPerformerName->setCompleter(cf->getCompleterPerformer());

        connect(mw->ui.pbSongEditSave, SIGNAL(clicked(bool)),
                this, SLOT(save()));
        connect(mw->ui.pbSongEditCancel, SIGNAL(clicked(bool)),
                Context::instance()->navigator(), SLOT(closeCurrentTab()));
    }
}

ScreenItem
SBTabSongEdit::_populate(const ScreenItem& si)
{
    _init();
    const MainWindow* mw=Context::instance()->mainWindow();
    SBIDSongPtr songPtr=SBIDSong::retrieveSong(si.key());
    SB_RETURN_IF_NULL(songPtr,ScreenItem());

    ScreenItem currentScreenItem=si;
    currentScreenItem.setEditFlag(1);
    //SBTab::_setCurrentScreenItem(currentScreenItem);

    mw->ui.songEditTitle->setText(songPtr->songTitle());
    mw->ui.songEditPerformerName->setText(songPtr->songOriginalPerformerName());
    mw->ui.songEditYearOfRelease->setText(QString("%1").arg(songPtr->songOriginalYear()));
    mw->ui.songEditNotes->setText(songPtr->notes());
    mw->ui.songEditLyrics->setText(songPtr->lyrics());

    //	Disable tmpButtons
    mw->ui.pbNA2->hide();

    //	Set correct focus
    mw->ui.songEditTitle->selectAll();
    mw->ui.songEditTitle->setFocus();

    mw->ui.tabSongEditLists->setCurrentIndex(0);

    return currentScreenItem;
}
