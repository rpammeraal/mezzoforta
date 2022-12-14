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
    explicit SongAlbumNotes(int index, QString songTitle,QString albumNotes,bool showNextSongButton,QWidget *parent = nullptr);
    ~SongAlbumNotes();

    QString albumNotes() const;
    inline QString modSongTitle() const { return _modSongTitle; }
    inline QString orgSongTitle() const { return _orgSongTitle; }
    static bool hasNotes(const QString& str);
    //inline bool loadNextSong() const { return _loadNextSong; }

public slots:
    void findNextNotes();
    void findPrevNotes();
    void move();
    void reset();
    void nextSongButton();
    void doneButton();

private:
    Ui::SongAlbumNotes* ui;
    int					_index;					//	index (in SBTabAlbumEdit) of song being displayed
    QString				_orgSongTitle;
    QString				_modSongTitle;
    int					_commentIndex;
    int					_maxNotesIndex;
    int					_commentStartIndex;
    int					_commentEndIndex;
    QString             _currentSongNotes;
    //bool                _loadNextSong;
    QWidget*            _parent;

    void _adjustButtons();
    static qsizetype _findInString(const QString& str, const QString& toFind, qsizetype fromPosition);
    void _findNextNotes();
    void _init();
    void _setup(int index, const QString& songTitle, const QString& albumNotes);
};

#endif // SONGALBUMNOTE_H
