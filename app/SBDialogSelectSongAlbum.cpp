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
    _songID(id),
    _dialogType(newDialogType)
{
}

void
SBDialogSelectSongAlbum::setTitle(const QString &title)
{

    setWindowTitle(title);
}

SBDialogSelectSongAlbum*
SBDialogSelectSongAlbum::selectAlbum(const SBID& id, const QSqlQueryModel* m, QWidget *parent)
{
    SBDialogSelectSongAlbum* d=new SBDialogSelectSongAlbum(id,parent,SBDialogSelectSongAlbum::sb_album);
    qDebug() << SB_DEBUG_INFO;
    d->ui->setupUi(d);

    //	Populate choices
    QList<SBID> albumIDPopulated;
    int currentRank=0;
    QString title=QString("Select album to keep as is");
    d->setTitle(title);
    d->ui->lHeader->setText(title+':');
    d->ui->lHeader->setFont(QFont("Trebuchet MS",13));
    for(int i=0;i<m->rowCount(); i++)
    {
        SBID currentAlbum;

        currentRank=m->data(m->index(i,0)).toInt();
        currentAlbum.sb_item_type=SBID::sb_type_album;
        currentAlbum.sb_item_id=m->data(m->index(i,1)).toInt();
        currentAlbum.sb_album_id=currentAlbum.sb_item_id;
        currentAlbum.albumTitle=m->data(m->index(i,2)).toString();
        currentAlbum.sb_performer_id=m->data(m->index(i,3)).toInt();
        currentAlbum.performerName=m->data(m->index(i,4)).toString();


        qDebug() << SB_DEBUG_INFO << "start list";
        for(int i=0;i<albumIDPopulated.count();i++)
        {
            qDebug() << SB_DEBUG_INFO << i << albumIDPopulated.at(i);

        }
        qDebug() << SB_DEBUG_INFO << "end list";

        if(albumIDPopulated.contains(currentAlbum)==0)
        {
            QLabel* l;
            if(i==1)
            {
                l=new QLabel;
                l->setText("Or pick album from this list to merge with:");
                d->ui->vlAlbumList->addWidget(l);
            }

            l=new QLabel;
            l->setWindowFlags(Qt::FramelessWindowHint);
            l->setTextFormat(Qt::RichText);
            l->setFont(QFont("Trebuchet MS",13));

            l->setText(QString("<html><head><style type=text/css> "
                               "a:link {color:black; text-decoration:none;} "
                               "</style></head><body><font face=\"Trebuchet MS\"><a href='%1'>&#8226;     <I>%2</I> by <B>%3</B></a></font></body></html>")
                       .arg(i)
                       .arg(currentAlbum.albumTitle)
                       .arg(currentAlbum.performerName)
            );

            l->setStyleSheet( ":hover{ background-color: darkgrey; }");
            connect(l, SIGNAL(linkActivated(QString)),
                    d, SLOT(OK(QString)));

            d->ui->vlAlbumList->addWidget(l);

            d->_itemsDisplayed[i]=currentAlbum;
            albumIDPopulated.append(currentAlbum);
        }
    }
    d->updateGeometry();
    return d;
}

