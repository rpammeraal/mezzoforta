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
    explicit SongAlbumNotes(QString songTitle,QString albumNotes,QWidget *parent = nullptr);
    ~SongAlbumNotes();

    QString albumNotes() const;
    inline QString modSongTitle() const { return _modSongTitle; }
    inline QString orgSongTitle() const { return _orgSongTitle; }

public slots:
    void findNextNotes();
    void findPrevNotes();
    void move();
    void reset();
    void nextSongButton();

private:
    Ui::SongAlbumNotes* ui;
    QString				_orgSongTitle;
    QString				_modSongTitle;
    int					_commentIndex;
    int					_maxNotesIndex;
    int					_commentStartIndex;
    int					_commentEndIndex;
    QString             _currentSongNotes;
    bool                _loadNextSong;

    qsizetype _findInString(const QString& str, const QString& toFind, qsizetype fromPosition) const;
    void _findNextNotes();
    void _init();
};

#endif // SONGALBUMNOTE_H
