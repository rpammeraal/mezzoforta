#include "SongAlbumNotes.h"
#include "ui_SongAlbumNotes.h"

#include <QDebug>
#include "Common.h"
#include "SBTabAlbumEdit.h"


SongAlbumNotes::SongAlbumNotes(int index, QString songTitle, QString albumNotes, bool showNextSongButton, QWidget *parent):
    QDialog(parent),
    ui(new Ui::SongAlbumNotes)
{
    ui->setupUi(this);
    _parent=parent;

    _init();
    _setup(index,songTitle,albumNotes);
    reset();

    ui->nextSongButton->setEnabled(showNextSongButton);
    _findNextNotes();
}

SongAlbumNotes::~SongAlbumNotes()
{
    delete ui;
}

//	Public methods
QString
SongAlbumNotes::albumNotes() const
{
    return ui->albumNotes->text();
}

bool
SongAlbumNotes::hasNotes(const QString& str)
{
    return (_findInString(str,")]",0)!=-1);
}

//	Public slots
void
SongAlbumNotes::findNextNotes()
{
    _commentIndex=(_commentIndex+1<_maxNotesIndex)?_commentIndex+1:_commentIndex;
    _findNextNotes();
    _adjustButtons();
}

void
SongAlbumNotes::findPrevNotes()
{
    _commentIndex=(_commentIndex<0)?0:_commentIndex-1;
    _findNextNotes();
    _adjustButtons();
}

void
SongAlbumNotes::move()
{
    //	Strip comment of parentheses at beginning and end
    QString comment=_currentSongNotes.mid(1,_currentSongNotes.length()-2);
    QString currentAlbumNotes=ui->albumNotes->text();

    //	Create new song title
    QString newSongTitle=_modSongTitle.left(_commentStartIndex);

    if(_commentEndIndex+1<_modSongTitle.length())
    {
        _commentEndIndex++;		//	move past parenthesis
    }
    if(_modSongTitle.at(_commentEndIndex)!=')' && _modSongTitle.at(_commentEndIndex)!=']')
    {
        newSongTitle+=_modSongTitle.mid(_commentEndIndex);
    }
    _modSongTitle=newSongTitle;

    //	Adjust comments if not empty
    if(currentAlbumNotes.length()!=0)
    {
        comment=", " + comment;
    }
    currentAlbumNotes+=comment;

    //	Adjust indexes
    _maxNotesIndex--;
    while(_commentIndex>=_maxNotesIndex)
    {
        _commentIndex--;
    }
    _commentStartIndex=-1;
    _commentEndIndex=-1;
    _currentSongNotes="";

    //	Update UI
    _modSongTitle=_modSongTitle.trimmed();
    currentAlbumNotes=currentAlbumNotes.trimmed();

    ui->songTitle->setText(_modSongTitle);
    ui->albumNotes->setText(currentAlbumNotes);
    _findNextNotes();

    if(_maxNotesIndex==0)
    {
        nextSongButton();
    }
}

void
SongAlbumNotes::reset()
{
    _modSongTitle=_orgSongTitle;
    _currentSongNotes=QString();
    _commentIndex=-1;
    _maxNotesIndex=-1;
    _commentStartIndex=-1;
    _commentEndIndex=-1;

    ui->songTitle->setText(_modSongTitle);
    ui->albumNotes->setText("");
    _findNextNotes();
}

void
SongAlbumNotes::nextSongButton()
{
    SBTabAlbumEdit* tae=dynamic_cast<SBTabAlbumEdit *>(_parent);
    SB_RETURN_VOID_IF_NULL(tae);

    tae->saveSongData(_index,_modSongTitle,this->albumNotes());
    QString songTitle;
    QString albumNotes;

    bool songDataLeft=1;

    while(songDataLeft)
    {
        songDataLeft=tae->getSongData(_index+1, songTitle, albumNotes);
        if(songDataLeft)
        {
            _setup(_index+1,songTitle,albumNotes);
            reset();
            _findNextNotes();

            if(_maxNotesIndex>0)
            {
                return;
            }
        }
        else
        {
            ui->nextSongButton->setEnabled(false);
        }
    }
    QDialog::accept();
}

void
SongAlbumNotes::doneButton()
{
    SBTabAlbumEdit* tae=dynamic_cast<SBTabAlbumEdit *>(_parent);
    SB_RETURN_VOID_IF_NULL(tae);

    tae->saveSongData(_index,_modSongTitle,this->albumNotes());
    QDialog::accept();
}

