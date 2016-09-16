#include "ui_SBDialogRenamePlaylist.h"
#include "SBDialogRenamePlaylist.h"

SBDialogRenamePlaylist::SBDialogRenamePlaylist(const SBIDPlaylist& nid,QWidget *parent) :
    QDialog(parent),
    id(nid),
    ui(new Ui::SBDialogRenamePlaylist)
{
    ui->setupUi(this);
    ui->playlistName->setText(id.playlistName());
    ui->playlistName->setFocus();
    ui->playlistName->selectAll();
    connect(ui->buttonBox, SIGNAL(accepted()),
            this, SLOT(accepted()));

}

SBDialogRenamePlaylist::~SBDialogRenamePlaylist()
{
    delete ui;
}

void
SBDialogRenamePlaylist::accepted()
{
    id.setPlaylistName(ui->playlistName->text());
    emit playlistNameChanged(id);
}
