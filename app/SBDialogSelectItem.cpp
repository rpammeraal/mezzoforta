#include <QDebug>
#include <QFile>
#include <QLabel>
#include <QPropertyAnimation>
#include <QRadioButton>
#include <QSqlQueryModel>

#include "Common.h"
#include "Context.h"
#include "ExternalData.h"
#include "SBDialogSelectItem.h"
#include "SBIDAlbum.h"
#include "SBIDPerformer.h"
#include "SBIDSong.h"
#include "ui_SBDialogSelectItem.h"

///	PUBLIC METHODS
SBDialogSelectItem::SBDialogSelectItem(const SBIDPtr& ptr, QWidget *parent, SBDialogSelectItem::SB_DialogType newDialogType) :
    QDialog(parent),
    ui(new Ui::SBDialogSelectItem),
    _currentPtr(ptr),
    _dialogType(newDialogType)
{
}

void
SBDialogSelectItem::setTitle(const QString &title)
{

    setWindowTitle(title);
}

SBDialogSelectItem*
SBDialogSelectItem::selectAlbum(const SBIDPtr& ptr, const QSqlQueryModel* m, QWidget *parent)
{
    SBDialogSelectItem* d=new SBDialogSelectItem(ptr,parent,SBDialogSelectItem::sb_album);
    d->ui->setupUi(d);

    //	Populate choices
    QList<SBIDBase> albumIDPopulated;
    QString title=QString("Select album to keep as is");
    d->setTitle(title);
    d->ui->lHeader->setText(title+':');
    d->ui->lHeader->setFont(QFont("Trebuchet MS",13));
    for(int i=0;i<m->rowCount(); i++)
    {
        SBIDAlbum currentAlbum(m->data(m->index(i,1)).toInt());
        currentAlbum.setAlbumTitle(m->data(m->index(i,2)).toString());
        currentAlbum.setAlbumPerformerID(m->data(m->index(i,3)).toInt());
        currentAlbum.setAlbumPerformerName(m->data(m->index(i,4)).toString());

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
                       .arg(currentAlbum.albumTitle())
                       .arg(currentAlbum.albumPerformerName())
            );

            l->setStyleSheet( ":hover{ background-color: darkgrey; }");
            connect(l, SIGNAL(linkActivated(QString)),
                    d, SLOT(OK(QString)));

            d->ui->vlAlbumList->addWidget(l);

            d->_itemsDisplayed[i]=std::make_shared<SBIDAlbum>(currentAlbum);
            albumIDPopulated.append(currentAlbum);
        }
    }
    d->updateGeometry();
    return d;
}

