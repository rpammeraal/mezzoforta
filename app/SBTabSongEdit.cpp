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
    //	3.	find same song performance based on edit data 'as is' (altSpPtr).
    //		If found:
    //			-	if songID is same, set original performance ID
    //			-	otherwise: merge -> save. [merge duplicate]
    //	4.	if performer has changed:
    //		4.1	lookup changed performer (userMatch)
    //			4.1.1	if not found:
    //				-	create new performer,
    //				-	create new song performance,
    //				-	set originalPerformanceID of song
    //				-	set other attributes, save song -- completely new performer means no other matches can be found.
    //			4.1.2	if found:
    //				-	find if song performance exist with edited song title & changed performer.
    //					-	If found:
    //						-	if song title has not changed, set new original performance
    //						-	otherwise: merge -> save.
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
    QString editTitle=mw->ui.songEditTitle->text().simplified();
    QString editPerformerName=mw->ui.songEditPerformerName->text().simplified();
    int editYearOfRelease=mw->ui.songEditYearOfRelease->text().toInt();
    QString editNotes=mw->ui.songEditNotes->text();
    QString editLyrics=mw->ui.songEditLyrics->toPlainText();

    //	Flags
    bool metaDataChangedFlag=0;
    bool songTitleChangedFlag=0;
    bool mergedFlag=0;
    bool setMetaDataFlag=0;	//	if set, update meta data

    //	1.	Check if song title only has changed.
    qDebug() << SB_DEBUG_INFO << Common::comparable(editTitle);
    qDebug() << SB_DEBUG_INFO << Common::comparable(orgSongPtr->songTitle());
    if(editPerformerName==orgSongPtr->songOriginalPerformerName() && Common::comparable(editTitle)==Common::comparable(orgSongPtr->songTitle()))
    {
        qDebug() << SB_DEBUG_INFO;
        songTitleChangedFlag=1;
    }

    //	2.	Determine metaDataChangedFlag
    if(editYearOfRelease!=orgSpPtr->year() || editNotes!=orgSongPtr->notes() || editLyrics!=orgSongPtr->lyrics())
    {
        metaDataChangedFlag=1;
        setMetaDataFlag=1;
    }

    //	3.	Look up if song already exists.
    qDebug() << SB_DEBUG_INFO;
    //	if(songTitleChangedFlag==0)
    {
        qDebug() << SB_DEBUG_INFO;
        //	Keep the scope of altSpPtr local
        SBIDSongPerformancePtr altSpPtr=SBIDSongPerformance::retrieveSongPerformanceByPerformer(
                    editTitle,
                    editPerformerName,
                    orgSpPtr->songPerformanceID());

        if(altSpPtr)
        {
            qDebug() << SB_DEBUG_INFO;
            if(altSpPtr->songID()==orgSongPtr->songID())
            {
            qDebug() << SB_DEBUG_INFO;
                newSongPtr=orgSongPtr;
                newSongPtr->setOriginalPerformanceID(altSpPtr->songPerformanceID());
            }
            else
            {
                newSongPtr=altSpPtr->songPtr();
                SB_RETURN_VOID_IF_NULL(newSongPtr);
                qDebug() << SB_DEBUG_INFO << "set merged flag";
                mergedFlag=1;
                setMetaDataFlag=0;
            }
        }
    }

    //	4.	if performer has changed
    if(!mergedFlag)
    {
        qDebug() << SB_DEBUG_INFO;
        if(editPerformerName!=orgSpPtr->songPerformerName())
        {
            qDebug() << SB_DEBUG_INFO;
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
            else if(result==Common::result_exists_derived || result==Common::result_exists_user_selected)
            {
                //	4.1.2	if found:
                //		find if song performance exist with edited song title & changed performer.
                SBIDSongPerformancePtr altSpPtr=SBIDSongPerformance::retrieveSongPerformanceByPerformer(
                            editTitle,
                            newPPtr->performerName(),
                            orgSpPtr->songPerformanceID());

                if(altSpPtr)
                {
                    qDebug() << SB_DEBUG_INFO << altSpPtr->songID();
                    qDebug() << SB_DEBUG_INFO << orgSongPtr->songID();
                    if(altSpPtr->songID()==orgSongPtr->songID())
                    {
                        qDebug() << SB_DEBUG_INFO << orgSongPtr->originalSongPerformanceID();
                        newSongPtr=orgSongPtr;
                        newSongPtr->setOriginalPerformanceID(altSpPtr->songPerformanceID());
                        qDebug() << SB_DEBUG_INFO << orgSongPtr->originalSongPerformanceID();
                    }
                    else
                    {
                        //		If found, merge -> save.
                        newSongPtr=altSpPtr->songPtr();
                        SB_RETURN_VOID_IF_NULL(newSongPtr);
                        qDebug() << SB_DEBUG_INFO << "set merged flag";
                        mergedFlag=1;
                    }
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
                    qDebug() << SB_DEBUG_INFO << p.songID;
                    if(result==Common::result_canceled)
                    {
                        qDebug() << SB_DEBUG_INFO << "none selected -- exit from import";
                        return;
                    }
                    qDebug() << SB_DEBUG_INFO;
                    if(result==Common::result_exists_derived || result==Common::result_exists_user_selected)
                    {
                        //		if found: merge
                        SB_RETURN_VOID_IF_NULL(newSongPtr);
                        qDebug() << SB_DEBUG_INFO << "set merged flag";
                        mergedFlag=1;
                    }
                    qDebug() << SB_DEBUG_INFO;
                    if(result==Common::result_missing)
                    {
                        //		if not found:
                        if(!newSongPtr)
                        {
                            newSongPtr=orgSongPtr;
                        }
                        //	Create new song performance
                        p.songID=orgSongPtr->songID();
                        p.performerID=newPPtr->performerID();
                        p.year=editYearOfRelease;
                        SBIDSongPerformancePtr newSpPtr=spMgr->createInDB(p);
                    qDebug() << SB_DEBUG_INFO;
                        if(!newSpPtr)
                        {
                            qDebug() << SB_DEBUG_INFO;
                        }
                        if(!newSongPtr)
                        {
                            qDebug() << SB_DEBUG_INFO;
                        }
                    qDebug() << SB_DEBUG_INFO << newSpPtr->key() << newSpPtr->ID();
                        newSongPtr->addSongPerformance(newSpPtr);
                    qDebug() << SB_DEBUG_INFO;

                        //	set originalPerformanceID of song
                    qDebug() << SB_DEBUG_INFO << newSongPtr->originalSongPerformanceID();
                        newSongPtr->setOriginalPerformanceID(newSpPtr->songPerformanceID());
                    qDebug() << SB_DEBUG_INFO;

                        //	set other attributes, save song -- completely new performer means no other matches can be found.
                        setMetaDataFlag=1;
                    }
                    qDebug() << SB_DEBUG_INFO;
                }
            }
        }
        else
        {
            qDebug() << SB_DEBUG_INFO;
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
            else if(result==Common::result_exists_derived || result==Common::result_exists_user_selected)
            {
                SB_RETURN_VOID_IF_NULL(newSongPtr);
                if(orgSongPtr->songID()!=newSongPtr->songID())
                {
                    qDebug() << SB_DEBUG_INFO << "set merged flag";
                    mergedFlag=1;
                }
            }
            else if(result==Common::result_missing)
            {
                newSongPtr=orgSongPtr;
                newSongPtr->setSongTitle(editTitle);
                setMetaDataFlag=1;
                songTitleChangedFlag=0;
            }
        }
    }

    if(mergedFlag)
    {
        qDebug() << SB_DEBUG_INFO << "merge" << orgSongPtr->key() << orgSongPtr->ID();
        qDebug() << SB_DEBUG_INFO << "to" << newSongPtr->key() << newSongPtr->ID();
        sMgr->merge(orgSongPtr,newSongPtr);
        setMetaDataFlag=0;
    }

    if(setMetaDataFlag && metaDataChangedFlag)
    {
        SB_RETURN_VOID_IF_NULL(newSongPtr);
        newSongPtr->setNotes(editNotes);
        newSongPtr->setLyrics(editLyrics);
        SBIDSongPerformancePtr spPtr=newSongPtr->originalSongPerformancePtr();
        if(spPtr)
        {
            spPtr->setYear(editYearOfRelease);
        }
    }

    //	5.	Consolidate song performances with what is on any album.
    //		If a non-original performer does not exist on an album, remove this (there is no other way to remove this).
    qDebug() << SB_DEBUG_INFO << "merged=" <<  mergedFlag;
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
            qDebug() << SB_DEBUG_INFO;
            bool foundFlag=0;
            qDebug() << SB_DEBUG_INFO;
            while(apIT.hasNext() && !foundFlag)
            {
                SBIDAlbumPerformancePtr apPtr=apIT.next();
                qDebug() << SB_DEBUG_INFO << apPtr->genericDescription();
                if(apPtr->songPerformanceID()==songPerformanceID)
                {
                    foundFlag=1;
                }
            }

            qDebug() << SB_DEBUG_INFO << songPerformanceID;
            if(!foundFlag && editPerformerName!=spPtr->songPerformerName())
            {
                qDebug() << SB_DEBUG_INFO << "Removing song performance by " << spPtr->songPerformerName() << "id=" << spPtr->songPerformanceID();
                newSongPtr->removeSongPerformance(spPtr);
            }
        }
    }

    if(songTitleChangedFlag==1)
    {
        qDebug() << SB_DEBUG_INFO;
        if(!newSongPtr)
        {
            newSongPtr=orgSongPtr;
        }
        if(newSongPtr->key()==orgSongPtr->key() && songTitleChangedFlag==1)
        {
            qDebug() << SB_DEBUG_INFO;
            orgSongPtr->setSongTitle(editTitle);
        }
    }

    cm->debugShowChanges("before save");
    const bool successFlag=cm->saveChanges("Saving Song");

    qDebug() << SB_DEBUG_INFO << mergedFlag;
    if(successFlag)
    {
        qDebug() << SB_DEBUG_INFO;
        ProgressDialog::instance()->startDialog(__SB_PRETTY_FUNCTION__,"Refreshing Data",1);
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step:refresh",1,5);

        //	Update screenstack, display notice, etc.
        QString updateText=QString("Saved song %1%2%3.")
            .arg(QChar(96))      //	1
            .arg(newSongPtr->songTitle())   //	2
            .arg(QChar(180));    //	3

        //	Update screenstack
        qDebug() << SB_DEBUG_INFO;
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step:refresh",2,5);
        currentScreenItem.setEditFlag(0);
        Context::instance()->controller()->updateStatusBarText(updateText);

        qDebug() << SB_DEBUG_INFO;
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step:refresh",3,5);
        if(mergedFlag)
        {
            //	Refresh models -- since song got removed.

            ScreenStack* st=Context::instance()->screenStack();

            newSongPtr->refreshDependents(1);
            ScreenItem from(orgSongPtr->key());
            ScreenItem to(newSongPtr->key());
            qDebug() << SB_DEBUG_INFO << orgSongPtr->key() << newSongPtr->key();
            st->replace(from,to);
        }

        qDebug() << SB_DEBUG_INFO;
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step:refresh",4,5);
        if(mergedFlag || songTitleChangedFlag)
        {
            mw->ui.tabAllSongs->preload();
        }

        qDebug() << SB_DEBUG_INFO;
        ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,"step:refresh");
        ProgressDialog::instance()->finishDialog(__SB_PRETTY_FUNCTION__);
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
    qDebug() << SB_DEBUG_INFO << si.key();
    const MainWindow* mw=Context::instance()->mainWindow();
    SBIDSongPtr sPtr=SBIDSong::retrieveSong(si.key());
    SB_RETURN_IF_NULL(sPtr,ScreenItem());

    if(si.key()!=sPtr->key())
    {
        //	Update screenstack with correct song key
        ScreenStack* st=Context::instance()->screenStack();

        st->replace(ScreenItem(si.key()),ScreenItem(sPtr->key()));
    }

    //	Refresh completers
    CompleterFactory* cf=Context::instance()->completerFactory();
    mw->ui.songEditPerformerName->setCompleter(cf->getCompleterPerformer());

    ScreenItem currentScreenItem=si;
    currentScreenItem.setEditFlag(1);
    currentScreenItem.updateSBIDBase(sPtr->key());
    //SBTab::_setCurrentScreenItem(currentScreenItem);

    mw->ui.songEditTitle->setText(sPtr->songTitle());
    mw->ui.songEditPerformerName->setText(sPtr->songOriginalPerformerName());
    mw->ui.songEditYearOfRelease->setText(QString("%1").arg(sPtr->songOriginalYear()));
    mw->ui.songEditNotes->setText(sPtr->notes());
    mw->ui.songEditLyrics->setText(sPtr->lyrics());

    //	Disable tmpButtons
    mw->ui.pbNA2->hide();

    //	Set correct focus
    mw->ui.songEditTitle->selectAll();
    mw->ui.songEditTitle->setFocus();

    mw->ui.tabSongEditLists->setCurrentIndex(0);

    return currentScreenItem;
}