//	Private methods
void
SongAlbumNotes::_adjustButtons()
{
    bool moveEnabled=_maxNotesIndex>0;
    bool prevEnabled=_maxNotesIndex>1 && _commentIndex>0;
    bool nextEnabled=_maxNotesIndex>1 && _commentIndex<(_maxNotesIndex-1);

    ui->moveButton->setEnabled(moveEnabled);
    ui->prevButton->setEnabled(prevEnabled);
    ui->nextButton->setEnabled(nextEnabled);
}

qsizetype
SongAlbumNotes::_findInString(const QString& str, const QString& toFind, qsizetype fromPosition)
{
    qsizetype pos=-1;
    for(qsizetype i=0;i<toFind.length();i++)
    {
        const QChar c=toFind.at(i);
        const int found=str.indexOf(c,fromPosition);
        if(found>=0)
        {
            if(pos==-1 || found<pos)
            {
                pos=found;
            }
        }
    }
    return pos;
}

void
SongAlbumNotes::_findNextNotes()
{
    _commentStartIndex=-1;
    _commentEndIndex=-1;
    _currentSongNotes="";
    QString toDisplay="";
    bool commentFound=0;

    //	Determine amount of comments
    _maxNotesIndex=_modSongTitle.count("(")+_modSongTitle.count("[");
    if(_maxNotesIndex && _commentIndex==-1)
    {
        //	Only set during 1st run -- this is when _commentIndex==-1
        _commentIndex=0;
    }

    //	Go to 'current' (_commentIndex) comment.
    for(int i=0;i<=_commentIndex;i++)
    {
        _commentStartIndex++;
        _commentStartIndex=_findInString(_modSongTitle,"[(",_commentStartIndex);
    }

    //	Find end of current comment
    if(_commentStartIndex!=-1)
    {
        _commentEndIndex=_findInString(_modSongTitle,")]",_commentStartIndex);
        _currentSongNotes=_modSongTitle.mid(_commentStartIndex,_commentEndIndex-_commentStartIndex+1);
        commentFound=1;
    }

    if(commentFound)
    {
        toDisplay=_modSongTitle.left(_commentStartIndex-1);
        toDisplay+=" <B><U><font size='+1'>";
        toDisplay+=_currentSongNotes;
        toDisplay+="</font></U></B> ";
        toDisplay+=_modSongTitle.right(_modSongTitle.length()-_commentEndIndex-1);
    }
    else
    {
        toDisplay=_modSongTitle + " <font color='grey'><I>&lt;no comments&gt;</I></font>";
    }
    ui->songTitle->setText(toDisplay);

    _adjustButtons();
}

void
SongAlbumNotes::_init()
{
    connect(ui->moveButton, SIGNAL(clicked(bool)),
            this, SLOT(move()));
    connect(ui->nextButton, SIGNAL(clicked(bool)),
            this, SLOT(findNextNotes()));
    connect(ui->prevButton, SIGNAL(clicked(bool)),
            this, SLOT(findPrevNotes()));
    connect(ui->resetButton, SIGNAL(clicked(bool)),
            this, SLOT(reset()));

    connect(ui->cancelButton, SIGNAL(clicked(bool)),
            this, SLOT(reject()));
    connect(ui->doneButton, SIGNAL(clicked()),
            this, SLOT(doneButton()));
    connect(ui->nextSongButton, SIGNAL(clicked()),
            this, SLOT(nextSongButton()));
    connect(ui->albumNotes, SIGNAL(returnPressed()),
            this, SLOT(nextSongButton()));

    Common::bindShortcut(ui->nextSongButton, Qt::Key_N | Qt::CTRL);
    ui->nextSongButton->setShortcut(Qt::Key_N | Qt::CTRL);
    Common::bindShortcut(ui->moveButton, Qt::Key_M | Qt::CTRL);
    ui->moveButton->setShortcut(Qt::Key_M | Qt::CTRL);

}

void
SongAlbumNotes::_setup(int index, const QString& songTitle, const QString& albumNotes)
{
    _index=index;
    _orgSongTitle=songTitle;
    _modSongTitle=songTitle;
    ui->songTitle->setText(songTitle);
    ui->albumNotes->setText(albumNotes);
}
