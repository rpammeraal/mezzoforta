#include <QMessageBox>

#include "SBTabSongEdit.h"

#include "CacheManager.h"
#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "SBDialogSelectItem.h"
#include "SBIDPerformer.h"
#include "SBSqlQueryModel.h"

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
    const SBIDPtr& ptr=this->currentScreenItem().ptr();
    const MainWindow* mw=Context::instance()->getMainWindow();

    if(ptr->itemType()==SBIDBase::sb_type_song)
    {
        SBIDSongPtr songPtr=std::dynamic_pointer_cast<SBIDSong>(ptr);
        if(songPtr->songTitle()!=mw->ui.songEditTitle->text() ||
            songPtr->songOriginalPerformerName()!=mw->ui.songEditPerformerName->text() ||
            songPtr->songOriginalYear()!=mw->ui.songEditYearOfRelease->text().toInt() ||
            songPtr->notes()!=mw->ui.songEditNotes->text() ||
            songPtr->lyrics()!=mw->ui.songEditLyrics->toPlainText()
        )
        {
            return 1;
        }
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
    //	A4.	[merge song (within performer)] Badaa - U22 to Acrobat. Note that album listing for Acrobat should include 'Bad' albums.
    //	A5.	[remove performances without album performances] Edit (no changes) and save Acrobat - U2 one more time to remove the U22 entry.

    //	B.	[switch original performer to non-original performer] Dancing Barefoot: change from Patti Smith to U2 and back

    //	C.	[switch original performer to different existing performer] "C" Moon Cry Like A Baby: Simple Minds -> U2

    //	D.	[Assign song to different song and its non-original performer]  Assign <whatever> to Sunday Bloody Sunday by The Royal Philharmonic Orchestra

    //	Refresh database
    //	F2.	[merge song (within performer)] Bad - U2 to Acrobat. Note that album listing for Acrobat should include 'Bad' albums.

    //	G.	[merge to different performer] "C" Moon Cry Like A Baby/Simple Minds -> "40"/U2

    //	H.	[merge song with existing song by renaming original performer] Get Lucky - Daft Poonk => Get Lucky - Daft Poonk & Squirrel W.
    //	I.	[simple edits]: 'Elusive Dreams ' -> 'Elusive Dreams': should just save the new title.

    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QString restorePoint=dal->createRestorePoint();
    const MainWindow* mw=Context::instance()->getMainWindow();
    ScreenItem currentScreenItem=this->currentScreenItem();
    SBIDSongPtr orgSongPtr=SBIDSong::retrieveSong(this->currentScreenItem().ptr()->itemID());
    SBIDSongPtr newSongPtr=orgSongPtr;
    SBIDSongPerformancePtr orgSpPtr=orgSongPtr->originalSongPerformancePtr();
    SBIDSongPerformancePtr newSpPtr;	//	only populated if a new original spPtr is created.
    CacheManager* cm=Context::instance()->cacheManager();
    CacheSongMgr* sMgr=cm->songMgr();
    CacheSongPerformanceMgr* spMgr=cm->songPerformanceMgr();
    CachePerformerMgr* peMgr=cm->performerMgr();
    bool mergeFlag=0;
    int orgSongPerformanceID=orgSongPtr->originalSongPerformanceID();

    qDebug() << SB_DEBUG_INFO << "orgSong" << orgSongPtr->text();
    qDebug() << SB_DEBUG_INFO << "newSong" << newSongPtr->text();

    if(!orgSongPtr)
    {
        qDebug() << SB_DEBUG_ERROR <<  "SBTabSongEdit::save:current song undefined. Aborting.";
        return;
    }

    if(currentScreenItem.editFlag()==0)
    {
        qDebug() << SB_DEBUG_ERROR << "isEditFlag flag not set. Aborting.";
        return;
    }

    QString editTitle=mw->ui.songEditTitle->text();
    QString editPerformerName=mw->ui.songEditPerformerName->text();
    int editYearOfRelease=mw->ui.songEditYearOfRelease->text().toInt();
    QString editNotes=mw->ui.songEditNotes->text();
    QString editLyrics=mw->ui.songEditLyrics->toPlainText();
    bool metaDataChangedFlag=0;
    bool simpleTitleChangedFlag=0;
    bool songTitleChangedFlag=0;

    qDebug() << SB_DEBUG_INFO << editTitle << editPerformerName;
    qDebug() << SB_DEBUG_INFO << orgSongPtr->songTitle() << newSongPtr->songOriginalPerformerName();
    qDebug() << SB_DEBUG_INFO << newSongPtr->songID();
    qDebug() << SB_DEBUG_INFO << "orgSongPtr:sb_song_performer_id" << orgSongPtr->originalSongPerformanceID();
    qDebug() << SB_DEBUG_INFO << "orgSongPtr:sb_song_id" << orgSongPtr->songID();
    qDebug() << SB_DEBUG_INFO << "newSongPtr:sb_song_performer_id" << newSongPtr->originalSongPerformanceID();
    qDebug() << SB_DEBUG_INFO << "newSongPtr:sb_song_id" << newSongPtr->songID();

    ///	Take care of meta data changes and simple title changes.
    if(
            editYearOfRelease!=orgSpPtr->year() ||
            editNotes!=orgSongPtr->notes() ||
            editLyrics!=orgSongPtr->lyrics()

        )
    {
        metaDataChangedFlag=1;
    }

    if(
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
            sMgr->setChanged(newSongPtr);
            simpleTitleChangedFlag=1;
            songTitleChangedFlag=1;
        }
    }

    if(editYearOfRelease!=orgSpPtr->year())
    {
        orgSpPtr->setYear(editYearOfRelease);
        spMgr->setChanged(orgSpPtr);
    }
    if(editNotes!=orgSongPtr->notes() ||
        editLyrics!=orgSongPtr->lyrics())
    {
        newSongPtr->setNotes(editNotes);
        newSongPtr->setLyrics(editLyrics);
        sMgr->setChanged(newSongPtr);
    }

    if(simpleTitleChangedFlag==0 && editTitle!=orgSongPtr->songTitle())
    {
        Common::toTitleCase(editTitle);
        Common::toTitleCase(editPerformerName);
        songTitleChangedFlag=1;
    }

    qDebug() << SB_DEBUG_INFO << metaDataChangedFlag << simpleTitleChangedFlag << songTitleChangedFlag;

    //	Handle performer name edits
    SBIDPerformerPtr pPtr;
    if(editPerformerName!=newSongPtr->songOriginalPerformerName())
    {
        qDebug() << SB_DEBUG_INFO;
        Common::sb_parameters p;
        p.performerName=editPerformerName;
        p.performerID=-1;
        Common::result result=peMgr->userMatch(p,SBIDPerformerPtr(),pPtr);
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
            pPtr=peMgr->createInDB(performer);

            //	CWIP: make sure all attributes are populated
            p.songID=newSongPtr->songID();
            p.performerID=pPtr->performerID();
            p.year=orgSpPtr->year();
            newSpPtr=spMgr->createInDB(p);
            newSongPtr->addSongPerformance(newSpPtr);
            sMgr->setChanged(newSongPtr);
        }
        else if(songTitleChangedFlag==0)
        {
        qDebug() << SB_DEBUG_INFO;
            //	Find out if a songperformance already exists for the unchanged title and changed performer.
            //	If it not existing, create one. Set original_performance_id to this songPerformance.
            newSpPtr=SBIDSongPerformance::retrieveSongPerformanceByPerformerID(orgSongPtr->songID(),pPtr->performerID());
            if(!newSpPtr)
            {
        qDebug() << SB_DEBUG_INFO;
                //	Not found -- create one
                p.songID=newSongPtr->songID();
                p.performerID=pPtr->performerID();
                p.year=orgSpPtr->year();
                newSpPtr=spMgr->createInDB(p);
                newSongPtr->addSongPerformance(newSpPtr);
                sMgr->setChanged(newSongPtr);
            }
        }

        if(newSpPtr)
        {
            newSongPtr->setOriginalPerformanceID(newSpPtr->songPerformanceID());
            sMgr->setChanged(newSongPtr);
        }
    }
    else
    {
        qDebug() << SB_DEBUG_INFO;
        pPtr=SBIDPerformer::retrievePerformer(newSongPtr->songOriginalPerformerID());
    }

    //	At this point pPtr holds the performer data for the changed song.
    SB_RETURN_VOID_IF_NULL(pPtr);
    qDebug() << SB_DEBUG_INFO << pPtr->performerID() << pPtr->performerName();
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
    if(songTitleChangedFlag==1 && simpleTitleChangedFlag==0 && !newSpPtr)
    {
qDebug() << SB_DEBUG_INFO;
        Common::sb_parameters p;
        p.songTitle=editTitle;
        p.performerID=pPtr->performerID();
        p.performerName=pPtr->performerName();
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
            newSpPtr->setSongPerformerID(pPtr->performerID());
            sMgr->setChanged(newSongPtr);
            spMgr->setChanged(newSpPtr);
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

    //	H.	Check if there is another song with exactly the same title and performer but with different song_id.
    //		Goal is to detect another way of merging songs.
    SBIDSongPerformancePtr dupSpPtr=SBIDSongPerformance::retrieveSongPerformanceByPerformer(newSongPtr->songTitle(),newSongPtr->songOriginalPerformerName());
    if(dupSpPtr)
    {
        sMgr->merge(orgSongPtr,newSongPtr);
        mergeFlag=1;
        qDebug() << SB_DEBUG_INFO << "Found exact song for:" << newSongPtr->text();
    }

    //	I.	Consolidate song performances with what is on any album.
    //		If a non-original performer does not exist on an album, remove this (there is no other way to remove this).
    qDebug() << SB_DEBUG_INFO << orgSongPerformanceID << newSongPtr->originalSongPerformanceID();
    qDebug() << SB_DEBUG_INFO << editPerformerName << newSongPtr->originalSongPerformancePtr()->songPerformerName();
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

    cm->debugShowChanges("before save");
    const bool successFlag=cm->saveChanges();

    if(successFlag==1)
    {
        QString updateText=QString("Saved song %1%2%3.")
            .arg(QChar(96))      //	1
            .arg(newSongPtr->songTitle())   //	2
            .arg(QChar(180));    //	3

        //	Update screenstack
        currentScreenItem.setEditFlag(0);
        Context::instance()->getController()->updateStatusBarText(updateText);

        if(mergeFlag)
        {
            //	Refresh models -- since song got removed.

            ScreenStack* st=Context::instance()->getScreenStack();

            newSongPtr->refreshDependents(0,1);
            ScreenItem from(orgSongPtr);
            ScreenItem to(newSongPtr);
            st->replace(from,to);
        }

        if(mergeFlag || songTitleChangedFlag)
        {
            mw->ui.tabAllSongs->preload();
        }
    }
    else
    {
        qDebug() << SB_DEBUG_INFO;
        dal->restore(restorePoint);
    }

    //	Close screen
    Context::instance()->getNavigator()->closeCurrentTab(1);
}

void
SBTabSongEdit::_init()
{
    if(_initDoneFlag==0)
    {
        _initDoneFlag=1;

        const MainWindow* mw=Context::instance()->getMainWindow();

        //	Completers
        CompleterFactory* cf=Context::instance()->completerFactory();
        mw->ui.songEditPerformerName->setCompleter(cf->getCompleterPerformer());

        connect(mw->ui.pbSongEditSave, SIGNAL(clicked(bool)),
                this, SLOT(save()));
        connect(mw->ui.pbSongEditCancel, SIGNAL(clicked(bool)),
                Context::instance()->getNavigator(), SLOT(closeCurrentTab()));
    }
}

ScreenItem
SBTabSongEdit::_populate(const ScreenItem& si)
{
    _init();
    const MainWindow* mw=Context::instance()->getMainWindow();
    SBIDSongPtr songPtr;

    //	Get detail
    if(si.ptr())
    {
        qDebug() << SB_DEBUG_INFO;
        songPtr=SBIDSong::retrieveSong(si.ptr()->itemID());
    }

    if(!songPtr)
    {
        return ScreenItem();
    }

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
