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
SBDialogSelectItem::SBDialogSelectItem(const SBIDPtr& ptr, QWidget *parent):
    QDialog(parent),
    ui(new Ui::SBDialogSelectItem),
    _currentPtr(ptr)
{
    _init();
}

void
SBDialogSelectItem::setTitle(const QString &title)
{

    setWindowTitle(title);
}

SBDialogSelectItem*
SBDialogSelectItem::selectAlbum(const Common::sb_parameters& newAlbum, const SBIDPtr& existingAlbumPtr, const QMap<int,QList<SBIDAlbumPtr>>& matches, QWidget *parent)
{
    SBDialogSelectItem* d=new SBDialogSelectItem(existingAlbumPtr,parent);
    d->ui->setupUi(d);

    //	Populate choices
    QString title=QString("Select album ");
    d->setTitle(title);
    d->ui->lHeader->setText(title+':');
    d->ui->lHeader->setFont(QFont("Trebuchet MS",13));

    for(int i=-2;i<matches.count(); i++)
    {
        int numMatches=(i<0?1:matches[i].count());	//	allow for newPerformerName to be shown

        for(int j=0;j<numMatches;j++)
        {
            QString imagePath;
            QString currentKey;
            QString currentAlbumTitle;
            QString currentAlbumPerformerName;

            QLabel* l;
            if(i==-2)
            {
                //	Process new performerName
                SBIDAlbumPtr tmpPtr=SBIDAlbum::retrieveUnknownAlbum();	//	only to get iconResourceLocation
                imagePath=tmpPtr->iconResourceLocation();
                currentKey="x:x";
                currentAlbumTitle=newAlbum.albumTitle;
                currentAlbumPerformerName=newAlbum.performerName;
            }
            if(i==-1)
            {
                l=new QLabel;
                l->setText("alt label");
                d->ui->vlAlbumList->addWidget(l);
            }
            else
            {
                SBIDAlbumPtr currentAlbumPtr=matches[i][j];
                imagePath=ExternalData::getCachePath(currentAlbumPtr);
                QFile imageFile(imagePath);
                if(imageFile.exists()==0)
                {
                    imagePath=currentAlbumPtr->iconResourceLocation();
                }
                currentKey=currentAlbumPtr->key();
                currentAlbumTitle=currentAlbumPtr->albumTitle();
                currentAlbumPerformerName=currentAlbumPtr->albumPerformerName();
            }

            l=new QLabel;
            l->setWindowFlags(Qt::FramelessWindowHint);
            l->setTextFormat(Qt::RichText);
            l->setFont(QFont("Trebuchet MS",13));

            l->setText(QString("<html><head><style type=text/css> "
                               "a:link {color:black; text-decoration:none;} "
                               "</style></head><body><font face=\"Trebuchet MS\"><a href='%1'>&#8226;     <I>%2</I> by <B>%3</B></a></font></body></html>")
                       .arg(currentKey)
                       .arg(currentAlbumTitle)
                       .arg(currentAlbumPerformerName)
            );

            l->setStyleSheet( ":hover{ background-color: darkgrey; }");
            connect(l, SIGNAL(linkActivated(QString)),
                    d, SLOT(OK(QString)));

            d->ui->vlAlbumList->addWidget(l);
        }
    }
    d->updateGeometry();
    return d;
}