SBDialogSelectSongAlbum*
SBDialogSelectSongAlbum::selectSongAlbum(const SBID& id, const QSqlQueryModel* m, QWidget *parent)
{
    SBDialogSelectSongAlbum* d=new SBDialogSelectSongAlbum(id,parent,SBDialogSelectSongAlbum::sb_songalbum);
    qDebug() << SB_DEBUG_INFO;
    d->ui->setupUi(d);

    //	Populate choices
    QString title=QString("Choose album for song %1%2%3:").arg(QChar(96)).arg(d->_songID.songTitle).arg(QChar(180));
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
    Q_UNUSED(editPerformerName);
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

        d->_itemsDisplayed[performerID.sb_item_id]=performerID;

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
SBDialogSelectSongAlbum::selectSongByPerformer(const SBID& id, const QSqlQueryModel* m, QWidget *parent)
{
    SBDialogSelectSongAlbum* d=new SBDialogSelectSongAlbum(id,parent,SBDialogSelectSongAlbum::sb_songperformer);
    qDebug() << SB_DEBUG_INFO;
    d->ui->setupUi(d);

    //	Populate choices
    QList<SBID> songIDPopulated;
    int lastSeenRank=0;
    int currentRank=0;
    QString title=QString("Choose Original Performer");
    d->setTitle(title);
    title="<FONT SIZE+=1><B>"+title+"</B></FONT> for <B><I><FONT SIZE=+1>"+id.songTitle+"</FONT></I></B>";
    d->ui->lHeader->setText(title+':');
    d->ui->lHeader->setFont(QFont("Trebuchet MS",13));
    for(int i=0;i<m->rowCount(); i++)
    {
        SBID songID;

        currentRank=m->data(m->index(i,0)).toInt();
        songID.sb_item_type=SBID::sb_type_song;
        songID.sb_item_id=m->data(m->index(i,1)).toInt();
        songID.sb_song_id=songID.sb_item_id;
        songID.songTitle=m->data(m->index(i,2)).toString();
        songID.sb_performer_id=m->data(m->index(i,3)).toInt();
        songID.performerName=m->data(m->index(i,4)).toString();


        qDebug() << SB_DEBUG_INFO << "start list";
        for(int i=0;i<songIDPopulated.count();i++)
        {
            qDebug() << SB_DEBUG_INFO << i << songIDPopulated.at(i);

        }
        qDebug() << SB_DEBUG_INFO << "end list";

        if(songIDPopulated.contains(songID)==0)
        {
            bool isAlternativeSongFlag=0;

            if(currentRank!=lastSeenRank && currentRank==3)
            {
                QLabel* lh=new QLabel;
                lh->setWindowFlags(Qt::FramelessWindowHint);
                lh->setTextFormat(Qt::RichText);
                lh->setText("<FONT SIZE+=1><B>Other Choices:</B></FONT>");
                lh->setFont(QFont("Trebuchet MS",13));
                d->ui->vlAlbumList->addWidget(lh);
            }

            QLabel* l=new QLabel;
            l->setWindowFlags(Qt::FramelessWindowHint);
            l->setTextFormat(Qt::RichText);
            l->setFont(QFont("Trebuchet MS",13));

            if(currentRank==3)
            {
                isAlternativeSongFlag=1;
                l->setText(QString("<html><head><style type=text/css> "
                                   "a:link {color:black; text-decoration:none;} "
                                   "</style></head><body><font face=\"Trebuchet MS\"><a href='%1'>&#8226;     <I>%2</I> by <B>%3</B></a></font></body></html>")
                           .arg(i)
                           .arg(songID.songTitle)
                           .arg(songID.performerName)
                );
            }
            else
            {

                isAlternativeSongFlag=0;
                l->setText(QString("<html><head><style type=text/css> "
                                   "a:link {color:black; text-decoration:none;} "
                                   "</style></head><body><font face=\"Trebuchet MS\"><a href='%1'>&#8226;     %2</a></font></body></html>")
                           .arg(i)
                           .arg(songID.performerName)
                );
            }
            l->setStyleSheet( ":hover{ background-color: darkgrey; }");
            connect(l, SIGNAL(linkActivated(QString)),
                    d, SLOT(OK(QString)));

            d->ui->vlAlbumList->addWidget(l);

            d->_itemsDisplayed[i]=songID;
            songIDPopulated.append(songID);
        }
        lastSeenRank=currentRank;
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
    return _songID;
}


///	PRIVATE SLOTS
void
SBDialogSelectSongAlbum::OK(const QString& i)
{
    _hasSelectedItemFlag=1;
    qDebug() << SB_DEBUG_INFO << i;
    QMapIterator<int,SBID> it(_itemsDisplayed);
    while(it.hasNext())
    {
        it.next();
        qDebug() << SB_DEBUG_INFO << it.value().sb_item_id << it.value().performerName;
    }

    switch(_dialogType)
    {
    case sb_songalbum:
        {
            QStringList l=i.split(":");
            //	i is albumID selected
            //	Flash the selected entry a couple of times.
            _songID.sb_album_id=l.at(0).toInt();
            _songID.sb_position=l.at(1).toInt();
            qDebug() << SB_DEBUG_INFO << _songID << _songID.sb_album_id << _songID.sb_position;
        }
        break;

    case sb_performer:
        {
            SBID newID;
            newID.sb_item_type=SBID::sb_type_performer;
            newID.sb_item_id=i.toInt();
            newID.sb_performer_id=i.toInt();
            newID.performerName=_itemsDisplayed[i.toInt()].performerName;
            qDebug() << SB_DEBUG_INFO << newID.performerName;
            _songID=newID;
        }
        break;

    case sb_songperformer:
        {
            SBID newID;
            newID.sb_item_type=SBID::sb_type_song;
            newID=_itemsDisplayed[i.toInt()];
            _songID=newID;
            qDebug() << SB_DEBUG_INFO << newID;
        }
        break;

    case sb_album:
        {
            SBID newID;
            newID.sb_item_type=SBID::sb_type_album;
            newID=_itemsDisplayed[i.toInt()];
            _songID=newID;
            qDebug() << SB_DEBUG_INFO << newID;
        }
        break;
    }
    this->close();
}

void
SBDialogSelectSongAlbum::init()
{
    ui=NULL;
    _hasSelectedItemFlag=0;
}