SBDialogSelectItem*
SBDialogSelectItem::selectSongAlbum(const SBIDPtr& ptr, const QSqlQueryModel* m, QWidget *parent)
{
    SBDialogSelectItem* dialog=new SBDialogSelectItem(ptr,parent,SBDialogSelectItem::sb_songalbum);
    dialog->ui->setupUi(dialog);

    //	Populate choices
    QString title=QString("Choose album and performer for song %1%2%3:").arg(QChar(96)).arg(dialog->_currentPtr->songTitle()).arg(QChar(180));
    dialog->setTitle(title);
    for(int i=0;i<m->rowCount(); i++)
    {
        QLabel* l=new QLabel;

        SBIDSong songChoice(ptr->itemID());
        songChoice.setSongTitle(ptr->songTitle());
        songChoice.setAlbumID(m->data(m->index(i,1)).toInt());
        songChoice.setAlbumTitle(m->data(m->index(i,2)).toString());
        songChoice.setDuration(m->data(m->index(i,3)).toTime());
        songChoice.setYear(m->data(m->index(i,4)).toInt());
        songChoice.setSongPerformerID(m->data(m->index(i,6)).toInt());
        songChoice.setSongPerformerName(m->data(m->index(i,7)).toString());
        songChoice.setAlbumPosition(m->data(m->index(i,9)).toInt());
        songChoice.setPath(m->data(m->index(i,10)).toString());

        SBIDAlbum albumIcon(songChoice.albumID());

        l->setWindowFlags(Qt::FramelessWindowHint);
        l->setTextFormat(Qt::RichText);
        QString imagePath=ExternalData::getCachePath(std::make_shared<SBIDAlbum>(albumIcon));
        QFile imageFile(imagePath);

        if(imageFile.exists()==0)
        {
            imagePath=songChoice.iconResourceLocation();
        }
        l->setText(QString("<html><head><style type=text/css> "
                           "a:link {color:black; text-decoration:none;} "
                           "</style></head><body><a href='%2'><img align=\"MIDDLE\" src=\"%1\" width=\"50\">     by %4 on album '%3' (%5)</a></body></html>")
                   //	set args correctly
                   .arg(imagePath)
                   .arg(i)
                   .arg(songChoice.albumTitle())
                   .arg(songChoice.songPerformerName())
                   .arg(songChoice.duration().toString(Duration::sb_hhmmss_format)));

        l->setStyleSheet( ":hover{ background-color: darkgrey; }");
        connect(l, SIGNAL(linkActivated(QString)),
                dialog, SLOT(OK(QString)));


        dialog->ui->vlAlbumList->addWidget(l);
        dialog->_itemsDisplayed[i]=std::make_shared<SBIDSong>(songChoice);
    }
    dialog->updateGeometry();
    return dialog;
}

SBDialogSelectItem*
SBDialogSelectItem::selectPerformer(const SBIDPtr& ptr, QList<QList<SBIDPerformerPtr>> matches, QWidget *parent)
{
    //	Used by MusicLibrary to import songs

    SBDialogSelectItem* d=new SBDialogSelectItem(ptr,parent,SBDialogSelectItem::sb_performer);
    d->ui->setupUi(d);

    //	Populate choices
    QList<SBIDPerformerPtr> performersShown;
    QString title=QString("Choose Performer ");
    d->setTitle(title);
    d->ui->lHeader->setText(title);
    d->ui->lHeader->setFont(QFont("Trebuchet MS",13));

    int performerCount=0;
    for(int i=0;i<matches.count();i++)
    {
        for(int j=0;j<matches[i].count();j++)
        {
            SBIDPerformerPtr currentPerformerPtr=matches[i][j];

            if(performersShown.contains(currentPerformerPtr)==0)
            {
                QString imagePath=ExternalData::getCachePath(currentPerformerPtr);
                QFile imageFile(imagePath);
                if(imageFile.exists()==0)
                {
                    imagePath=currentPerformerPtr->iconResourceLocation();
                }

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
                           .arg(currentPerformerPtr->performerName())
                );

                l->setStyleSheet( ":hover{ background-color: darkgrey; }");
                connect(l, SIGNAL(linkActivated(QString)),
                        d, SLOT(OK(QString)));

                d->ui->vlAlbumList->addWidget(l);

                d->_itemsDisplayed[performerCount++]=currentPerformerPtr;
                performersShown.append(currentPerformerPtr);
            }
        }
    }
    d->updateGeometry();
    return d;
}