SBDialogSelectItem*
SBDialogSelectItem::selectPerformanceFromSong(const SBIDSongPtr& songPtr, bool playableOnlyFlag, QWidget *parent)
{
    QVector<SBIDPerformancePtr> performanceList=songPtr->allPerformances();
    SBDialogSelectItem* dialog=new SBDialogSelectItem(songPtr,parent);
    dialog->ui->setupUi(dialog);

    //	Populate choices
    QString title=QString("Choose album and performer for song %1%2%3:").arg(QChar(96)).arg(songPtr->songTitle()).arg(QChar(180));
    dialog->setTitle(title);
    for(int i=0;i<performanceList.size();i++)
    {
        SBIDPerformancePtr currentPerformancePtr=performanceList.at(i);

        if(playableOnlyFlag==0 || (playableOnlyFlag==1 && currentPerformancePtr->path().length()>0))
        {
            QLabel* l=new QLabel;
            SBIDAlbumPtr currentAlbumPtr=SBIDAlbum::retrieveAlbum(currentPerformancePtr->albumID());

            l->setWindowFlags(Qt::FramelessWindowHint);
            l->setTextFormat(Qt::RichText);
            QString imagePath=ExternalData::getCachePath(currentAlbumPtr);
            QFile imageFile(imagePath);

            if(imageFile.exists()==0)
            {
                imagePath=songPtr->iconResourceLocation();
            }
            l->setText(QString("<html><head><style type=text/css> "
                               "a:link {color:black; text-decoration:none;} "
                               "</style></head><body><a href='%2'><img align=\"MIDDLE\" src=\"%1\" width=\"50\">     by %4 on album '%3' (%5)</a></body></html>")
                       //	set args correctly
                       .arg(imagePath)
                       .arg(currentPerformancePtr->key())
                       .arg(currentAlbumPtr->albumTitle())
                       .arg(currentPerformancePtr->songPerformerName())
                       .arg(currentPerformancePtr->duration().toString(Duration::sb_hhmmss_format)));

            l->setStyleSheet( ":hover{ background-color: darkgrey; }");
            connect(l, SIGNAL(linkActivated(QString)),
                    dialog, SLOT(OK(QString)));

            dialog->ui->vlAlbumList->addWidget(l);
        }
    }
    dialog->updateGeometry();
    return dialog;
}

SBDialogSelectItem*
SBDialogSelectItem::selectPerformer(const QString& newPerformerName,const SBIDPtr& existingPerformerPtr, const QMap<int,QList<SBIDPerformerPtr>>& matches, QWidget *parent)
{
    //	Used by MusicLibrary to import songs

    SBDialogSelectItem* d=new SBDialogSelectItem(existingPerformerPtr,parent);
    d->ui->setupUi(d);

    //	Populate choices
    QString title=QString("Choose Performer ");
    d->setTitle(title);
    d->ui->lHeader->setText(title);
    d->ui->lHeader->setFont(QFont("Trebuchet MS",13));

    for(int i=-1;i<matches.count();i++)
    {
        int numMatches=(i==-1?1:matches[i].count());	//	allow for newPerformerName to be shown

        for(int j=0;j<numMatches;j++)
        {
            QString imagePath;
            QString currentKey;
            QString currentPerformerName;

            if(i==-1)
            {
                //	Process new performerName
                SBIDPerformerPtr tmpPtr=SBIDPerformer::retrieveVariousArtists();
                imagePath=tmpPtr->iconResourceLocation();
                currentKey="x:x";
                currentPerformerName=newPerformerName;
            }
            else
            {
                SBIDPerformerPtr currentPerformerPtr=matches[i][j];
                imagePath=ExternalData::getCachePath(currentPerformerPtr);
                QFile imageFile(imagePath);
                if(imageFile.exists()==0)
                {
                    imagePath=currentPerformerPtr->iconResourceLocation();
                }
                currentKey=currentPerformerPtr->key();
                currentPerformerName=currentPerformerPtr->performerName();
            }
            qDebug() << SB_DEBUG_INFO << i << j << currentKey << currentPerformerName;

            QLabel* l=new QLabel;
            l->setWindowFlags(Qt::FramelessWindowHint);
            l->setTextFormat(Qt::RichText);
            l->setFont(QFont("Trebuchet MS",13));
            l->setText(QString("<html><head><style type=text/css> "
                               "a:link {color:black; text-decoration:none;} "
                               "</style></head><body><font face=\"Trebuchet\"><a href='%2'><img align=\"MIDDLE\" src=\"%1\" width=\"50\">     %3</a></font></body></html>")
                       .arg(imagePath)
                       .arg(currentKey)
                       .arg(currentPerformerName)
            );

            l->setStyleSheet( ":hover{ background-color: darkgrey; }");
            connect(l, SIGNAL(linkActivated(QString)),
                    d, SLOT(OK(QString)));

            d->ui->vlAlbumList->addWidget(l);
        }
    }
    d->updateGeometry();
    return d;
}

