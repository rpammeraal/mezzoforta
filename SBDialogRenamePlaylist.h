#ifndef RENAMEPLAYLIST_H
#define RENAMEPLAYLIST_H

#include <QDialog>

#include "SBID.h"

namespace Ui
{
    class SBDialogRenamePlaylist;
}

class SBDialogRenamePlaylist : public QDialog
{
    Q_OBJECT

public:
    explicit SBDialogRenamePlaylist(const SBID& nid, QWidget *parent = 0);
    ~SBDialogRenamePlaylist();

signals:
    void playlistNameChanged(const SBID& id);

private:
    SBID id;
    Ui::SBDialogRenamePlaylist *ui;

private slots:
    void accepted();
};

#endif // RENAMEPLAYLIST_H
