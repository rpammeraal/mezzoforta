#include "ui_SBDialogRenamePlaylist.h"
#include "SBDialogRenamePlaylist.h"

SBDialogRenamePlaylist::SBDialogRenamePlaylist(const SBIDPlaylistPtr& playlistPtr,QWidget *parent) :
    QDialog(parent),
    _playlistPtr(playlistPtr),
    _ui(new Ui::SBDialogRenamePlaylist)
{
    _ui->setupUi(this);
    _ui->playlistName->setText(playlistPtr->playlistName());
    _ui->playlistName->setFocus();
    _ui->playlistName->selectAll();
    connect(_ui->buttonBox, SIGNAL(accepted()),
            this, SLOT(accepted()));

}

SBDialogRenamePlaylist::~SBDialogRenamePlaylist()
{
    delete _ui;
}

void
SBDialogRenamePlaylist::accepted()
{
    _playlistPtr->setPlaylistName(_ui->playlistName->text());
    emit playlistNameChanged(_playlistPtr);
}
