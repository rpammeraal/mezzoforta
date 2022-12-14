#include <QMessageBox>

#include "SBTabSongEdit.h"
#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"

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
    ScreenItem currentScreenItem=this->currentScreenItem();
    if(currentScreenItem.editFlag()==0)
    {
        qDebug() << SB_DEBUG_ERROR << "isEditFlag flag not set. Aborting.";
        return;
    }

    SBIDSongPtr orgSongPtr=SBIDSong::retrieveSong(this->currentScreenItem().key());
    const MainWindow* mw=Context::instance()->mainWindow();
    QString editTitle=mw->ui.songEditTitle->text().simplified();
    QString editPerformerName=mw->ui.songEditPerformerName->text().simplified();
    int editYearOfRelease=mw->ui.songEditYearOfRelease->text().toInt();
    QString editNotes=mw->ui.songEditNotes->text();
    QString editLyrics=mw->ui.songEditLyrics->toPlainText();


    QString updateText;
    int dataHasChanged=0;
    dataHasChanged=SBIDSong::setAndSave(orgSongPtr,editTitle,editPerformerName,editYearOfRelease,editNotes,editLyrics,updateText,1);
    currentScreenItem.setEditFlag(0);
    Context::instance()->controller()->updateStatusBarText(updateText);
    if(dataHasChanged)
    {
        mw->ui.tabAllSongs->preload();
    }
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
