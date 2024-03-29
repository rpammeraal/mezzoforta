#ifndef RENAMEPLAYLIST_H
#define RENAMEPLAYLIST_H

#include <QDialog>

#include "SBIDPlaylist.h"

namespace Ui
{
    class SBDialogRenamePlaylist;
}

class SBDialogRenamePlaylist : public QDialog
{
    Q_OBJECT

public:
    explicit SBDialogRenamePlaylist(const SBIDPlaylistPtr& playlistPtr, bool newPlaylistFlag, QWidget *parent = 0);
    ~SBDialogRenamePlaylist();

signals:
    void playlistNameChanged(const SBIDPlaylistPtr& playlistPtr);

private:
    SBIDPlaylistPtr             _playlistPtr;
    Ui::SBDialogRenamePlaylist* _ui;

private slots:
    void accepted();
};

#endif // RENAMEPLAYLIST_H