SBDialogSelectItem*
SBDialogSelectItem::selectSong(const Common::sb_parameters& newSong,const SBIDPtr& existingSongPtr, const QMap<int,QList<SBIDSongPtr>>& matches, QWidget *parent)
{
    SBDialogSelectItem* d=new SBDialogSelectItem(existingSongPtr,parent);
    d->ui->setupUi(d);

    //	Populate choices
    QString title=QString("Select song ");
    d->setTitle(title);
    d->ui->lHeader->setText(title+':');
    d->ui->lHeader->setFont(QFont("Trebuchet MS",13));

    for(int i=-2;i<matches.count(); i++)
    {
        int numMatches=(i<0?1:matches[i].count());	//	allow for newPerformerName to be shown

        for(int j=0;j<numMatches;j++)
        {
            QString imagePath;
            QString currentKey;
            QString currentSongTitle;
            QString currentSongPerformerName;

            QLabel* l;
            if(i==-2)
            {
                qDebug() << SB_DEBUG_INFO << newSong.songTitle << newSong.performerName;
                //	Process new performerName
                imagePath=SBIDSong::iconResourceLocationStatic();
                currentKey="x:x";
                currentSongTitle=newSong.songTitle;
                currentSongPerformerName=newSong.performerName;
            }
            else if(i==-1)
            {
                l=new QLabel;
                l->setText("alt label");
                d->ui->vlAlbumList->addWidget(l);
            }
            else
            {
                qDebug() << SB_DEBUG_INFO << i << j;
                SBIDSongPtr currentSongPtr=matches[i][j];
                imagePath=ExternalData::getCachePath(currentSongPtr);
                QFile imageFile(imagePath);
                if(imageFile.exists()==0)
                {
                    imagePath=currentSongPtr->iconResourceLocation();
                }
                currentKey=currentSongPtr->key();
                currentSongTitle=currentSongPtr->songTitle();
                currentSongPerformerName=currentSongPtr->songPerformerName();
            }

            l=new QLabel;
            l->setWindowFlags(Qt::FramelessWindowHint);
            l->setTextFormat(Qt::RichText);
            l->setFont(QFont("Trebuchet MS",13));

            l->setText(QString("<html><head><style type=text/css> "
                               "a:link {color:black; text-decoration:none;} "
                               "</style></head><body><font face=\"Trebuchet MS\"><a href='%1'>&#8226;     <I>%2</I> by <B>%3</B></a></font></body></html>")
                       .arg(currentKey)
                       .arg(currentSongTitle)
                       .arg(currentSongPerformerName)
            );

            l->setStyleSheet( ":hover{ background-color: darkgrey; }");
            connect(l, SIGNAL(linkActivated(QString)),
                    d, SLOT(OK(QString)));

            d->ui->vlAlbumList->addWidget(l);
        }
    }
    d->updateGeometry();
    return d;
}

//SBDialogSelectItem*
//SBDialogSelectItem::selectSongByPerformer(const SBIDSongPtr& songPtr, const QString& newPerformerName, QWidget *parent)
//{
//    SBDialogSelectItem* d=new SBDialogSelectItem(songPtr,parent);
//    d->ui->setupUi(d);

//    //	Populate choices
//    //int lastSeenRank=0;
//    //int currentRank=0;
//    QString title=QString("Who is the original performer");
//    d->setTitle(title+"?");
//    title="<FONT SIZE+=1><B>"+title+" for </B></FONT><B><I><FONT SIZE=+1>"+songPtr->songTitle()+"</FONT></I></B>";
//    d->ui->lHeader->setText(title+':');
//    d->ui->lHeader->setFont(QFont("Trebuchet MS",13));
//    QVector<int> performers=songPtr->performerIDList();
//    SBIDPerformerPtr performerPtr;

