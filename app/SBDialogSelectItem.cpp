#include <QDebug>
#include <QFile>
#include <QLabel>
#include <QPropertyAnimation>
#include <QRadioButton>
#include <QSqlQueryModel>

#include "Common.h"
#include "ExternalData.h"
#include "SBDialogSelectItem.h"
#include "ui_SBDialogSelectItem.h"

///	PUBLIC METHODS
SBDialogSelectItem::SBDialogSelectItem(const SBID& id, QWidget *parent, SBDialogSelectItem::SB_DialogType newDialogType) :
    QDialog(parent),
    ui(new Ui::SBDialogSelectItem),
    _songID(id),
    _dialogType(newDialogType)
{
}

void
SBDialogSelectItem::setTitle(const QString &title)
{

    setWindowTitle(title);
}

SBDialogSelectItem*
SBDialogSelectItem::selectAlbum(const SBID& id, const QSqlQueryModel* m, QWidget *parent)
{
    SBDialogSelectItem* d=new SBDialogSelectItem(id,parent,SBDialogSelectItem::sb_album);
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
        currentAlbum.assign(SBID::sb_type_album,m->data(m->index(i,1)).toInt());
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

SBDialogSelectItem*
SBDialogSelectItem::selectSongAlbum(const SBID& id, const QSqlQueryModel* m, QWidget *parent)
{
    SBDialogSelectItem* d=new SBDialogSelectItem(id,parent,SBDialogSelectItem::sb_songalbum);
    qDebug() << SB_DEBUG_INFO;
    d->ui->setupUi(d);

    //	Populate choices
    QString title=QString("Choose album for song %1%2%3:").arg(QChar(96)).arg(d->_songID.songTitle).arg(QChar(180));
    d->setTitle(title);
    for(int i=0;i<m->rowCount(); i++)
    {
        qDebug() << SB_DEBUG_INFO << i;
        QLabel* l=new QLabel;

        SBID albumID(SBID::sb_type_album,m->data(m->index(i,1)).toInt());
        albumID.sb_position=m->data(m->index(i,8)).toInt();
        albumID.albumTitle=m->data(m->index(i,2)).toString();

        l->setWindowFlags(Qt::FramelessWindowHint);
        l->setTextFormat(Qt::RichText);
        QString imagePath=ExternalData::getCachePath(albumID);
        QFile imageFile(imagePath);

        qDebug() << SB_DEBUG_INFO << l;
        if(imageFile.exists()==0)
        {
            imagePath=SBID::getIconResourceLocation(albumID.sb_item_type());
        }
        qDebug() << SB_DEBUG_INFO << imagePath;
        l->setText(QString("<html><head><style type=text/css> "
                           "a:link {color:black; text-decoration:none;} "
                           "</style></head><body><a href='%2'><img align=\"MIDDLE\" src=\"%1\" width=\"50\">     %3</a></body></html>")
                   //	set args correctly
                   .arg(imagePath)
                   .arg(i)
                   .arg(albumID.albumTitle));
        l->setStyleSheet( ":hover{ background-color: darkgrey; }");
        connect(l, SIGNAL(linkActivated(QString)),
                d, SLOT(OK(QString)));


        d->ui->vlAlbumList->addWidget(l);
        d->_itemsDisplayed[i]=albumID;
    }
    d->updateGeometry();
    return d;
}

SBDialogSelectItem*
SBDialogSelectItem::selectPerformer(const SBID& orgSong, const QSqlQueryModel* m, QWidget *parent)
{
    SBDialogSelectItem* d=new SBDialogSelectItem(orgSong,parent,SBDialogSelectItem::sb_performer);
    qDebug() << SB_DEBUG_INFO;
    d->ui->setupUi(d);

    //	Populate choices
    QList<SBID> songIDPopulated;
    int lastSeenRank=0;
    int currentRank=0;
    QString title=QString("Choose Performer");
    d->setTitle(title);
    d->ui->lHeader->setText(title);
    d->ui->lHeader->setFont(QFont("Trebuchet MS",13));
    qDebug() << SB_DEBUG_INFO << m->rowCount();
    for(int i=0;i<m->rowCount(); i++)
    {
        qDebug() << SB_DEBUG_INFO
                 << m->data(m->index(i,0)).toString()
                 << m->data(m->index(i,1)).toString()
                 << m->data(m->index(i,2)).toString()
        ;
        SBID songID(SBID::sb_type_performer,m->data(m->index(i,1)).toInt());
        currentRank=m->data(m->index(i,0)).toInt();
        songID.performerName=m->data(m->index(i,2)).toString();


        qDebug() << SB_DEBUG_INFO << "current songID=" << songID;

        if(songIDPopulated.contains(songID)==0)
        {
            QString imagePath=ExternalData::getCachePath(songID);
            QFile imageFile(imagePath);
            if(imageFile.exists()==0)
            {
                imagePath=SBID::getIconResourceLocation(songID.sb_item_type());
            }
            qDebug() << SB_DEBUG_INFO << songID << imagePath;

            QLabel* l=new QLabel;
            l->setWindowFlags(Qt::FramelessWindowHint);
            l->setTextFormat(Qt::RichText);
            l->setFont(QFont("Trebuchet MS",13));
            l->setText(QString("<html><head><style type=text/css> "
                               "a:link {color:black; text-decoration:none;} "
                               //"</style></head><body><font face=\"Trebuchet MS\"><a href='%1'>&#8226;     %2</a></font></body></html>")
                               "</style></head><body><font face=\"Trebuchet\"><a href='%2'><img align=\"MIDDLE\" src=\"%1\" width=\"50\">     %3</a></font></body></html>")
                       .arg(imagePath)
                       .arg(i)
                       .arg(songID.performerName)
            );
            qDebug() << SB_DEBUG_INFO << l->text();

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

SBDialogSelectItem*
SBDialogSelectItem::selectSongByPerformer(const SBID& orgSong, const QSqlQueryModel* m, QWidget *parent)
{
    SBDialogSelectItem* d=new SBDialogSelectItem(orgSong,parent,SBDialogSelectItem::sb_songperformer);
    qDebug() << SB_DEBUG_INFO;
    d->ui->setupUi(d);

    //	Populate choices
    QList<SBID> songIDPopulated;
    int lastSeenRank=0;
    int currentRank=0;
    QString title=QString("Choose Song");
    d->setTitle(title);
    title="<FONT SIZE+=1><B>"+title+"</B></FONT> for <B><I><FONT SIZE=+1>"+orgSong.songTitle+"</FONT></I></B>";
    d->ui->lHeader->setText(title+':');
    d->ui->lHeader->setFont(QFont("Trebuchet MS",13));
    for(int i=0;i<m->rowCount(); i++)
    {
        SBID songID;

        currentRank=m->data(m->index(i,0)).toInt();
        songID.assign(SBID::sb_type_song,m->data(m->index(i,1)).toInt());
        songID.songTitle=m->data(m->index(i,2)).toString();
        songID.sb_performer_id=m->data(m->index(i,3)).toInt();
        songID.performerName=m->data(m->index(i,4)).toString();


        qDebug() << SB_DEBUG_INFO << "current songID=" << songID;

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
                           .arg(songID.songTitle)
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

SBDialogSelectItem::~SBDialogSelectItem()
{
    delete ui;
}

SBID
SBDialogSelectItem::getSBID() const
{
    return _songID;
}


///	PRIVATE SLOTS
void
SBDialogSelectItem::OK(const QString& i)
{
    _hasSelectedItemFlag=1;
    qDebug() << SB_DEBUG_INFO << i;
    QMapIterator<int,SBID> it(_itemsDisplayed);
    while(it.hasNext())
    {
        it.next();
        qDebug() << SB_DEBUG_INFO << it.value().sb_item_id() << it.value().performerName;
    }

    _songID=_itemsDisplayed[i.toInt()];
    qDebug() << SB_DEBUG_INFO << _songID;
    qDebug() << SB_DEBUG_INFO << "position=" << _songID.sb_position;

    this->close();
}

void
SBDialogSelectItem::init()
{
    ui=NULL;
    _hasSelectedItemFlag=0;
}