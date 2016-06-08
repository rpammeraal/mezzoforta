#include <QCoreApplication>
#include <QDebug>
#include <QMimeData>
#include <QSqlRecord>

#include "Common.h"
#include "Context.h"
#include "SBID.h"
#include "DataEntityCurrentPlaylist.h"
#include "MainWindow.h"
#include "SBSqlQueryModel.h"
#include "SBModelCurrentPlaylist.h"
//#include "SBStandardItem.h"

SBModelCurrentPlaylist::SBModelCurrentPlaylist(QObject* parent):QStandardItemModel(parent)
{
    qDebug() << SB_DEBUG_INFO;
    qDebug() << SB_DEBUG_INFO << "++++++++++++++++++++++++++++++++++++++";
    _currentPlayID=-1;
}

//	Methods related to drag&drop
bool
SBModelCurrentPlaylist::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(action);
    qDebug() << SB_DEBUG_INFO << row << column << parent;
    if(row==-1 && column==-1)
    {
        qDebug() << SB_DEBUG_INFO << "ABORTED DROP";
        return false;
    }
    if(column>=0)
    {
        //	Always make sure that we don't create extra columns
        column=-1;
    }

    //	Populate record
    QList<QStandardItem *> newRow;
    QByteArray encodedData = data->data("application/vnd.text.list");
    QDataStream ds(&encodedData, QIODevice::ReadOnly);
    QStandardItem* it;
    QString value;
    for(int i=0;i<this->columnCount();i++)
    {
        ds >> value;
        if(value.length()==0)
        {
            value="not set!";
        }
        it=new QStandardItem();
        switch(i)
        {
        case sb_column_deleteflag:
            value="0";
            break;

        default:
            break;
        }


        it->setText(value);
        qDebug() << SB_DEBUG_INFO << i << value;
        newRow.append(it);
    }

    //	Add record
    this->insertRow(row,newRow);

    return true;
}


