#include <QDebug>
#include <QFile>
#include <QLabel>
#include <QPropertyAnimation>
#include <QRadioButton>

#include "CacheManager.h"
#include "Common.h"
#include "ExternalData.h"
#include "SBDialogSelectItem.h"
#include "SBIDAlbum.h"
#include "SBIDPerformer.h"
#include "SBIDOnlinePerformance.h"
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
    SBDialogSelectItem* dialog=new SBDialogSelectItem(existingAlbumPtr,parent);
    dialog->ui->setupUi(dialog);

    //	Populate choices
    QString title=QString("Select Album ");
    dialog->setTitle(title);
    dialog->ui->lHeader->setText(title+':');
    dialog->ui->lHeader->setFont(QFont("Trebuchet MS",13));

    for(int i=-2;i<matches.count(); i++)
    {
        int numMatches=(i<0?1:matches[i].count());	//	allow for newPerformerName to be shown

        for(int j=0;j<numMatches;j++)
        {
            QString imagePath;
            SBKey currentKey;
            QString currentAlbumTitle;
            QString currentAlbumPerformerName;

            QLabel* l;
            if(i==-2)
            {
                //	Process new performerName
                SBIDAlbumPtr tmpPtr=SBIDAlbum::retrieveUnknownAlbum();	//	only to get iconResourceLocation
                imagePath=tmpPtr->iconResourceLocation();
                //imagePath=SBIDBase::iconResourceLocation();
                currentKey=SBKey();
                currentAlbumTitle=newAlbum.albumTitle;
                currentAlbumPerformerName=newAlbum.performerName;
            }
            else if(i==-1)
            {
//                l=new QLabel;
//                l->setText("alt label");
//                d->ui->vlAlbumList->addWidget(l);
            }
            else
            {
                SBIDAlbumPtr currentAlbumPtr=matches[i][j];
                imagePath=ExternalData::getCachePath(currentAlbumPtr->key());
                QFile imageFile(imagePath);
                if(imageFile.exists()==0)
                {
                    imagePath=currentAlbumPtr->iconResourceLocation();
                }
                currentKey=currentAlbumPtr->key();
                currentAlbumTitle=currentAlbumPtr->albumTitle();
                currentAlbumPerformerName=currentAlbumPtr->albumPerformerName();
            }

            if(currentAlbumTitle.length() && currentAlbumPerformerName.length())
            {
                l=new QLabel;
                l->setWindowFlags(Qt::FramelessWindowHint);
                l->setTextFormat(Qt::RichText);
                l->setFont(QFont("Trebuchet MS",13));

                l->setText(QString("<html><head><style type=text/css> "
                                   "a:link {color:black; text-decoration:none;} "
                                   "</style></head><body><font face=\"Trebuchet MS\"><a href='%1'>&#8226;     <I>%2</I> by <B>%3</B></a></font></body></html>")
                           .arg(currentKey.toString())
                           .arg(currentAlbumTitle)
                           .arg(currentAlbumPerformerName)
                );

                l->setStyleSheet( ":hover{ background-color: darkgrey; }");
                connect(l, SIGNAL(linkActivated(QString)),
                        dialog, SLOT(OK(QString)));

                dialog->ui->vlAlbumList->addWidget(l);
            }
        }
    }
    dialog->updateGeometry();
    return dialog;
}