//    //	CWIP: uncommented out. Need to create data structure for performances

////    for(int i=0;performers.size();i++)
////    {
////        performerPtr=pemgr->retrieve(performers.at(i));

////        //currentRank=m->data(m->index(i,0)).toInt();
////        //currentSong.setSongTitle(m->data(m->index(i,2)).toString());
////        //currentSong.setSongPerformerID(m->data(m->index(i,3)).toInt());
////        //currentSong.setSongPerformerName(m->data(m->index(i,4)).toString());

////            if(currentRank!=lastSeenRank && currentRank==3)
////            {
////                QLabel* lh=new QLabel;
////                lh->setWindowFlags(Qt::FramelessWindowHint);
////                lh->setTextFormat(Qt::RichText);
////                lh->setText("<FONT SIZE+=1><B>Other Choices:</B></FONT>");
////                lh->setFont(QFont("Trebuchet MS",13));
////                d->ui->vlAlbumList->addWidget(lh);
////            }

////            QLabel* l=new QLabel;
////            l->setWindowFlags(Qt::FramelessWindowHint);
////            l->setTextFormat(Qt::RichText);
////            l->setFont(QFont("Trebuchet MS",13));

////            switch(currentRank)
////            {
////            case 0:
////                l->setText(QString("<html><head><style type=text/css> "
////                                   "a:link {color:black; text-decoration:none;} "
////                                   "</style></head><body><font face=\"Trebuchet MS\"><a href='%1'>&#8226;     Is <I>%2</I> a <B>%3</B> original, or </a></font></body></html>")
////                           .arg(i)
////                           .arg(currentSongPtr->songTitle())
////                           .arg(newPerformerName);
////                );
////                break;
////            case 1:
////                l->setText(QString("<html><head><style type=text/css> "
////                                   "a:link {color:black; text-decoration:none;} "
////                                   "</style></head><body><font face=\"Trebuchet MS\"><a href='%1'>&#8226;     Is this a <B>%4</B> cover of the <B>%3</B> song <I>%2</I>?</a></font></body></html>")
////                           .arg(i)
////                           .arg(currentSongPtr->songTitle())
////                           .arg(performerPtr->performerName())
////                           .arg(newPerformerName)
////                );
////                break;

////            case 2:
////                l->setText(QString("<html><head><style type=text/css> "
////                                   "a:link {color:black; text-decoration:none;} "
////                                   "</style></head><body><font face=\"Trebuchet MS\"><a href='%1'>&#8226;     Is this a <B>%4</B> cover of the <B>%3</B> song <I>%2</I>?</a></font></body></html>")
////                           .arg(i)
////                           .arg(currentSongPtr->songTitle())
////                           .arg(performerPtr->performerName())
////                           .arg(newPerformerName)
////                );
////                break;

////            case 3:
////                l->setText(QString("<html><head><style type=text/css> "
////                                   "a:link {color:black; text-decoration:none;} "
////                                   "</style></head><body><font face=\"Trebuchet MS\"><a href='%1'>&#8226;     <I>%2</I>(3) by <B>%3</B></a></font></body></html>")
////                           .arg(i)
////                           .arg(currentSongPtr->songTitle())
////                           .arg(performerPtr->performerName())
////                );
////            default:
////                break;

////            }

////            l->setStyleSheet( ":hover{ background-color: darkgrey; }");
////            connect(l, SIGNAL(linkActivated(QString)),
////                    d, SLOT(OK(QString)));

////            d->ui->vlAlbumList->addWidget(l);

////            songsShown.append(currentSong);
////        lastSeenRank=currentRank;
////    }
//    d->updateGeometry();
//    return d;
//}

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
    if(i!="x:x")
    {
        _currentPtr=SBIDBase::createPtr(i);
    }
    this->close();
}

void
SBDialogSelectItem::_init()
{
    _hasSelectedItemFlag=0;
}
