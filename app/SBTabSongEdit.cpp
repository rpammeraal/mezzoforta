#include <QDebug>
#include <QCompleter>
#include <QMessageBox>
#include <QTableWidget>

#include "SBTabSongEdit.h"

#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "SBDialogSelectItem.h"
#include "SBModelSong.h"
#include "SBSqlQueryModel.h"
#include "Navigator.h"

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
    const SBID& currentID=Context::instance()->getScreenStack()->currentScreen();
    const MainWindow* mw=Context::instance()->getMainWindow();

    qDebug() << SB_DEBUG_INFO << currentID;

    if(currentID.sb_item_type()!=SBID::sb_type_invalid)
    {
        if(currentID.songTitle!=mw->ui.songEditTitle->text() ||
            currentID.performerName!=mw->ui.songEditPerformerName->text() ||
            currentID.year!=mw->ui.songEditYearOfRelease->text().toInt() ||
            currentID.notes!=mw->ui.songEditNotes->text() ||
            currentID.lyrics!=mw->ui.songEditLyrics->toPlainText()
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

    const MainWindow* mw=Context::instance()->getMainWindow();
    SBID orgSongID=Context::instance()->getScreenStack()->currentScreen();
    SBID newSongID=orgSongID;

    Context::instance()->getScreenStack()->debugShow("save");

    qDebug() << SB_DEBUG_INFO << "orgSong" << orgSongID;
    qDebug() << SB_DEBUG_INFO << "newSong" << newSongID;

    if(orgSongID.sb_song_id==-1 || newSongID.sb_song_id==-1)
    {
        QMessageBox msgBox;
        msgBox.setText("SBTabSongEdit::save:old or new song undefined");
        msgBox.exec();
        return;
    }

    if(orgSongID.isEditFlag==0)
    {
        qDebug() << SB_DEBUG_INFO << "isEditFlag flag not set";
        return;
    }

    QString editTitle=mw->ui.songEditTitle->text();
    QString editPerformerName=mw->ui.songEditPerformerName->text();
    int editYearOfRelease=mw->ui.songEditYearOfRelease->text().toInt();
    QString editNotes=mw->ui.songEditNotes->text();
    QString editLyrics=mw->ui.songEditLyrics->toPlainText();
    bool hasCaseChange=0;

    qDebug() << SB_DEBUG_INFO << editTitle << editPerformerName;
    qDebug() << SB_DEBUG_INFO << orgSongID.songTitle << newSongID.performerName;
    qDebug() << SB_DEBUG_INFO << newSongID;
    qDebug() << SB_DEBUG_INFO << "orgSongID:sb_performer_id" << orgSongID.sb_performer_id;
    qDebug() << SB_DEBUG_INFO << "orgSongID:sb_song_id" << orgSongID.sb_song_id;
    qDebug() << SB_DEBUG_INFO << "newSongID:sb_performer_id" << newSongID.sb_performer_id;
    qDebug() << SB_DEBUG_INFO << "newSongID:sb_song_id" << newSongID.sb_song_id;

    //	If only case is different in songTitle, save the new title as is.
    if((editTitle.toLower()==newSongID.songTitle.toLower()) &&
        (editTitle!=newSongID.songTitle) &&
        (editPerformerName==newSongID.performerName))
    {
        qDebug() << SB_DEBUG_INFO;
        newSongID.sb_song_id=-1;
        newSongID.songTitle=editTitle;
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
    qDebug() << SB_DEBUG_INFO << "orgSongID:sb_performer_id" << orgSongID.sb_performer_id;
    qDebug() << SB_DEBUG_INFO << "newSongID:sb_performer_id" << newSongID.sb_performer_id;

    //	Handle performer name edits
    if(hasCaseChange==0 && editPerformerName!=newSongID.performerName)
    {
        SBID selectedPerformer=newSongID;
        if(processPerformerEdit(editPerformerName,selectedPerformer,mw->ui.songEditPerformerName)==0)
        {
            return;
        }
        newSongID.sb_performer_id=selectedPerformer.sb_performer_id;
        newSongID.performerName=selectedPerformer.performerName;
    }

    qDebug() << SB_DEBUG_INFO << "orgSongID:sb_performer_id" << orgSongID.sb_performer_id;
    qDebug() << SB_DEBUG_INFO << "orgSongID:sb_song_id" << orgSongID.sb_song_id;
    qDebug() << SB_DEBUG_INFO << "orgSongID:performerName" << orgSongID.performerName;
    qDebug() << SB_DEBUG_INFO << "orgSongID:performerName" << orgSongID.songTitle;
    qDebug() << SB_DEBUG_INFO << "newSongID:sb_performer_id" << newSongID.sb_performer_id;
    qDebug() << SB_DEBUG_INFO << "newSongID:sb_song_id" << newSongID.sb_song_id;
    qDebug() << SB_DEBUG_INFO << "newSongID:performerName" << newSongID.performerName;
    qDebug() << SB_DEBUG_INFO << "newSongID:performerName" << newSongID.songTitle;

    //	Handle song title edits
    if(editTitle.toLower()!=newSongID.songTitle.toLower() &&
        hasCaseChange==0)
    {
        SBID selectedSongID(SBID::sb_type_song,-1);
        selectedSongID.songTitle=editTitle;
        qDebug() << SB_DEBUG_INFO << selectedSongID;

        qDebug() << SB_DEBUG_INFO << editTitle << "!=" << newSongID.songTitle << newSongID.sb_song_id;
        SBSqlQueryModel* songMatches=SBModelSong::matchSongWithinPerformer(newSongID, editTitle);

        qDebug() << SB_DEBUG_INFO << songMatches->rowCount();

        if(songMatches->rowCount()>1)
        {

            if(songMatches->rowCount()>=2 &&
                songMatches->record(1).value(0).toInt()==1
            )
            {
                selectedSongID.sb_song_id=songMatches->record(1).value(1).toInt();
                selectedSongID.songTitle=songMatches->record(1).value(2).toString();
                qDebug() << SB_DEBUG_INFO << "selectedSongID.sb_song_id" << selectedSongID.sb_song_id;
            }
            else
            {
                SBDialogSelectItem* pu=SBDialogSelectItem::selectSongByPerformer(selectedSongID,songMatches);
                pu->exec();

                selectedSongID=pu->getSBID();

                //	Go back to screen if no item has been selected
                if(pu->hasSelectedItem()==0)
                {
                    return;
                }
                qDebug() << SB_DEBUG_INFO << "selectedSongID.sb_song_id" << selectedSongID.sb_song_id;
            }

            //	Update field
            mw->ui.songEditTitle->setText(selectedSongID.songTitle);
        }

        //	Update newSongID
        newSongID.sb_song_id=selectedSongID.sb_song_id;
        newSongID.songTitle=selectedSongID.songTitle;
        qDebug() << SB_DEBUG_INFO << "selected song" << newSongID.sb_song_id << newSongID.songTitle;
    }
    newSongID.year=editYearOfRelease;
    newSongID.notes=editNotes;
    newSongID.lyrics=editLyrics;

    //	H.	Check if there is another song with exactly the same title and performer but with different song_id.
    //		Goal is to detect another way of merging songs.
    qDebug() << SB_DEBUG_INFO << orgSongID;
    qDebug() << SB_DEBUG_INFO << newSongID;
    SBSqlQueryModel* existingSongs=SBModelSong::findSong(newSongID);
    qDebug() << SB_DEBUG_INFO << existingSongs->rowCount();
    if(existingSongs!=0 && existingSongs->rowCount()==1)
    {
        qDebug() << SB_DEBUG_INFO << "Found exact song for:" << newSongID;
        newSongID.sb_song_id=existingSongs->record(0).value(0).toInt();
        newSongID.notes=existingSongs->record(0).value(2).toString();
        newSongID.sb_performer_id=existingSongs->record(0).value(3).toInt();
        newSongID.year=existingSongs->record(0).value(5).toInt();
        newSongID.lyrics=existingSongs->record(0).value(6).toInt();

        qDebug() << SB_DEBUG_INFO << "Changed to:" << newSongID;
    }

    qDebug() << SB_DEBUG_INFO << "orgSongID:sb_song_id" << orgSongID.sb_song_id;
    qDebug() << SB_DEBUG_INFO << "newSongID:sb_song_id" << newSongID.sb_song_id;

    Context::instance()->getScreenStack()->debugShow("save_225");

    if(orgSongID!=newSongID ||
        orgSongID.year!=newSongID.year ||
        orgSongID.notes!=newSongID.notes ||
        orgSongID.lyrics!=newSongID.lyrics ||
        hasCaseChange==1)
    {
        qDebug() << SB_DEBUG_INFO;

        const bool successFlag=SBModelSong::updateExistingSong(orgSongID,newSongID,QStringList(),1);

        if(successFlag==1)
        {
            QString updateText=QString("Saved song %1%2%3.")
                .arg(QChar(96))      //	1
                .arg(newSongID.songTitle)   //	2
                .arg(QChar(180));    //	3
            Context::instance()->getController()->updateStatusBar(updateText);

            if(orgSongID!=newSongID)
            {
                //	Update models!
                Context::instance()->getController()->refreshModels();

                if(orgSongID.sb_song_id!=newSongID.sb_song_id)
                {
                    //	Remove old from screenstack
                    Context::instance()->getScreenStack()->removeScreen(orgSongID);
                }
            }
            else
            {
                //	Update screenstack
                newSongID.isEditFlag=0;

                ScreenStack* st=Context::instance()->getScreenStack();
                st->updateSBIDInStack(newSongID);
            }
        }
    }
    //	Close screen
    Context::instance()->getScreenStack()->debugShow("save_264");
    Context::instance()->getNavigator()->closeCurrentTab();
    Context::instance()->getScreenStack()->debugShow("save_266");
}

void
SBTabSongEdit::init()
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

SBID
SBTabSongEdit::_populate(const SBID& id)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    init();

    //	Get detail
    SBID result=SBModelSong::getDetail(id);
    if(result.sb_song_id==-1)
    {
        //	Not found
        return result;
    }
    result.isEditFlag=1;

    qDebug() << SB_DEBUG_INFO << result;
    mw->ui.songEditTitle->setText(result.songTitle);
    mw->ui.songEditPerformerName->setText(result.performerName);
    mw->ui.songEditYearOfRelease->setText(QString("%1").arg(result.year));
    mw->ui.songEditNotes->setText(result.notes);
    mw->ui.songEditLyrics->setText(result.lyrics);

    qDebug() << SB_DEBUG_INFO << result.performerName;
    qDebug() << SB_DEBUG_INFO << mw->ui.songEditPerformerName->text();

    //	Disable tmpButtons
    mw->ui.pbNA2->hide();

    //	Set correct focus
    mw->ui.songEditTitle->selectAll();
    mw->ui.songEditTitle->setFocus();

    mw->ui.tabSongEditLists->setCurrentIndex(0);

    return result;
}