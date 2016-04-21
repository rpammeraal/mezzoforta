#include <QCoreApplication>
#include <QDebug>
#include <QMimeData>
#include <QSqlRecord>

#include "Common.h"
#include "Context.h"
#include "PlayerController.h"
#include "SBID.h"
#include "DataEntityCurrentPlaylist.h"
#include "SBSqlQueryModel.h"

#include "SBCurrentPlaylistModel.h"


SBCurrentPlaylistModel::SBCurrentPlaylistModel(QObject* parent):QStandardItemModel(parent)
{
    qDebug() << SB_DEBUG_INFO;
    qDebug() << SB_DEBUG_INFO << "++++++++++++++++++++++++++++++++++++++";
    _currentPlayID=-1;
}

//	Methods related to drag&drop
bool
SBCurrentPlaylistModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
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
SBCurrentPlaylistModel::flags(const QModelIndex &index) const
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
SBCurrentPlaylistModel::mimeData(const QModelIndexList & indexes) const
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
SBCurrentPlaylistModel::mimeTypes() const
{
    QStringList types;
    types << "application/vnd.text.list";
    return types;
}

Qt::DropActions
SBCurrentPlaylistModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

//	Methods unrelated to drag&drop
QModelIndex
SBCurrentPlaylistModel::addRow()
{
    qDebug() << SB_DEBUG_INFO;
    QList<QStandardItem *>column;
    QStandardItem* item;
    int newRowID=this->rowCount()+1;
    QString idStr;
    idStr.sprintf("%08d",newRowID);

    item=new QStandardItem("0"); column.append(item);                         //	sb_column_deletedflag
    item=new QStandardItem("0"); column.append(item);                         //	sb_column_albumid
    item=new QStandardItem(QString("%1").arg(newRowID)); column.append(item); //	sb_column_displayplaylistid
    item=new QStandardItem("0"); column.append(item);                         //	sb_column_songid
    item=new QStandardItem("0"); column.append(item);                         //	sb_column_performerid
    item=new QStandardItem(idStr); column.append(item);                       //	sb_column_playlistid
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
SBCurrentPlaylistModel::clear()
{
    _currentPlayID=-1;
    QStandardItemModel::clear();
}

QString
SBCurrentPlaylistModel::formatDisplayPlayID(int playID,bool isCurrent)
{
    QString str;

    str.sprintf("%s %d",(isCurrent?">":"   "),playID);
    return str;
}

void
SBCurrentPlaylistModel::paintRow(int i)
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
        item=this->item(i,SBCurrentPlaylistModel::sb_column_playlistid);
        if(item)
        {
            playlistID=item->text().toInt();

            item=this->item(i,j);
            if(item)
            {
                if(j==SBCurrentPlaylistModel::sb_column_displayplaylistid)
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


///
/// \brief populate
/// \return
///
///	This populate() is called when the playlist is taken from the current_playlist table.
///	It'll also create a playlist (to be consumed by PlayerController).
///
QMap<int,SBID>
SBCurrentPlaylistModel::populate()
{
    qDebug() << SB_DEBUG_INFO << _currentPlayID;
    _currentPlayID=-1;
    SBSqlQueryModel* qm=DataEntityCurrentPlaylist::getAllSongs();
    QList<QStandardItem *>column;
    QStandardItem* item;
    QMap<int,SBID> playlist;

    for(int i=0;i<qm->rowCount();i++)
    {
        QString idStr;
        SBID id=SBID(SBID::sb_type_song,qm->record(i).value(2).toInt());
        idStr.sprintf("%08d",i+1);

        item=new QStandardItem("0"); column.append(item);                                 //	sb_column_deleteflag
        item=new QStandardItem(qm->record(i).value(8).toString()); column.append(item);   //	sb_column_albumid
        id.sb_album_id=qm->record(i).value(8).toInt();
        item=new QStandardItem(formatDisplayPlayID(i+1)); column.append(item);            //	sb_column_displayplaylistid
        item->setData(Qt::AlignRight, Qt::TextAlignmentRole);

        item=new QStandardItem(qm->record(i).value(2).toString()); column.append(item);   //	sb_column_songid
        item=new QStandardItem(qm->record(i).value(5).toString()); column.append(item);   //	sb_column_performerid
        id.sb_performer_id=qm->record(i).value(5).toInt();
        item=new QStandardItem(idStr); column.append(item);                               //	sb_column_playlistid
        item=new QStandardItem(qm->record(i).value(11).toString()); column.append(item);  //	sb_column_position
        id.sb_position=qm->record(i).value(11).toInt();
        item=new QStandardItem(qm->record(i).value(12).toString()); column.append(item);  //	sb_column_path
        id.path=qm->record(i).value(12).toString();

        item=new QStandardItem(qm->record(i).value(3).toString()); column.append(item);   //	sb_column_songtitle
        id.songTitle=qm->record(i).value(3).toString();
        item=new QStandardItem(qm->record(i).value(13).toString()); column.append(item);  //	sb_column_duration
        item=new QStandardItem(qm->record(i).value(6).toString()); column.append(item);   //	sb_column_performername
        id.performerName=qm->record(i).value(6).toString();
        item=new QStandardItem(qm->record(i).value(9).toString()); column.append(item);   //	sb_column_albumtitle
        id.albumTitle=qm->record(i).value(9).toString();

        this->appendRow(column); column.clear();
        playlist[playlist.count()]=id;

        if(i%100==0)
        {
            qDebug() << SB_DEBUG_INFO << "Populated" << i << "from " << qm->rowCount();
            QCoreApplication::processEvents();
        }
    }
    populateHeader();
    qDebug() << SB_DEBUG_INFO;
    setSongPlaying(_currentPlayID);
    return playlist;
}

///
/// \brief populate
/// \param newPlaylist
/// \param firstBatchHasLoadedFlag: provides a way to split loading in two batches:
/// 	-	first batch (a small set of records) is immediately loaded, displayed and music starts to play
///		-	second batch (with the remainder) is loaded while playing music.
void
SBCurrentPlaylistModel::populate(QMap<int,SBID> newPlaylist,bool firstBatchHasLoadedFlag)
{
    qDebug() << SB_DEBUG_INFO << "newPlaylist.count()=" << newPlaylist.count();
    int offset=0;

    if(!firstBatchHasLoadedFlag)
    {
        QStandardItemModel::clear();
        _currentPlayID=-1;
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
        SBID song=newPlaylist[i];
        if(firstBatchHasLoadedFlag)
        {
            qDebug() << SB_DEBUG_INFO << song;
        }
        QString idStr;
        idStr.sprintf("%08d",i+1);

        item=new QStandardItem("0"); column.append(item);                                       //	sb_column_deleteflag
        item=new QStandardItem(QString("%1").arg(song.sb_album_id)); column.append(item);       //	sb_column_albumid
        item=new QStandardItem(formatDisplayPlayID(i+1)); column.append(item);                  //	sb_column_displayplaylistid
        item->setData(Qt::AlignRight, Qt::TextAlignmentRole);

        item=new QStandardItem(QString("%1").arg(song.sb_song_id)); column.append(item);        //	sb_column_songid
        item=new QStandardItem(QString("%1").arg(song.sb_performer_id)); column.append(item);	//	sb_column_performerid
        item=new QStandardItem(idStr); column.append(item);                                     //	sb_column_playlistid
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
        setSongPlaying(_currentPlayID);
    }
    qDebug() << SB_DEBUG_INFO << "newPlaylist.count()" << newPlaylist.count();
    qDebug() << SB_DEBUG_INFO << "rowCount.count()" << this->rowCount();
}

void
SBCurrentPlaylistModel::populateHeader()
{
    QList<QStandardItem *>column;
    QStandardItem* item;

    int columnIndex=0;
    item=new QStandardItem("DEL"); this->setHorizontalHeaderItem(columnIndex++,item);
    item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);
    item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);
    item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);
    item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);
    item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);
    item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);
    item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);

    item=new QStandardItem("Song"); this->setHorizontalHeaderItem(columnIndex++,item);
    item=new QStandardItem("Duration"); this->setHorizontalHeaderItem(columnIndex++,item);
    item=new QStandardItem("Performer"); this->setHorizontalHeaderItem(columnIndex++,item);
    item=new QStandardItem("Album"); this->setHorizontalHeaderItem(columnIndex++,item);
}