Qt::ItemFlags
SBModelCurrentPlaylist::flags(const QModelIndex &index) const
{
    if(index.isValid())
    {
        if(index.column()>=sb_column_startofdata)
        {
            return Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        }
        return Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
    return Qt::ItemIsDropEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QMimeData*
SBModelCurrentPlaylist::mimeData(const QModelIndexList & indexes) const
{
    foreach (const QModelIndex &idx, indexes)
    {
        if (idx.isValid())
        {
            QMimeData* mimeData = new QMimeData();
            QByteArray ba;
            QDataStream ds(&ba, QIODevice::WriteOnly);

            QStandardItem* it;

            for(int i=0;i<this->columnCount();i++)
            {
                it=this->item(idx.row(),i);
                {
                    ds << it->text();
                }
            }

            mimeData->setData("application/vnd.text.list", ba);
            return mimeData;
        }
    }
    return NULL;
}

QStringList
SBModelCurrentPlaylist::mimeTypes() const
{
    QStringList types;
    types << "application/vnd.text.list";
    return types;
}

Qt::DropActions
SBModelCurrentPlaylist::supportedDropActions() const
{
    return Qt::MoveAction;
}

//	Methods unrelated to drag&drop
QModelIndex
SBModelCurrentPlaylist::addRow()
{
    qDebug() << SB_DEBUG_INFO;
    QList<QStandardItem *>column;
    QStandardItem* item;
    int newRowID=this->rowCount()+1;
    QString idStr;
    idStr.sprintf("%08d",newRowID);

    item=new QStandardItem("0"); column.append(item);                         //	sb_column_deleteflag
    item=new QStandardItem(""); column.append(item);                          //	sb_column_playflag
    item=new QStandardItem("0"); column.append(item);                         //	sb_column_albumid
    item=new QStandardItem(QString("%1").arg(newRowID)); column.append(item); //	sb_column_displayplaylistpositionid
    item=new QStandardItem("0"); column.append(item);                         //	sb_column_songid
    item=new QStandardItem("0"); column.append(item);                         //	sb_column_performerid
    item=new QStandardItem(idStr); column.append(item);                    //	sb_column_playlistpositionid
    item=new QStandardItem("0"); column.append(item);                         //	sb_column_position
    item=new QStandardItem("0"); column.append(item);                         //	sb_column_path
    item=new QStandardItem("Title"); column.append(item);                     //	sb_column_songtitle
    item=new QStandardItem("Duration"); column.append(item);                  //	sb_column_duration
    item=new QStandardItem("Performer"); column.append(item);                 //	sb_column_performername
    item=new QStandardItem("Album"); column.append(item);                     //	sb_column_albumtitle
    this->appendRow(column); column.clear();

    return this->createIndex(newRowID-1,0);
}

void
SBModelCurrentPlaylist::clear()
{
    _currentPlayID=-1;
    QStandardItemModel::clear();
}

QString
SBModelCurrentPlaylist::formatDisplayPlayID(int playID,bool isCurrent)
{
    QString str;

    //str.sprintf("%s %d",(isCurrent?">":"   "),playID);
    str.sprintf("%s %d",(isCurrent?" ":"   "),playID);
    return str;
}

void
SBModelCurrentPlaylist::paintRow(int i)
{
    if(i<0 || i>=this->rowCount())
    {
        qDebug() << SB_DEBUG_INFO << "noop";
        return;
    }
    const QColor normalColor(SB_VIEW_BG_COLOR);
    const QColor altColor(SB_VIEW_BG_ALT_COLOR);

    QColor newColor=(i%2?altColor:normalColor);

    int playlistID=-1;
    QStandardItem* item=NULL;

    for(int j=0;j<this->columnCount();j++)
    {
        item=this->item(i,SBModelCurrentPlaylist::sb_column_playlistpositionid);
        if(item)
        {
            playlistID=item->text().toInt();

            item=this->item(i,j);
            if(item)
            {
                if(j==SBModelCurrentPlaylist::sb_column_displayplaylistpositionid)
                {
                    item->setBackground(QBrush(newColor));
                }
                QFont f=item->font();
                f.setItalic(playlistID-1==_currentPlayID?1:0);
                item->setFont(f);
            }
        }
    }
}

void
SBModelCurrentPlaylist::sort(int column, Qt::SortOrder order)
{
    qDebug() << SB_DEBUG_INFO;
    if(column==sb_column_displayplaylistpositionid)
    {
        qDebug() << SB_DEBUG_INFO;
        column=sb_column_playlistpositionid;
    }
    QStandardItemModel::sort(column,order);
}

//	Methods related to playlists
int
SBModelCurrentPlaylist::playlistCount() const
{
    return this->rowCount();
}

SBID
SBModelCurrentPlaylist::getNextSong(bool previousFlag)
{
    SBID song;
    int newPlayID=_currentPlayID+(previousFlag==1?-1:1);
    qDebug() << SB_DEBUG_INFO
             << "_currentPlayID=" << _currentPlayID
             << "newPlayID=" << newPlayID
    ;
    if(newPlayID<0)
    {
        newPlayID=0;
    }
    else if(newPlayID>=this->playlistCount())
    {
        newPlayID=this->playlistCount()-1;
    }
    song=getSongFromPlaylist(newPlayID);
    song.playPosition=newPlayID;
    return song;
}

SBID
SBModelCurrentPlaylist::getSongFromPlaylist(int playlistIndex)
{
    QStandardItem* item;
    SBID song;
    int songID=0;

    if(playlistIndex<0 || playlistIndex>=playlistCount())
    {
        return song;
    }

    //	Find song first by playlistIndex.
    for(int internalPlaylistIndex=0;internalPlaylistIndex<this->rowCount();internalPlaylistIndex++)
    {
        item=this->item(internalPlaylistIndex,SBModelCurrentPlaylist::sb_column_playlistpositionid);
        if(item)
        {
            if(item->text().toInt()==playlistIndex+1)
            {
                //	sb_column_songid
                item=this->item(internalPlaylistIndex,SBModelCurrentPlaylist::sb_column_songid);
                if(item)
                {
                    songID=item->text().toInt();
                    song.assign(SBID::sb_type_song,songID);
                }

                //	sb_column_albumid
                item=this->item(internalPlaylistIndex,SBModelCurrentPlaylist::sb_column_albumid);
                if(item)
                {
                    song.sb_album_id=item->text().toInt();
                }

                //	sb_column_performerid
                item=this->item(internalPlaylistIndex,SBModelCurrentPlaylist::sb_column_performerid);
                if(item)
                {
                    song.sb_performer_id=item->text().toInt();
                }

                //	sb_column_position
                item=this->item(internalPlaylistIndex,SBModelCurrentPlaylist::sb_column_position);
                if(item)
                {
                    song.sb_position=item->text().toInt();
                }

                //	sb_column_path
                item=this->item(internalPlaylistIndex,SBModelCurrentPlaylist::sb_column_path);
                if(item)
                {
                    song.path=item->text();
                }

                //	sb_column_songtitle
                item=this->item(internalPlaylistIndex,SBModelCurrentPlaylist::sb_column_songtitle);
                if(item)
                {
                    song.songTitle=item->text();
                }

                //	sb_column_performername
                item=this->item(internalPlaylistIndex,SBModelCurrentPlaylist::sb_column_performername);
                if(item)
                {
                    song.performerName=item->text();
                }

                //	sb_column_albumtitle
                item=this->item(internalPlaylistIndex,SBModelCurrentPlaylist::sb_column_albumtitle);
                if(item)
                {
                    song.albumTitle=item->text();
                }

                //	playlistPosition
                song.playPosition=_currentPlayID;
            }
        }
    }

    return song;
}

void
SBModelCurrentPlaylist::resetCurrentPlayID()
{
    setCurrentSongByID(-1);
    _currentPlayID=-1;
}

int
SBModelCurrentPlaylist::currentPlaylistIndex() const
{
    return _currentPlayID;
}

///
/// \brief populate
/// \param newPlaylist
/// \param firstBatchHasLoadedFlag: provides a way to split loading in two batches:
/// 	-	first batch (a small set of records) is immediately loaded, displayed and music starts to play
///		-	second batch (with the remainder) is loaded while playing music.
void
SBModelCurrentPlaylist::populate(QMap<int,SBID> newPlaylist,bool firstBatchHasLoadedFlag)
{
    qDebug() << SB_DEBUG_INFO << "newPlaylist.count()=" << newPlaylist.count();
    int offset=0;

    if(!firstBatchHasLoadedFlag)
    {
        qDebug() << SB_DEBUG_INFO;
        this->clear();
        qDebug() << SB_DEBUG_INFO;
    }
    else
    {
        qDebug() << SB_DEBUG_INFO << "Appending at position " << offset;
        //	Set the index to get the first record from to count().
        //	This works in data structures where the index is 0-based.
        offset=this->rowCount();
    }
    QList<QStandardItem *>column;
    QStandardItem* item;

    for(int i=offset;i<newPlaylist.count();i++)
    {
        if(_currentPlayID==-1)
        {
            _currentPlayID=0;	//	now that we have at least one entry, set current song to play to 0.
        }
        SBID song=newPlaylist[i];
//        if(firstBatchHasLoadedFlag)
//        {
//            qDebug() << SB_DEBUG_INFO << song;
//        }
        QString idStr;
        idStr.sprintf("%08d",i+1);

        item=new QStandardItem("0"); column.append(item);                                       //	sb_column_deleteflag
        item=new QStandardItem(""); column.append(item);                                        //	sb_column_playflag
        item=new QStandardItem(QString("%1").arg(song.sb_album_id)); column.append(item);       //	sb_column_albumid
        item=new QStandardItem(formatDisplayPlayID(i+1)); column.append(item);                  //	sb_column_displayplaylistpositionid
        item->setData(Qt::AlignRight, Qt::TextAlignmentRole);

        item=new QStandardItem(QString("%1").arg(song.sb_song_id)); column.append(item);        //	sb_column_songid
        item=new QStandardItem(QString("%1").arg(song.sb_performer_id)); column.append(item);	//	sb_column_performerid
        item=new QStandardItem(idStr); column.append(item);                                     //	sb_column_playlistpositionid
        item=new QStandardItem(QString("%1").arg(song.sb_position)); column.append(item);       //	sb_column_position
        item=new QStandardItem(song.path); column.append(item);                                 //	sb_column_path

        item=new QStandardItem(song.songTitle); column.append(item);                            //	sb_column_songtitle
        item=new QStandardItem(song.duration.toString()); column.append(item);                  //	sb_column_duration
        item=new QStandardItem(song.performerName); column.append(item);                        //	sb_column_performername
        item=new QStandardItem(song.albumTitle); column.append(item);                           //	sb_column_albumtitle
        this->appendRow(column); column.clear();

        QCoreApplication::processEvents();

    }
    if(!firstBatchHasLoadedFlag)
    {
        populateHeader();
        setCurrentSongByID(_currentPlayID);
    }
    qDebug() << SB_DEBUG_INFO << "newPlaylist.count()" << newPlaylist.count();
    qDebug() << SB_DEBUG_INFO << "rowCount.count()" << this->rowCount();
}

void
SBModelCurrentPlaylist::populateHeader()
{
    QList<QStandardItem *>column;
    QStandardItem* item;

    int columnIndex=0;
    item=new QStandardItem("DEL"); this->setHorizontalHeaderItem(columnIndex++,item);        //	sb_column_deleteflag
    item=new QStandardItem(""); this->setHorizontalHeaderItem(columnIndex++,item);           // sb_column_playflag
    item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);          //	sb_column_albumid
    item=new QStandardItem(""); this->setHorizontalHeaderItem(columnIndex++,item);           //	sb_column_displayplaylistpositionid
    item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);          //	sb_column_songid
    item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);          //	sb_column_performerid
    item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);          //	sb_column_playlistpositionid
    item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);          //	sb_column_position
    item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);          //	sb_column_path
    item=new QStandardItem("Song"); this->setHorizontalHeaderItem(columnIndex++,item);       //	sb_column_songtitle
    item=new QStandardItem("Duration"); this->setHorizontalHeaderItem(columnIndex++,item);   //	sb_column_duration
    item=new QStandardItem("Performer"); this->setHorizontalHeaderItem(columnIndex++,item);  //	sb_column_performername
    item=new QStandardItem("Album"); this->setHorizontalHeaderItem(columnIndex++,item);      //	sb_column_albumtitle
}

