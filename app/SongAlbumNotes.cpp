#include "SongAlbumNotes.h"
#include "ui_SongAlbumNotes.h"

#include <QDebug>

#include "Common.h"

SongAlbumNotes::SongAlbumNotes(QString songTitle, QWidget *parent):
    QDialog(parent),
    ui(new Ui::SongAlbumNotes)
{
    qDebug() << SB_DEBUG_INFO << songTitle;
    ui->setupUi(this);
    _orgSongTitle=songTitle;
    _modSongTitle=songTitle;
    _albumComment=QString("");
    _commentIndex=-1;

    ui->songTitle->setText(songTitle);
    ui->albumComment->setText("");
    _findNextComment();
}

SongAlbumNotes::~SongAlbumNotes()
{
    delete ui;
}

void
SongAlbumNotes::_findNextComment()
{
    _commentStartIndex=-1;
    _commentEndIndex=-1;
    _currentSongComment="";
    QString toDisplay="";
    bool commentFound=0;
    int i=0;
    while(i<_modSongTitle.length() && commentFound==0)
    {
        QChar c=_modSongTitle.at(i);
        qDebug() << SB_DEBUG_INFO << i << ":" << c << "[" << _commentStartIndex << ":" << _commentEndIndex << ":" << commentFound << "]:" << _modSongTitle.left(i) << "*" << _modSongTitle.right(_modSongTitle.length()-i);
        if(c=='[' || c=='(')
        {
            _commentStartIndex=i;
        }
        else if(_commentStartIndex>=0 && (c==')' || c==']'))
        {
            _commentEndIndex=i;
            commentFound=1;
            _commentIndex++;
            _currentSongComment=_modSongTitle.mid(_commentStartIndex,_commentEndIndex-_commentStartIndex+1);
        }
        i++;
    }
    qDebug() << SB_DEBUG_INFO << -1 << ":" << '-' << "[" << _commentStartIndex << ":" << _commentEndIndex << ":" << commentFound << "]:" << _modSongTitle.left(i) << "*" << _modSongTitle.right(_modSongTitle.length()-i);
    if(commentFound)
    {
        toDisplay=_modSongTitle.left(_commentStartIndex-1);
        toDisplay+=" <B><U>";
        toDisplay+=_currentSongComment;
        toDisplay+="</U></B> ";
        toDisplay+=_modSongTitle.right(_modSongTitle.length()-_commentEndIndex-1);
    }
    ui->songTitle->setText(toDisplay);
    qDebug() << SB_DEBUG_INFO << toDisplay;
}
