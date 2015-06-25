#ifndef RENAMEPLAYLIST_H
#define RENAMEPLAYLIST_H

#include <QDialog>

#include "SBID.h"

namespace Ui
{
    class RenamePlaylist;
}

class RenamePlaylist : public QDialog
{
    Q_OBJECT

public:
    explicit RenamePlaylist(const SBID& nid, QWidget *parent = 0);
    ~RenamePlaylist();

signals:
    void playlistNameChanged(const SBID& id);

private:
    SBID id;
    Ui::RenamePlaylist *ui;

private slots:
    void accepted();
};

#endif // RENAMEPLAYLIST_H