void
SBModelCurrentPlaylist::reorderItems()
{
    QMap<int,int> toFrom;	//	map from old to new index (0-based)
    //	Create map

    //	Reset first column
    for(int i=0;i<this->rowCount();i++)
    {
        int playID=-1;
        QStandardItem* item;

        item=this->item(i,sb_column_playlistpositionid);
        if(item!=NULL)
        {
            item=this->item(i,sb_column_playlistpositionid);
            playID=item->text().toInt();


            item->setText(QString("%1").arg(i+1));
            toFrom[playID-1]=i;
        }

        item=this->item(i,sb_column_displayplaylistpositionid);
        if(item!=NULL)
        {
            item->setText(formatDisplayPlayID(i+1));
            item->setData(Qt::AlignRight, Qt::TextAlignmentRole);
        }
        paintRow(i);
    }
    _currentPlayID=toFrom[_currentPlayID];

    setCurrentSongByID(_currentPlayID);
}

bool
SBModelCurrentPlaylist::removeRows(int row, int count, const QModelIndex &parent)
{
    qDebug() << SB_DEBUG_INFO << row << count << parent.row() << parent.column();
    bool result=QStandardItemModel::removeRows(row,count,parent);
    this->reorderItems();
    return result;
}

void
SBModelCurrentPlaylist::repaintAll()
{
    for(int i=0;i<this->rowCount();i++)
    {
        paintRow(i);
    }
}


