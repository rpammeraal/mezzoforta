#include "SongAlbumNotes.h"
#include "ui_SongAlbumNotes.h"

#include <QDebug>
#include "Common.h"


SongAlbumNotes::SongAlbumNotes(QString songTitle, QString albumNotes, QWidget *parent):
    QDialog(parent),
    ui(new Ui::SongAlbumNotes)
{
    ui->setupUi(this);
    _orgSongTitle=songTitle;
    _modSongTitle=songTitle;

    _init();
    reset();

    ui->songTitle->setText(songTitle);
    ui->albumNotes->setText(albumNotes);
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
    qDebug() << SB_DEBUG_INFO << _maxNotesIndex << _commentIndex;
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
    _loadNextSong=1;
    QDialog::accept();
}

//	Private methods
void
SongAlbumNotes::_adjustButtons()
{
    qDebug() << SB_DEBUG_INFO << _maxNotesIndex << _commentIndex;
    bool moveEnabled=_maxNotesIndex>0;
    bool prevEnabled=_maxNotesIndex>1 && _commentIndex>0;
    bool nextEnabled=_maxNotesIndex>1 && _commentIndex<(_maxNotesIndex-1);

    ui->moveButton->setEnabled(moveEnabled);
    ui->prevButton->setEnabled(prevEnabled);
    ui->nextButton->setEnabled(nextEnabled);
}

qsizetype
SongAlbumNotes::_findInString(const QString& str, const QString& toFind, qsizetype fromPosition) const
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
    _loadNextSong=0;

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
            this, SLOT(accept()));
    connect(ui->nextSongButton, SIGNAL(clicked()),
            this, SLOT(nextSongButton()));
    connect(ui->albumNotes, SIGNAL(returnPressed()),
            this, SLOT(nextSongButton()));
}
