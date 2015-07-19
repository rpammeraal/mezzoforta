#include <QDebug>
#include <QFile>
#include <QLabel>
#include <QPropertyAnimation>
#include <QRadioButton>
#include <QSqlQueryModel>

#include "Common.h"
#include "ExternalData.h"
#include "SBDialogSelectSongAlbum.h"
#include "ui_SBDialogSelectSongAlbum.h"

///	PUBLIC METHODS
SBDialogSelectSongAlbum::SBDialogSelectSongAlbum(const SBID& id, const QSqlQueryModel* m, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SBDialogSelectSongAlbum),
    songID(id)
{
    qDebug() << SB_DEBUG_INFO;
    ui->setupUi(this);

    //	Populate choices
    this->ui->lHeader->setText(QString("Choose album for song %1%2%3:").arg(QChar(96)).arg(songID.songTitle).arg(QChar(180)));
    for(int i=0;i<m->rowCount(); i++)
    {
        qDebug() << SB_DEBUG_INFO << i;
        QLabel* l=new QLabel;

        SBID albumID;
        albumID.sb_item_type=SBID::sb_type_album;
        albumID.sb_item_id=m->data(m->index(i,0)).toInt();
        albumID.sb_position=m->data(m->index(i,6)).toInt();

        l->setWindowFlags(Qt::FramelessWindowHint);
        l->setTextFormat(Qt::RichText);
        QString imagePath=ExternalData::getCachePath(albumID);
        QFile imageFile(imagePath);

        qDebug() << SB_DEBUG_INFO << l;
        if(imageFile.exists()==0)
        {
            imagePath=SBID::getIconResourceLocation(albumID.sb_item_type);
        }
        qDebug() << SB_DEBUG_INFO << imagePath;
        l->setText(QString("<html><head><style type=text/css> "
                           "a:link {color:black; text-decoration:none;} "
                           "</style></head><body><a href='%2:%3'><img align=\"MIDDLE\" src=\"%1\" width=\"50\">     %4</a></body></html>")
                   //	set args correctly
                   .arg(imagePath)
                   .arg(m->data(m->index(i,0)).toString())
                   .arg(m->data(m->index(i,6)).toString())
                   .arg(m->data(m->index(i,1)).toString()));
        l->setStyleSheet( ":hover{ background-color: darkgrey; }");
        connect(l, SIGNAL(linkActivated(QString)),
                this, SLOT(OK(QString)));

        this->ui->vlAlbumList->addWidget(l);
    }
    this->updateGeometry();
}

SBDialogSelectSongAlbum::~SBDialogSelectSongAlbum()
{
    delete ui;
}

SBID
SBDialogSelectSongAlbum::getSBID() const
{
    return songID;
}

///	PRIVATE SLOTS
void
SBDialogSelectSongAlbum::OK(const QString& i)
{
    QStringList l=i.split(":");
    //	i is albumID selected
    //	Flash the selected entry a couple of times.
    songID.sb_album_id=l.at(0).toInt();
    songID.sb_position=l.at(1).toInt();
    qDebug() << SB_DEBUG_INFO << songID << songID.sb_album_id << songID.sb_position;
    this->close();
}