SBDialogSelectItem*
SBDialogSelectItem::selectSongByPerformer(const SBIDPtr& orgSong, const QSqlQueryModel* m, QWidget *parent)
{
    SBDialogSelectItem* d=new SBDialogSelectItem(orgSong,parent,SBDialogSelectItem::sb_songperformer);
    d->ui->setupUi(d);

    //	Populate choices
    QList<SBIDBase> songsShown;
    int lastSeenRank=0;
    int currentRank=0;
    QString title=QString("Who is the original performer");
    d->setTitle(title+"?");
    title="<FONT SIZE+=1><B>"+title+" for </B></FONT><B><I><FONT SIZE=+1>"+orgSong->songTitle()+"</FONT></I></B>";
    d->ui->lHeader->setText(title+':');
    d->ui->lHeader->setFont(QFont("Trebuchet MS",13));
    for(int i=0;i<m->rowCount(); i++)
    {
        SBIDSong currentSong(m->data(m->index(i,1)).toInt());

        currentRank=m->data(m->index(i,0)).toInt();
        currentSong.setSongTitle(m->data(m->index(i,2)).toString());
        currentSong.setSongPerformerID(m->data(m->index(i,3)).toInt());
        currentSong.setSongPerformerName(m->data(m->index(i,4)).toString());

        if(songsShown.contains(currentSong)==0)
        {
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

            switch(currentRank)
            {
            case 0:
                l->setText(QString("<html><head><style type=text/css> "
                                   "a:link {color:black; text-decoration:none;} "
                                   "</style></head><body><font face=\"Trebuchet MS\"><a href='%1'>&#8226;     Is <I>%2</I> a <B>%3</B> original, or </a></font></body></html>")
                           .arg(i)
                           .arg(currentSong.songTitle())
                           .arg(currentSong.songPerformerName())
                );
                break;
            case 1:
                l->setText(QString("<html><head><style type=text/css> "
                                   "a:link {color:black; text-decoration:none;} "
                                   "</style></head><body><font face=\"Trebuchet MS\"><a href='%1'>&#8226;     Is this a <B>%4</B> cover of the <B>%3</B> song <I>%2</I>?</a></font></body></html>")
                           .arg(i)
                           .arg(currentSong.songTitle())
                           .arg(currentSong.songPerformerName())
                           .arg(orgSong->songPerformerName())
                );
                break;

            case 2:
                l->setText(QString("<html><head><style type=text/css> "
                                   "a:link {color:black; text-decoration:none;} "
                                   "</style></head><body><font face=\"Trebuchet MS\"><a href='%1'>&#8226;     Is this a <B>%4</B> cover of the <B>%3</B> song <I>%2</I>?</a></font></body></html>")
                           .arg(i)
                           .arg(currentSong.songTitle())
                           .arg(currentSong.songPerformerName())
                           .arg(orgSong->songPerformerName())
                );
                break;

            case 3:
                l->setText(QString("<html><head><style type=text/css> "
                                   "a:link {color:black; text-decoration:none;} "
                                   "</style></head><body><font face=\"Trebuchet MS\"><a href='%1'>&#8226;     <I>%2</I>(3) by <B>%3</B></a></font></body></html>")
                           .arg(i)
                           .arg(currentSong.songTitle())
                           .arg(currentSong.songPerformerName())
                );
            default:
                break;

            }

            l->setStyleSheet( ":hover{ background-color: darkgrey; }");
            connect(l, SIGNAL(linkActivated(QString)),
                    d, SLOT(OK(QString)));

            d->ui->vlAlbumList->addWidget(l);

            d->_itemsDisplayed[i]=std::make_shared<SBIDSong>(currentSong);
            songsShown.append(currentSong);
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

SBIDPtr
SBDialogSelectItem::getSelected() const
{
    return _currentPtr;
}


///	PRIVATE SLOTS
void
SBDialogSelectItem::OK(const QString& i)
{
    _hasSelectedItemFlag=1;
    QMapIterator<int,SBIDPtr> it(_itemsDisplayed);
    while(it.hasNext())
    {
        it.next();
        qDebug() << SB_DEBUG_INFO << it.value()->itemID() << it.value()->songPerformerName() << it.value()->songPerformerID();
    }

    _currentPtr=_itemsDisplayed[i.toInt()];
    qDebug() << SB_DEBUG_INFO << *_currentPtr;
    qDebug() << SB_DEBUG_INFO << "position=" << _currentPtr->albumPosition();

    this->close();
}

void
SBDialogSelectItem::init()
{
    ui=NULL;
    _hasSelectedItemFlag=0;
}
