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
    explicit SBDialogRenamePlaylist(const SBIDPlaylist& nid, QWidget *parent = 0);
    ~SBDialogRenamePlaylist();

signals:
    void playlistNameChanged(const SBIDPlaylist& id);

private:
    SBIDPlaylist id;
    Ui::SBDialogRenamePlaylist *ui;

private slots:
    void accepted();
};

#endif // RENAMEPLAYLIST_H
