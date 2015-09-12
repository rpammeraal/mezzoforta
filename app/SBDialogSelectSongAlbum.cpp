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
SBDialogSelectSongAlbum::SBDialogSelectSongAlbum(const SBID& id, QWidget *parent, SBDialogSelectSongAlbum::SB_DialogType newDialogType) :
    QDialog(parent),
    ui(new Ui::SBDialogSelectSongAlbum),
    songID(id),
    dialogType(newDialogType)
{
}

void
SBDialogSelectSongAlbum::setTitle(const QString &title)
{

    setWindowTitle(title);
    ui->lHeader->setText(title+':');
}

SBDialogSelectSongAlbum*
SBDialogSelectSongAlbum::selectSongAlbum(const SBID& id, const QSqlQueryModel* m, QWidget *parent)
{
    SBDialogSelectSongAlbum* d=new SBDialogSelectSongAlbum(id,parent,SBDialogSelectSongAlbum::sb_songalbum);
    qDebug() << SB_DEBUG_INFO;
    d->ui->setupUi(d);

    //	Populate choices
    QString title=QString("Choose album for song %1%2%3:").arg(QChar(96)).arg(d->songID.songTitle).arg(QChar(180));
    d->setTitle(title);
    for(int i=0;i<m->rowCount(); i++)
    {
        qDebug() << SB_DEBUG_INFO << i;
        QLabel* l=new QLabel;

        SBID albumID;
        albumID.sb_item_type=SBID::sb_type_album;
        albumID.sb_item_id=m->data(m->index(i,1)).toInt();
        albumID.sb_position=m->data(m->index(i,8)).toInt();

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
                   .arg(m->data(m->index(i,1)).toString())
                   .arg(m->data(m->index(i,8)).toString())
                   .arg(m->data(m->index(i,2)).toString()));
        l->setStyleSheet( ":hover{ background-color: darkgrey; }");
        connect(l, SIGNAL(linkActivated(QString)),
                d, SLOT(OK(QString)));

        d->ui->vlAlbumList->addWidget(l);
    }
    d->updateGeometry();
    return d;
}

SBDialogSelectSongAlbum*
SBDialogSelectSongAlbum::selectPerformer(const QString& editPerformerName, const SBID& id, const QSqlQueryModel* m, QWidget *parent)
{
    SBDialogSelectSongAlbum* d=new SBDialogSelectSongAlbum(id,parent,SBDialogSelectSongAlbum::sb_performer);
    qDebug() << SB_DEBUG_INFO;
    d->ui->setupUi(d);

    //	Populate choices
    QString title=QString("Choose performer");
    d->setTitle(title);
    for(int i=0;i<m->rowCount(); i++)
    {
        qDebug() << SB_DEBUG_INFO << i;
        QLabel* l=new QLabel;

        SBID performerID;
        performerID.sb_item_type=SBID::sb_type_performer;
        performerID.sb_item_id=m->data(m->index(i,1)).toInt();
        performerID.performerName=   m->data(m->index(i,2)).toString();
        qDebug() << SB_DEBUG_INFO << performerID.sb_item_id << performerID.performerName;
        qDebug() << SB_DEBUG_INFO << m->data(m->index(i,2)).toString();

        d->itemsDisplayed[performerID.sb_item_id]=performerID;

        l->setWindowFlags(Qt::FramelessWindowHint);
        l->setTextFormat(Qt::RichText);
        QString imagePath=ExternalData::getCachePath(performerID);
        QFile imageFile(imagePath);

        qDebug() << SB_DEBUG_INFO << l;
        if(imageFile.exists()==0)
        {
            imagePath=SBID::getIconResourceLocation(performerID.sb_item_type);
        }
        qDebug() << SB_DEBUG_INFO
            << imagePath
            << m->data(m->index(i,1)).toString()
            << m->data(m->index(i,2)).toString()
        ;
        l->setText(QString("<html><head><style type=text/css> "
                           "a:link {color:black; text-decoration:none;} "
                           "</style></head><body><font face=\"Trebuchet\"><a href='%2'><img align=\"MIDDLE\" src=\"%1\" width=\"50\">     %3</a></font></body></html>")
                   //	set args correctly
                   .arg(imagePath)
                   .arg(m->data(m->index(i,1)).toString())
                   .arg(m->data(m->index(i,2)).toString())
        );
        l->setStyleSheet( ":hover{ background-color: darkgrey; }");
        connect(l, SIGNAL(linkActivated(QString)),
                d, SLOT(OK(QString)));

        d->ui->vlAlbumList->addWidget(l);
    }
    d->updateGeometry();
    return d;
}