///
/// \brief SBModelCurrentPlaylist::setCurrentSongByID
/// \param playID
/// \return
///
///	setCurrentSongByID() returns tableView row that is current.
/// It also sets/unsets the indicator to the current song.
/// This is the *ONLY* function that may assign a new value
/// to _currentPlayID.
QModelIndex
SBModelCurrentPlaylist::setCurrentSongByID(int playID)
{
    qDebug() << SB_DEBUG_INFO << _currentPlayID << playID;
    QStandardItem* item=NULL;
    int oldRowID=-1;
    int newRowID=-1;

    //	Mark current song in UI as not being played.
    if(_currentPlayID<this->rowCount() && _currentPlayID>=0)
    {
        //	Don't execute if, if _currentPlayID:
        //	-	greater than rowcount, or
        //	-	not set (-1), or
        for(int i=0;oldRowID==-1 && i<this->rowCount();i++)
        {
            item=this->item(i,sb_column_playlistpositionid);

            if(item->text().toInt()-1==_currentPlayID)
            {
                //	Found item that is pointing to by _currentPlayID
                item=this->item(i,sb_column_displayplaylistpositionid);
                if(item!=NULL)
                {
                    item->setText(formatDisplayPlayID(_currentPlayID+1));
                    item->setData(Qt::AlignRight, Qt::TextAlignmentRole);
                    oldRowID=i;
                    item->setIcon(QIcon());
                }

                item=this->item(i,sb_column_playflag);
                if(item!=NULL)
                {
                    item->setIcon(QIcon());
                }
            }
        }
    }

    //	Now mark the new song in UI as being played.
    if(playID<this->rowCount() && playID>=0)
    {
        _currentPlayID=playID;

        for(int i=0;newRowID==-1 && i<this->rowCount();i++)
        {
            item=this->item(i,sb_column_playlistpositionid);

            if(item->text().toInt()-1==_currentPlayID)
            {
                item=this->item(i,sb_column_displayplaylistpositionid);
                if(item!=NULL)
                {
                    item->setText(formatDisplayPlayID(_currentPlayID+1,1));
                    item->setData(Qt::AlignRight, Qt::TextAlignmentRole);
                    paintRow(i);
                    newRowID=i;
                    item->setIcon(QIcon(":/images/playing.png"));
                }

                item=this->item(i,sb_column_playflag);
                if(item!=NULL)
                {
                    item->setIcon(QIcon(":/images/playing.png"));
                }
            }
        }
    }
    if(oldRowID>=0)
    {
        paintRow(oldRowID);
    }
    return this->index(newRowID,sb_column_displayplaylistpositionid);
}

