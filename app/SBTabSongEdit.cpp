#include <QMessageBox>

#include "SBTabSongEdit.h"

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

    if(ptr->itemType()!=SBIDBase::sb_type_invalid)
    {
        if(ptr->songTitle()!=mw->ui.songEditTitle->text() ||
            ptr->songPerformerName()!=mw->ui.songEditPerformerName->text() ||
            ptr->year()!=mw->ui.songEditYearOfRelease->text().toInt() ||
            ptr->notes()!=mw->ui.songEditNotes->text() ||
            ptr->lyrics()!=mw->ui.songEditLyrics->toPlainText()
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
    //	A.	[simple rename] Bad - U2: change to Badaa. Should be simple renames.
    //	B.	[simple rename w/ case] Badaa - U2 to BadaA. Should take into account case change.
    //	C.	[switch original performer to non-original performer] Dancing Barefoot: change from Patti Smith to U2 and back
    //	D.	[switch original performer to completely different performer] "C" Moon Cry Like A Baby: Simple Minds -> U2

    //	E.	[switch original performer to completely new performer] Bad - U2: change performer to U22.
    //	F1.	[merge song (within performer)] Badaa - U22 to Acrobat. Note that album listing for Acrobat should include 'Bad' albums.

    //	Refresh database
    //	F2.	[merge song (within performer)] Bad - U2 to Acrobat. Note that album listing for Acrobat should include 'Bad' albums.

    //	G.	[merge to different performer] "C" Moon Cry Like A Baby/Simple Minds -> "40"/U2

    //	H.	[merge song with existing song by renaming original performer] Get Lucky - Daft Poonk => Get Lucky - Daft Poonk & Squirrel W.
    //	I.	[simple edits]: 'Elusive Dreams ' -> 'Elusive Dreams': should just save the new title.

    const MainWindow* mw=Context::instance()->getMainWindow();
    ScreenItem currentScreenItem=this->currentScreenItem();
    SBIDSong orgSongID=static_cast<SBIDSong>(*(currentScreenItem.ptr()));
    SBIDSong newSongID=orgSongID;

    qDebug() << SB_DEBUG_INFO << "orgSong" << orgSongID;
    qDebug() << SB_DEBUG_INFO << "newSong" << newSongID;

    if(orgSongID.songID()==-1 || newSongID.songID()==-1)
    {
        QMessageBox msgBox;
        msgBox.setText("SBTabSongEdit::save:old or new song undefined");
        msgBox.exec();
        return;
    }

    if(currentScreenItem.editFlag()==0)
    {
        qDebug() << SB_DEBUG_WARNING << "isEditFlag flag not set -- not continuing";
        return;
    }

    QString editTitle=mw->ui.songEditTitle->text();
    QString editPerformerName=mw->ui.songEditPerformerName->text();
    int editYearOfRelease=mw->ui.songEditYearOfRelease->text().toInt();
    QString editNotes=mw->ui.songEditNotes->text();
    QString editLyrics=mw->ui.songEditLyrics->toPlainText();
    bool hasCaseChange=0;

    qDebug() << SB_DEBUG_INFO << editTitle << editPerformerName;
    qDebug() << SB_DEBUG_INFO << orgSongID.songTitle() << newSongID.songPerformerName();
    qDebug() << SB_DEBUG_INFO << newSongID;
    qDebug() << SB_DEBUG_INFO << "orgSongID:sb_song_performer_id" << orgSongID.songPerformerID();
    qDebug() << SB_DEBUG_INFO << "orgSongID:sb_song_id" << orgSongID.songID();
    qDebug() << SB_DEBUG_INFO << "newSongID:sb_song_performer_id" << newSongID.songPerformerID();
    qDebug() << SB_DEBUG_INFO << "newSongID:sb_song_id" << newSongID.songID();

    //	If only case is different in songTitle, save the new title as is.
    if((Common::simplified(editTitle)==Common::simplified(newSongID.songTitle())) &&
        (editTitle!=newSongID.songTitle()) &&
        (editPerformerName==newSongID.songPerformerName()))
    {
        qDebug() << SB_DEBUG_INFO;
        newSongID.setSongID(-1);
        newSongID.setSongTitle(editTitle);
        hasCaseChange=1;	//	Identify to saveSong that title has changed.
    }
    else
    {
        qDebug() << SB_DEBUG_INFO;
        Common::toTitleCase(editTitle);
        Common::toTitleCase(editPerformerName);
        hasCaseChange=0;
    }
    qDebug() << SB_DEBUG_INFO << hasCaseChange;
    qDebug() << SB_DEBUG_INFO << "orgSongID:sb_song_performer_id" << orgSongID.songPerformerID();
    qDebug() << SB_DEBUG_INFO << "newSongID:sb_song_performer_id" << newSongID.songPerformerID();

    //	Handle performer name edits
    if(hasCaseChange==0 && editPerformerName!=newSongID.songPerformerName())
    {
        SBIDPerformer selectedPerformer=newSongID;
        if(SBIDPerformer::selectSavePerformer(editPerformerName,selectedPerformer,selectedPerformer,mw->ui.songEditPerformerName)==0)
        {
            return;
        }
        newSongID.setSongPerformerID(selectedPerformer.songPerformerID());
        newSongID.setSongPerformerName(selectedPerformer.songPerformerName());
    }

    qDebug() << SB_DEBUG_INFO << "orgSongID:sb_song_performer_id" << orgSongID.songPerformerID();
    qDebug() << SB_DEBUG_INFO << "orgSongID:sb_song_id" << orgSongID.songID();
    qDebug() << SB_DEBUG_INFO << "orgSongID:songPerformerName" << orgSongID.songPerformerName();
    qDebug() << SB_DEBUG_INFO << "orgSongID:songTitle" << orgSongID.songTitle();
    qDebug() << SB_DEBUG_INFO << "newSongID:sb_song_performer_id" << newSongID.songPerformerID();
    qDebug() << SB_DEBUG_INFO << "newSongID:sb_song_id" << newSongID.songID();
    qDebug() << SB_DEBUG_INFO << "newSongID:songPerformerName" << newSongID.songPerformerName();
    qDebug() << SB_DEBUG_INFO << "newSongID:songTitle" << newSongID.songTitle();

    //	Handle song title edits
    if(editTitle.toLower()!=newSongID.songTitle().toLower() &&
        hasCaseChange==0)
    {
        SBIDSong selectedSongID(-1);
        selectedSongID.setSongTitle(editTitle);
        qDebug() << SB_DEBUG_INFO << selectedSongID;

        qDebug() << SB_DEBUG_INFO << editTitle << "!=" << newSongID.songTitle() << newSongID.songID();
        SBSqlQueryModel* songMatches=newSongID.matchSongWithinPerformer(editTitle);

        qDebug() << SB_DEBUG_INFO << songMatches->rowCount();

        if(songMatches->rowCount()>1)
        {

            if(songMatches->rowCount()>=2 &&
                songMatches->record(1).value(0).toInt()==1
            )
            {
                selectedSongID.setSongID(songMatches->record(1).value(1).toInt());
                selectedSongID.setSongTitle(songMatches->record(1).value(2).toString());
                qDebug() << SB_DEBUG_INFO << "selectedSongID.sb_song_id" << selectedSongID.songID();
            }
            else
            {
                SBDialogSelectItem* pu=SBDialogSelectItem::selectSongByPerformer(std::make_shared<SBIDSong>(selectedSongID),songMatches);
                pu->exec();

                SBIDPtr selected=pu->getSelected();
                selectedSongID=SBIDSong(*selected);

                //	Go back to screen if no item has been selected
                if(pu->hasSelectedItem()==0)
                {
                    return;
                }
                qDebug() << SB_DEBUG_INFO << "selectedSongID.sb_song_id" << selectedSongID.songID();
            }

            //	Update field
            mw->ui.songEditTitle->setText(selectedSongID.songTitle());
        }

        //	Update newSongID
        newSongID.setSongID(selectedSongID.songID());
        newSongID.setSongTitle(selectedSongID.songTitle());
        qDebug() << SB_DEBUG_INFO << "selected song" << newSongID.songID() << newSongID.songTitle();
    }
    newSongID.setYear(editYearOfRelease);
    newSongID.setNotes(editNotes);
    newSongID.setLyrics(editLyrics);

    //	H.	Check if there is another song with exactly the same title and performer but with different song_id.
    //		Goal is to detect another way of merging songs.
    SBSqlQueryModel* existingSongs=newSongID.findSong();
    if(existingSongs!=0 && existingSongs->rowCount()==1)
    {
        qDebug() << SB_DEBUG_INFO << "Found exact song for:" << newSongID;
        newSongID.setSongID(existingSongs->record(0).value(0).toInt());
        newSongID.setNotes(existingSongs->record(0).value(2).toString());
        newSongID.setSongPerformerID(existingSongs->record(0).value(3).toInt());
        newSongID.setYear(existingSongs->record(0).value(5).toInt());
        newSongID.setLyrics(existingSongs->record(0).value(6).toString());

        qDebug() << SB_DEBUG_INFO << "Changed to:" << newSongID;
    }

    qDebug() << SB_DEBUG_INFO << "orgSongID:sb_song_id" << orgSongID.songID();
    qDebug() << SB_DEBUG_INFO << "newSongID:sb_song_id" << newSongID.songID();

    if(orgSongID!=newSongID ||
        orgSongID.year()!=newSongID.year() ||
        orgSongID.notes()!=newSongID.notes() ||
        orgSongID.lyrics()!=newSongID.lyrics() ||
        hasCaseChange==1)
    {

        const bool successFlag=SBIDSong::updateExistingSong(orgSongID,newSongID,QStringList(),1);

        if(successFlag==1)
        {
            QString updateText=QString("Saved song %1%2%3.")
                .arg(QChar(96))      //	1
                .arg(newSongID.songTitle())   //	2
                .arg(QChar(180));    //	3
            Context::instance()->getController()->updateStatusBarText(updateText);

            if(orgSongID!=newSongID)
            {
                //	Update models!
                Context::instance()->getController()->refreshModels();

                if(orgSongID.songID()!=newSongID.songID())
                {
                    //	Remove old from screenstack
                    Context::instance()->getScreenStack()->removeScreen(ScreenItem(std::make_shared<SBIDSong>(orgSongID)));
                }
            }
            else
            {
                //	Update screenstack
                currentScreenItem.setEditFlag(0);

                ScreenStack* st=Context::instance()->getScreenStack();
                st->updateSBIDInStack(currentScreenItem);
            }
        }
    }
    //	Close screen
    Context::instance()->getNavigator()->closeCurrentTab();
}

void
SBTabSongEdit::_init()
{
    if(_initDoneFlag==0)
    {
        _initDoneFlag=1;

        const MainWindow* mw=Context::instance()->getMainWindow();

        connect(mw->ui.pbSongEditSave, SIGNAL(clicked(bool)),
                this, SLOT(save()));
        connect(mw->ui.pbSongEditCancel, SIGNAL(clicked(bool)),
                Context::instance()->getNavigator(), SLOT(closeCurrentTab()));
    }
}

ScreenItem
SBTabSongEdit::_populate(const ScreenItem& si)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    _init();

    //	Get detail
    SBIDSong song=SBIDSong(si.ptr()->itemID());
    if(song.getDetail()<0)
    {
        //	Not found
        return ScreenItem();
    }
    ScreenItem currentScreenItem=si;
    currentScreenItem.setEditFlag(1);
    //SBTab::_setCurrentScreenItem(currentScreenItem);

    mw->ui.songEditTitle->setText(song.songTitle());
    mw->ui.songEditPerformerName->setText(song.songPerformerName());
    mw->ui.songEditYearOfRelease->setText(QString("%1").arg(song.year()));
    mw->ui.songEditNotes->setText(song.notes());
    mw->ui.songEditLyrics->setText(song.lyrics());

    //	Disable tmpButtons
    mw->ui.pbNA2->hide();

    //	Set correct focus
    mw->ui.songEditTitle->selectAll();
    mw->ui.songEditTitle->setFocus();

    mw->ui.tabSongEditLists->setCurrentIndex(0);

    return currentScreenItem;
}