void
SBCurrentPlaylistModel::reorderItems()
{
    QMap<int,int> toFrom;	//	map from old to new index (1-based)
    //	Create map
    qDebug() << SB_DEBUG_INFO << "start";
    qDebug() << SB_DEBUG_INFO << "_currentPlayID:before" << _currentPlayID;

    //	Reset first column
    for(int i=0;i<this->rowCount();i++)
    {
        int playID=-1;
        QStandardItem* item;

        item=this->item(i,sb_column_playlistid);
        if(item!=NULL)
        {
            item=this->item(i,sb_column_playlistid);
            playID=item->text().toInt();


            item->setText(QString("%1").arg(i+1));
            toFrom[playID-1]=i;
            qDebug() << SB_DEBUG_INFO << i << "playID=" << playID;
        }


        item=this->item(i,sb_column_displayplaylistid);
        if(item!=NULL)
        {
            item->setText(formatDisplayPlayID(i+1));
            item->setData(Qt::AlignRight, Qt::TextAlignmentRole);
        }
        paintRow(i);
    }
    qDebug() << SB_DEBUG_INFO << "_currentPlayID:after" << _currentPlayID;
    qDebug() << SB_DEBUG_INFO << "end";


    setSongPlaying(_currentPlayID);
    Context::instance()->getPlayerController()->reorderPlaylist(toFrom);
}

