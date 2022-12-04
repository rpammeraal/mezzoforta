#ifndef SONGALBUMNOTE_H
#define SONGALBUMNOTE_H

#include <QDialog>

namespace Ui {
class SongAlbumNotes;
}

class SongAlbumNotes : public QDialog
{
    Q_OBJECT

public:
    explicit SongAlbumNotes(QString songTitle,QWidget *parent = nullptr);
    ~SongAlbumNotes();

private:
    Ui::SongAlbumNotes  *ui;
    QString				_albumComment;
    QString				_orgSongTitle;
    QString				_modSongTitle;
    int					_commentIndex;
    int					_commentStartIndex;
    int					_commentEndIndex;
    QString             _currentSongComment;

    void _findNextComment();
};

#endif // SONGALBUMNOTE_H