///
/// \brief SBDialogSelectItem::selectOnlinePerformanceFromSong
/// \param songPtr
/// \param allOPPtr
/// \param parent
/// \return
///
/// Intended to get an online performance for the given song. Used when assigning songs to playlist or to play/enqueue songs.
///
SBDialogSelectItem*
SBDialogSelectItem::selectOnlinePerformanceFromSong(const SBIDSongPtr& songPtr, QVector<SBIDOnlinePerformancePtr> allOPPtr, QWidget *parent)
{
    Q_UNUSED(songPtr);
    Q_UNUSED(parent);
    SBDialogSelectItem* dialog=new SBDialogSelectItem(songPtr,parent);
    dialog->ui->setupUi(dialog);

    //	Populate choices
    QString title=QString("Select Performance of %1%2%3:").arg(QChar(96)).arg(songPtr->songTitle()).arg(QChar(180));
    dialog->setTitle(title);
    dialog->ui->lHeader->setText(title);
    dialog->ui->lHeader->setFont(QFont("Trebuchet MS",13));

    QList<int> a;
    QListIterator<int> ita(a);
    while(ita.hasNext())
    {
        ita.next();
    }
    QVector<int> b;
    QListIterator<int> itb(b);
    while(itb.hasNext())
    {
    }

    QListIterator<SBIDOnlinePerformancePtr> allOPPtrIt(allOPPtr);
    while(allOPPtrIt.hasNext())
    {
        SBIDOnlinePerformancePtr opPtr=allOPPtrIt.next();

        //	CWIP: SBIDOnlinePerformance
        QLabel* l=new QLabel;
        SBIDAlbumPtr aPtr=opPtr->albumPtr();
        SBIDAlbumPerformancePtr apPtr=opPtr->albumPerformancePtr();
        QString albumPerformanceNotes="";
        QString albumNotes="";
        if(apPtr)
        {
            albumPerformanceNotes=apPtr->notes();
            if(albumPerformanceNotes.length()>0)
            {
                albumPerformanceNotes=QString("'%1' " ).arg(apPtr->notes());
            }
        }
        if(aPtr)
        {
            albumNotes=aPtr->notes();
            if(albumNotes.length()>0)
            {
                albumNotes=QString(" (%1)" ).arg(aPtr->notes());
            }
        }

        SB_DEBUG_IF_NULL(aPtr);

        l->setWindowFlags(Qt::FramelessWindowHint);
        l->setTextFormat(Qt::RichText);
        QString imagePath=ExternalData::getCachePath(aPtr->key());
        QFile imageFile(imagePath);

        if(imageFile.exists()==0)
        {
            imagePath=opPtr->iconResourceLocation();
        }
        l->setText(QString("<html><head><style type=text/css> "
                           "a:link {color:black; text-decoration:none;} "
                           "</style></head><body><a href='%2'><img align=\"MIDDLE\" src=\"%1\" width=\"50\">     %6by %4 on album '%3%7' (%5)</a></body></html>")
                   //	set args correctly
                   .arg(imagePath)
                   .arg(opPtr->key().toString())
                   .arg(aPtr->albumTitle())
                   .arg(opPtr->songPerformerName())
                   .arg(opPtr->duration().toString(SBDuration::sb_hhmmss_format))
                   .arg(albumPerformanceNotes)
                   .arg(albumNotes));

        l->setStyleSheet( ":hover{ background-color: darkgrey; }");
        connect(l, SIGNAL(linkActivated(QString)),
                dialog, SLOT(OK(QString)));

        dialog->ui->vlAlbumList->addWidget(l);
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
    QString title=QString("Select Correct Name for: '%1'").arg(newPerformerName);
    d->setTitle(title);
    title=QString("Select Correct Name for: <B>'%1'</B>").arg(newPerformerName);
    d->ui->lHeader->setText(title);
    d->ui->lHeader->setFont(QFont("Trebuchet MS",13));

    for(int i=-1;i<matches.count();i++)
    {
        int numMatches=(i==-1?1:matches[i].count());	//	allow for newPerformerName to be shown

        for(int j=0;j<numMatches;j++)
        {
            QString imagePath;
            SBKey currentKey;
            QString currentPerformerName;

            if(i==-1)
            {
                //	Process new performerName
                SBIDPerformerPtr tmpPtr=SBIDPerformer::retrieveVariousPerformers();
                imagePath=tmpPtr->iconResourceLocation();
                currentKey=SBKey();
                currentPerformerName=QString("Use as is: <B>%1</B>").arg(newPerformerName);
            }
            else
            {
                SBIDPerformerPtr currentPerformerPtr=matches[i][j];
                imagePath=ExternalData::getCachePath(currentPerformerPtr->key());
                QFile imageFile(imagePath);
                if(imageFile.exists()==0)
                {
                    imagePath=currentPerformerPtr->iconResourceLocation();
                }
                currentKey=currentPerformerPtr->key();
                currentPerformerName=QString("Existing Performer: <B>%1</B>").arg(currentPerformerPtr->performerName());
            }

            QLabel* l=new QLabel;
            l->setWindowFlags(Qt::FramelessWindowHint);
            l->setTextFormat(Qt::RichText);
            l->setFont(QFont("Trebuchet MS",13));
            l->setText(QString("<html><head><style type=text/css> "
                               "a:link {color:black; text-decoration:none;} "
                               "</style></head><body><font face=\"Trebuchet\"><a href='%2'><img align=\"MIDDLE\" src=\"%1\" width=\"50\">     %3</a></font></body></html>")
                       .arg(imagePath)
                       .arg(currentKey.toString())
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
    Q_UNUSED(parent);

    SBDialogSelectItem* d=new SBDialogSelectItem(existingSongPtr,parent);
    d->ui->setupUi(d);

    //	matches contains the following lists:
    //	0:	0 or 1 exact matches. If count==1, this method shouldn't be called,
    //		so ignore
    //	1:	0 or more exact matches on title, but with different performers.
    //	2:	0 or more soundex matches.
    bool foundExactMatchesFlag=(matches[1].count()>=1?1:0);
    bool foundSoundexMatchesFlag=(matches[2].count()>=1?1:0);

    //	Populate choices
    QString title;

    if(foundExactMatchesFlag)
    {
        title=QString("Select Original Performer for song <I>'%1'</I> performed by <B>'%2'</B>").arg(newSong.songTitle).arg(newSong.performerName);
    }
    else
    {
        title=QString("Select Original Performer for song <I>'%1'</I> performed by <B>'%2'</B>").arg(newSong.songTitle).arg(newSong.performerName);
    }
    d->setTitle("Select Original Performer");
    d->ui->lHeader->setText(title+':');
    d->ui->lHeader->setFont(QFont("Trebuchet MS",13));

    for(int i=0;i<matches.count(); i++)
    {
        int numMatches=(i==0?1:matches[i].count());

        for(int j=0;j<numMatches;j++)
        {
            QString imagePath;
            SBKey currentKey;
            QString currentSongTitle;
            QString currentSongPerformerName;

            QLabel* l=new QLabel;
            l->setWindowFlags(Qt::FramelessWindowHint);
            l->setTextFormat(Qt::RichText);
            l->setFont(QFont("Trebuchet MS",13));

            if(i==0)
            {
                currentKey=SBKey();
                currentSongPerformerName=newSong.performerName;

                l->setText(QString("<html><head><style type=text/css> "
                                   "a:link {color:black; text-decoration:none;} "
                                   "</style></head><body><font face=\"Trebuchet MS\"><a href='%1'>&#8226;     <B>%2</B></a></font></body></html>")
                           .arg(currentKey.toString())
                           .arg(currentSongPerformerName)
                );
            }
            else if(i==1)
            {
                SBIDSongPtr sPtr=matches[i][j];
                SBIDSongPerformancePtr spPtr=sPtr->originalSongPerformancePtr();
                SBIDPerformerPtr pPtr;

                if(spPtr)
                {
                    pPtr=spPtr->performerPtr();

                    imagePath=ExternalData::getCachePath(pPtr->key());
                    QFile imageFile(imagePath);
                    if(imageFile.exists()==0)
                    {
                        imagePath=pPtr->iconResourceLocation();
                    }
                    currentKey=sPtr->key();
                    currentSongPerformerName=pPtr->performerName();

                    l->setText(QString("<html><head><style type=text/css> "
                                       "a:link {color:black; text-decoration:none;} "
                                       "</style></head><body><font face=\"Trebuchet MS\"><a href='%1'>&#8226;     <B>%2</B></a></font></body></html>")
                               .arg(currentKey.toString())
                               .arg(currentSongPerformerName)
                    );
                }
                else
                {
                    qDebug() << SB_DEBUG_ERROR << "No performerPtr";
                }
            }
            else
            {
                if(j==0 && foundSoundexMatchesFlag)
                {
                    QLabel* l=new QLabel;
                    l->setText("Or, select one of the following songs below:");
                    l->setFont(QFont("Trebuchet MS",13));
                    d->ui->vlAlbumList->addWidget(l);
                }

                SBIDSongPtr sPtr=matches[i][j];
                imagePath=ExternalData::getCachePath(sPtr->key());
                QFile imageFile(imagePath);
                if(imageFile.exists()==0)
                {
                    imagePath=sPtr->iconResourceLocation();
                }
                currentKey=sPtr->key();
                currentSongTitle=sPtr->songTitle();
                currentSongPerformerName=sPtr->songOriginalPerformerName();

                l->setText(QString("<html><head><style type=text/css> "
                                   "a:link {color:black; text-decoration:none;} "
                                   "</style></head><body><font face=\"Trebuchet MS\"><a href='%1'>&#8226;     <I>%2</I> by <B>%3</B></a></font></body></html>")
                           .arg(currentKey.toString())
                           .arg(currentSongTitle)
                           .arg(currentSongPerformerName)
                );
            }

            l->setStyleSheet( ":hover{ background-color: darkgrey; }");
            connect(l, SIGNAL(linkActivated(QString)),
                    d, SLOT(OK(QString)));

            d->ui->vlAlbumList->addWidget(l);
        }
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
    if(i!="x:x")
    {
        _currentPtr=CacheManager::get(i.toLocal8Bit());
    }
    this->close();
}

void
SBDialogSelectItem::_init()
{
    _hasSelectedItemFlag=0;
}