bool
SBCurrentPlaylistModel::removeRows(int row, int count, const QModelIndex &parent)
{
    qDebug() << SB_DEBUG_INFO << row << count << parent.row() << parent.column();
    bool result=QStandardItemModel::removeRows(row,count,parent);
    debugShow("after removeRows");
    this->reorderItems();
    return result;
}

void
SBCurrentPlaylistModel::repaintAll()
{
    for(int i=0;i<this->rowCount();i++)
    {
        paintRow(i);
    }
}

///	setSongPlaying() returns tableView row that is current.

QModelIndex
SBCurrentPlaylistModel::setSongPlaying(int playID)
{
    QStandardItem* item=NULL;
    int oldRowID=-1;
    int newRowID=-1;

    qDebug() << SB_DEBUG_INFO << "start" << playID << _currentPlayID;
    if(_currentPlayID<this->rowCount() && _currentPlayID>=0)
    {
        //	Don't execute if, if _currentPlayID:
        //	-	greater than rowcount, or
        //	-	not set (-1), or
        qDebug() << SB_DEBUG_INFO << this->rowCount();
        for(int i=0;oldRowID==-1 && i<this->rowCount();i++)
        {
            item=this->item(i,sb_column_playlistid);
            qDebug() << SB_DEBUG_INFO << i << "playlistid=" << item->text().toInt();

            if(item->text().toInt()-1==_currentPlayID)
            {
                qDebug() << SB_DEBUG_INFO;
                item=this->item(i,sb_column_displayplaylistid);
                if(item!=NULL)
                {
                    qDebug() << SB_DEBUG_INFO << "found:i=" << i;
                    item->setText(formatDisplayPlayID(_currentPlayID+1));
                    item->setData(Qt::AlignRight, Qt::TextAlignmentRole);
                    oldRowID=i;
                }
            }
        }
    }
    qDebug() << SB_DEBUG_INFO << "++++++++++++++++++++++++++++++++++++++";

    if(playID<this->rowCount() && playID>=0)
    {
        _currentPlayID=playID;

        for(int i=0;newRowID==-1 && i<this->rowCount();i++)
        {
            item=this->item(i,sb_column_playlistid);
            qDebug() << SB_DEBUG_INFO << i << "playlistid=" << item->text().toInt();

            if(item->text().toInt()-1==_currentPlayID)
            {
                qDebug() << SB_DEBUG_INFO;
                item=this->item(i,sb_column_displayplaylistid);
                if(item!=NULL)
                {
                    qDebug() << SB_DEBUG_INFO << "found:i=" << i;
                    item->setText(formatDisplayPlayID(_currentPlayID+1,1));
                    item->setData(Qt::AlignRight, Qt::TextAlignmentRole);
                    paintRow(i);
                    newRowID=i;
                }
            }
        }
    }
    qDebug() << SB_DEBUG_INFO << "end" << playID << _currentPlayID << oldRowID;
    if(oldRowID>=0)
    {
        paintRow(oldRowID);
    }
    return this->index(newRowID,0);
}

void
SBCurrentPlaylistModel::shuffle()
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

        item=this->item(i,sb_column_playlistid);
        if(item!=NULL)
        {
            item=this->item(i,sb_column_playlistid);
            playID=item->text().toInt();
            //qDebug() << SB_DEBUG_INFO << "assigning from" << playID << "to" << fromTo[playID];

            item->setText(QString("%1").arg(fromTo[playID]));
        }


        item=this->item(i,sb_column_displayplaylistid);
        if(item!=NULL)
        {
            item->setText(formatDisplayPlayID(fromTo[playID]));
            item->setData(Qt::AlignRight, Qt::TextAlignmentRole);
        }
        //paintRow(i);
    }
    debugShow("after shuffle");
    qDebug() << SB_DEBUG_INFO << "currentPlayID:before=" << _currentPlayID;
    _currentPlayID=fromTo[_currentPlayID+1]-1;
    qDebug() << SB_DEBUG_INFO << "currentPlayID:new=" << _currentPlayID;
    Context::instance()->getPlayerController()->reorderPlaylist(fromTo);
    //paintRow(oldPlayID);
    //paintRow(_currentPlayID);
    setSongPlaying(_currentPlayID);
}

///	Debugging
void
SBCurrentPlaylistModel::debugShow(const QString& title)
{
    qDebug() << SB_DEBUG_INFO << title;
    for(int i=0;i<this->rowCount();i++)
    {
        QString row=QString("row=%1").arg(i);
        for(int j=0;j<this->columnCount();j++)
        {
            if(j!=7 && j<9)
            {
                QStandardItem* item=this->item(i,j);
                if(item)
                {
                    row+="|'"+item->text()+"'";
                }
                else
                {
                    row+="|<NULL>";
                }
            }
        }
        qDebug() << SB_DEBUG_INFO << row;
    }
}
