#include "RenamePlaylist.h"
#include "ui_RenamePlaylist.h"

RenamePlaylist::RenamePlaylist(const SBID& nid,QWidget *parent) :
    QDialog(parent),
    id(nid),
    ui(new Ui::RenamePlaylist)
{
    ui->setupUi(this);
    ui->playlistName->setText(id.playlistName);
    ui->playlistName->setFocus();
    ui->playlistName->selectAll();
    connect(ui->buttonBox, SIGNAL(accepted()),
            this, SLOT(accepted()));

}

RenamePlaylist::~RenamePlaylist()
{
    delete ui;
}

void
RenamePlaylist::accepted()
{
    id.playlistName=ui->playlistName->text();
    emit playlistNameChanged(id);
}