void
SBModelCurrentPlaylist::shuffle()
{
    QList<int> usedIndex; //	list of already used random numbers
    QMap<int,int> fromTo; //	map from old to new index (1-based)
    int index=1;          //	1-based

    qDebug() << SB_DEBUG_INFO << _currentPlayID;
    //	Assign current playing to first spot
    if(_currentPlayID>=0 && _currentPlayID<this->rowCount())
    {
        qDebug() << SB_DEBUG_INFO << "from" << index << "to" << _currentPlayID+1;
        fromTo[index++]=(_currentPlayID+1);
        usedIndex.append(_currentPlayID+1);
    }

    //	Create fromTo mapping
    while(index<=this->rowCount())
    {
        int randomIndex=-1;
        while(randomIndex==-1)
        {
            randomIndex=Common::random(this->rowCount());
            randomIndex++;
            if(usedIndex.contains(randomIndex))
            {
                randomIndex=-1;
            }
        }
        qDebug() << SB_DEBUG_INFO << "from" << index << "to" << randomIndex;
        fromTo[index++]=(randomIndex);
        usedIndex.append(randomIndex);
    }

    //	Assign
    qDebug() << SB_DEBUG_INFO;
    for(int i=0;i<this->rowCount();i++)
    {
        //	Iterate using a 0-based iterator, fromTo is 1-based.
        //qDebug() << SB_DEBUG_INFO << "from" << i+1 << "to" << fromTo[i+1];

        int playID=-1;	//	playID:1-based
        QStandardItem* item;

        item=this->item(i,sb_column_playlistpositionid);
        if(item!=NULL)
        {
            item=this->item(i,sb_column_playlistpositionid);
            playID=item->text().toInt();

            //	Pad up to 10 chars so that view will sort correctly.
            item->setText(QString("%1").arg(fromTo[playID],10,10,QChar('0')));
        }

        item=this->item(i,sb_column_displayplaylistpositionid);
        if(item!=NULL)
        {
            item->setText(formatDisplayPlayID(fromTo[playID]));
            item->setData(Qt::AlignRight, Qt::TextAlignmentRole);
        }
    }
    qDebug() << SB_DEBUG_INFO << "currentPlayID:before=" << _currentPlayID;
    _currentPlayID=fromTo[_currentPlayID+1]-1;
    qDebug() << SB_DEBUG_INFO << "currentPlayID:new=" << _currentPlayID;
    setCurrentSongByID(_currentPlayID);
}

///	Debugging
void
SBModelCurrentPlaylist::debugShow(const QString& title)
{
    qDebug() << SB_DEBUG_INFO << title;
    for(int i=0;i<this->rowCount();i++)
    {
        QString row=QString("row=%1").arg(i);
        for(int j=0;j<this->columnCount();j++)
        {
            if(j!=sb_column_path)
            {
                QStandardItem* item=this->item(i,j);
                row+=QString("|c[%1]=").arg(j);
                if(item)
                {
                    row+="'"+item->text()+"'";
                }
                else
                {
                    row+="<NULL>";
                }
            }
        }
        qDebug() << SB_DEBUG_INFO << row;
    }
}