SBDialogSelectSongAlbum*
SBDialogSelectSongAlbum::selectSongByPerformer(const QString& editSongTitleName, const SBID& id, const QSqlQueryModel* m, QWidget *parent)
{
    SBDialogSelectSongAlbum* d=new SBDialogSelectSongAlbum(id,parent,SBDialogSelectSongAlbum::sb_songperformer);
    qDebug() << SB_DEBUG_INFO;
    d->ui->setupUi(d);

    //	Populate choices
    QString title=QString("Choose song");
    d->setTitle(title);
    for(int i=0;i<m->rowCount(); i++)
    {
        QLabel* l=new QLabel;

        SBID songID;
        songID.sb_item_type=SBID::sb_type_performer;
        songID.sb_item_id=m->data(m->index(i,1)).toInt();
        songID.songTitle=m->data(m->index(i,2)).toString();

        d->itemsDisplayed[songID.sb_item_id]=songID;

        l->setWindowFlags(Qt::FramelessWindowHint);
        l->setTextFormat(Qt::RichText);
        QString imagePath=ExternalData::getCachePath(songID);
        QFile imageFile(imagePath);

        if(imageFile.exists()==0)
        {
            imagePath=SBID::getIconResourceLocation(songID.sb_item_type);
        }
        l->setText(QString("<html><head><style type=text/css> "
                           "a:link {color:black; text-decoration:none;} "
                           "</style></head><body><font face=\"Trebuchet\"><a href='%1'>&#8226;     %2 by %3</a></font></body></html>")
                   //	set args correctly
                   .arg(m->data(m->index(i,1)).toString())
                   .arg(m->data(m->index(i,2)).toString())
                   .arg(m->data(m->index(i,4)).toString())
        );
        l->setStyleSheet( ":hover{ background-color: darkgrey; }");
        connect(l, SIGNAL(linkActivated(QString)),
                d, SLOT(OK(QString)));

        d->ui->vlAlbumList->addWidget(l);
    }
    d->updateGeometry();
    return d;
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

bool
SBDialogSelectSongAlbum::hasSelectedItem() const
{
    return hasSelectedItemFlag;
}

///	PRIVATE SLOTS
void
SBDialogSelectSongAlbum::OK(const QString& i)
{
    hasSelectedItemFlag=1;
    QMapIterator<int,SBID> it(itemsDisplayed);
    while(it.hasNext())
    {
        it.next();
        qDebug() << SB_DEBUG_INFO << it.value().sb_item_id << it.value().performerName;
    }

    switch(dialogType)
    {
    case sb_songalbum:
        {
            QStringList l=i.split(":");
            //	i is albumID selected
            //	Flash the selected entry a couple of times.
            songID.sb_album_id=l.at(0).toInt();
            songID.sb_position=l.at(1).toInt();
            qDebug() << SB_DEBUG_INFO << songID << songID.sb_album_id << songID.sb_position;
        }
        break;

    case sb_performer:
        {
            SBID newID;
            newID.sb_item_type=SBID::sb_type_performer;
            newID.sb_item_id=i.toInt();
            newID.sb_performer_id=i.toInt();
            newID.performerName=itemsDisplayed[i.toInt()].performerName;
            qDebug() << SB_DEBUG_INFO << newID.performerName;
            songID=newID;
        }
        break;

    case sb_songperformer:
        {
            SBID newID;
            newID.sb_item_type=SBID::sb_type_song;
            newID.sb_item_id=i.toInt();
            newID.songTitle=itemsDisplayed[i.toInt()].songTitle;
            qDebug() << SB_DEBUG_INFO << newID.performerName;
            songID=newID;
        }
        break;
    }
    this->close();
}

void
SBDialogSelectSongAlbum::init()
{
    ui=NULL;
    hasSelectedItemFlag=0;
}
