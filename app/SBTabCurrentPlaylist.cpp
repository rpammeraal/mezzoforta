#include "SBTabCurrentPlaylist.h"

#include <QProgressDialog>

#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "PlayerController.h"
#include "SBModelCurrentPlaylist.h"
#include "SBModelPlaylist.h"
#include "SBModelSong.h"
#include "SBSqlQueryModel.h"
#include "Navigator.h"

class CurrentPlaylistModel : public QStandardItemModel
{
public:

    enum sb_column_type
    {
        sb_column_deleteflag=0,
        sb_column_albumid=1,
        sb_column_displayplaylistid=2,
        sb_column_songid=3,
        sb_column_performerid=4,
        sb_column_playlistid=5,
        sb_column_position=6,
        sb_column_path=7,

        sb_column_startofdata=8,
        sb_column_songtitle=8,
        sb_column_duration=9,
        sb_column_performername=10,
        sb_column_albumtitle=11
    };

    CurrentPlaylistModel(QObject* parent=0):QStandardItemModel(parent)
    {
        qDebug() << SB_DEBUG_INFO;
        qDebug() << SB_DEBUG_INFO << "++++++++++++++++++++++++++++++++++++++";
        _currentPlayID=-1;
    }

    //	Methods related to drag&drop
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
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

    virtual Qt::ItemFlags flags(const QModelIndex &index) const
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

    virtual QMimeData* mimeData(const QModelIndexList & indexes) const
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

    virtual QStringList mimeTypes() const
    {
        QStringList types;
        types << "application/vnd.text.list";
        return types;
    }

    virtual Qt::DropActions supportedDropActions() const
    {
        return Qt::MoveAction;
    }

    //	Methods unrelated to drag&drop
    QModelIndex addRow()
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

    virtual void clear()
    {
        _currentPlayID=-1;
        QStandardItemModel::clear();
    }

    QString formatDisplayPlayID(int playID,bool isCurrent=0)
    {
        QString str;

        str.sprintf("%s %d",(isCurrent?">":"   "),playID);
        return str;
    }

    void paintRow(int i)
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
            item=this->item(i,CurrentPlaylistModel::sb_column_playlistid);
            if(item)
            {
                playlistID=item->text().toInt();

                item=this->item(i,j);
                if(item)
                {
                    if(j==CurrentPlaylistModel::sb_column_displayplaylistid)
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

    void populate()
    {
        qDebug() << SB_DEBUG_INFO << _currentPlayID;
        _currentPlayID=-1;
        SBSqlQueryModel* qm=SBModelCurrentPlaylist::getAllSongs();
        QList<QStandardItem *>column;
        QStandardItem* item;

        for(int i=0;i<qm->rowCount();i++)
        {
            QString idStr;
            idStr.sprintf("%08d",i+1);

            item=new QStandardItem("0"); column.append(item);                                 //	sb_column_deleteflag
            item=new QStandardItem(qm->record(i).value(8).toString()); column.append(item);   //	sb_column_albumid
            item=new QStandardItem(formatDisplayPlayID(i+1)); column.append(item);            //	sb_column_displayplaylistid
            item->setData(Qt::AlignRight, Qt::TextAlignmentRole);

            item=new QStandardItem(qm->record(i).value(2).toString()); column.append(item);   //	sb_column_songid
            item=new QStandardItem(qm->record(i).value(5).toString()); column.append(item);   //	sb_column_performerid
            item=new QStandardItem(idStr); column.append(item);                               //	sb_column_playlistid
            item=new QStandardItem(qm->record(i).value(11).toString()); column.append(item);  //	sb_column_position
            item=new QStandardItem(qm->record(i).value(12).toString()); column.append(item);  //	sb_column_path

            item=new QStandardItem(qm->record(i).value(3).toString()); column.append(item);   //	sb_column_songtitle
            item=new QStandardItem(qm->record(i).value(13).toString()); column.append(item);  //	sb_column_duration
            item=new QStandardItem(qm->record(i).value(6).toString()); column.append(item);   //	sb_column_performername
            item=new QStandardItem(qm->record(i).value(9).toString()); column.append(item);   //	sb_column_albumtitle
            this->appendRow(column); column.clear();

            if(i%100==0)
            {
                qDebug() << SB_DEBUG_INFO << "Populated" << i << "from " << qm->rowCount();
                QCoreApplication::processEvents();
            }
        }
        populateHeader();
        qDebug() << SB_DEBUG_INFO;
        setSongPlaying(_currentPlayID);
    }

    void populate(QMap<int,SBID> newPlaylist,bool addToExisting=0,int offset=0)
    {
        qDebug() << SB_DEBUG_INFO;
        if(!addToExisting)
        {
            QStandardItemModel::clear();
            _currentPlayID=-1;
        }
        QList<QStandardItem *>column;
        QStandardItem* item;

        for(int i=offset;i<newPlaylist.count();i++)
        {
            SBID song=newPlaylist[i];
            QString idStr;
            idStr.sprintf("%08d",i+1);

            item=new QStandardItem("0"); column.append(item);                                 //	sb_column_deleteflag
            item=new QStandardItem(song.sb_album_id); column.append(item);                    //	sb_column_albumid
            item=new QStandardItem(formatDisplayPlayID(i+1)); column.append(item);            //	sb_column_displayplaylistid
            item->setData(Qt::AlignRight, Qt::TextAlignmentRole);

            item=new QStandardItem(song.sb_song_id); column.append(item);                     //	sb_column_songid
            item=new QStandardItem(song.sb_performer_id); column.append(item);                //	sb_column_performerid
            item=new QStandardItem(idStr); column.append(item);                               //	sb_column_playlistid
            item=new QStandardItem(song.sb_position); column.append(item);                    //	sb_column_position
            item=new QStandardItem(song.path); column.append(item);                           //	sb_column_path

            item=new QStandardItem(song.songTitle); column.append(item);                      //	sb_column_songtitle
            item=new QStandardItem(song.duration.toString()); column.append(item);            //	sb_column_duration
            item=new QStandardItem(song.performerName); column.append(item);                  //	sb_column_performername
            item=new QStandardItem(song.albumTitle); column.append(item);                     //	sb_column_albumtitle
            this->appendRow(column); column.clear();

            QCoreApplication::processEvents();

        }
        if(!addToExisting)
        {
            populateHeader();
            setSongPlaying(_currentPlayID);
        }
    }

    void populateHeader()
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

    void reorderItems()
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
        Context::instance()->getPlayerController()->loadPlaylist(toFrom);
    }

    virtual bool removeRows(int row, int count, const QModelIndex &parent)
    {
        qDebug() << SB_DEBUG_INFO << row << count << parent.row() << parent.column();
        bool result=QStandardItemModel::removeRows(row,count,parent);
        debugShow("after removeRows");
        this->reorderItems();
        return result;
    }

    void repaintAll()
    {
        for(int i=0;i<this->rowCount();i++)
        {
            paintRow(i);
        }
    }

    ///	setSongPlaying() returns tableView row that is current.
    virtual QModelIndex setSongPlaying(int playID)
    {
        qDebug() << SB_DEBUG_INFO;
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

    void shuffle()
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
        Context::instance()->getPlayerController()->loadPlaylist(fromTo);
        //paintRow(oldPlayID);
        //paintRow(_currentPlayID);
        setSongPlaying(_currentPlayID);
    }

    ///	Debugging
    void debugShow(const QString& title=QString())
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

private:
    int _currentPlayID;             //	0-based
};

SBTabCurrentPlaylist::SBTabCurrentPlaylist(QWidget* parent) : SBTab(parent,0)
{
}


QMap<int,SBID>
SBTabCurrentPlaylist::playlist()
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;
    QMap<int,SBID> playlist;
    SBID song;

    if(_playlistLoadedFlag==0)
    {
       this->_populate(song);	//	argument song is ignored
    }

    CurrentPlaylistModel* aem=dynamic_cast<CurrentPlaylistModel *>(tv->model());


    int index=0;
    for(int i=0;aem && i<aem->rowCount();i++)
    {
        QStandardItem* item;
        int songID=0;

        //	sb_column_playlistid
        item=aem->item(i,CurrentPlaylistModel::sb_column_playlistid);
        if(item)
        {
            index=item->text().toInt();
        }

        //	sb_column_songid
        item=aem->item(i,CurrentPlaylistModel::sb_column_songid);
        if(item)
        {
            songID=item->text().toInt();
            song.assign(SBID::sb_type_song,songID);
        }

        //	sb_column_albumid
        item=aem->item(i,CurrentPlaylistModel::sb_column_albumid);
        if(item)
        {
            song.sb_album_id=item->text().toInt();
        }

        //	sb_column_performerid
        item=aem->item(i,CurrentPlaylistModel::sb_column_performerid);
        if(item)
        {
            song.sb_performer_id=item->text().toInt();
        }

        //	sb_column_position
        item=aem->item(i,CurrentPlaylistModel::sb_column_position);
        if(item)
        {
            song.sb_position=item->text().toInt();
        }

        //	sb_column_path
        item=aem->item(i,CurrentPlaylistModel::sb_column_path);
        if(item)
        {
            song.path=item->text();
        }

        //	sb_column_songtitle
        item=aem->item(i,CurrentPlaylistModel::sb_column_songtitle);
        if(item)
        {
            song.songTitle=item->text();
        }

        //	sb_column_performername
        item=aem->item(i,CurrentPlaylistModel::sb_column_performername);
        if(item)
        {
            song.performerName=item->text();
        }

        //	sb_column_albumtitle
        item=aem->item(i,CurrentPlaylistModel::sb_column_albumtitle);
        if(item)
        {
            song.albumTitle=item->text();
        }

        if(song.sb_item_type()!=SBID::sb_type_invalid)
        {
            playlist[index-1]=song;
        }
    }
    for(int i=0;i<playlist.count();i++)
    {
        qDebug() << SB_DEBUG_INFO << i << playlist[i];
    }

    return playlist;
}

QTableView*
SBTabCurrentPlaylist::subtabID2TableView(int subtabID) const
{
    Q_UNUSED(subtabID);
    const MainWindow* mw=Context::instance()->getMainWindow();
    return mw->ui.currentPlaylistDetailSongList;
}

///	Public slots
void
SBTabCurrentPlaylist::deletePlaylistItem()
{
    SBID assignID=getSBIDSelected(_lastClickedIndex);
    if(assignID.sb_item_type()!=SBID::sb_type_invalid)
    {
        MainWindow* mw=Context::instance()->getMainWindow();
        QTableView* tv=mw->ui.currentPlaylistDetailSongList;
        CurrentPlaylistModel* aem=dynamic_cast<CurrentPlaylistModel *>(tv->model());
        aem->removeRows(_lastClickedIndex.row(),1,QModelIndex());
        qDebug() << SB_DEBUG_INFO << _lastClickedIndex << _lastClickedIndex.row() << _lastClickedIndex.column();

        QString updateText=QString("Removed %4 %1%2%3 from playlist.")
            .arg(QChar(96))            //	1
            .arg(assignID.getText())   //	2
            .arg(QChar(180))           //	3
            .arg(assignID.getType())   //	4
        ;
        Context::instance()->getController()->updateStatusBar(updateText);
    }
}

void
SBTabCurrentPlaylist::movePlaylistItem(const SBID& fromID, const SBID &toID)
{
    //	Determine current playlist
    SBID currentID=Context::instance()->getScreenStack()->currentScreen();
    qDebug() << SB_DEBUG_INFO << currentID << fromID << toID;
    return;

    SBModelPlaylist *mpl=new SBModelPlaylist();
    mpl->reorderItem(currentID,fromID,toID);
    refreshTabIfCurrent(currentID);
}

void
SBTabCurrentPlaylist::showContextMenuPlaylist(const QPoint &p)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QModelIndex idx=mw->ui.currentPlaylistDetailSongList->indexAt(p);

    SBID id=getSBIDSelected(idx);
    qDebug() << SB_DEBUG_INFO << id;
    if(id.sb_item_type()!=SBID::sb_type_invalid)
    {
        _lastClickedIndex=idx;

        QPoint gp = mw->ui.currentPlaylistDetailSongList->mapToGlobal(p);

        QMenu menu(NULL);
        menu.addAction(deletePlaylistItemAction);
        menu.exec(gp);
    }
}

void
SBTabCurrentPlaylist::songChanged(int playID)
{
    qDebug() << SB_DEBUG_INFO << playID;
    MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;
    CurrentPlaylistModel* aem=dynamic_cast<CurrentPlaylistModel *>(tv->model());
    QModelIndex idx=aem->setSongPlaying(playID);
    qDebug() << SB_DEBUG_INFO << idx << idx.row() << idx.column();
    tv->scrollTo(idx);
}

///	Private slots
void
SBTabCurrentPlaylist::clearPlaylist()
{
    qDebug() << SB_DEBUG_INFO;
    MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;
    CurrentPlaylistModel* aem=dynamic_cast<CurrentPlaylistModel *>(tv->model());
    aem->clear();
}

void
SBTabCurrentPlaylist::shufflePlaylist()
{
    qDebug() << SB_DEBUG_INFO;
    MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;
    CurrentPlaylistModel* aem=dynamic_cast<CurrentPlaylistModel *>(tv->model());
    aem->shuffle();
    tv->sortByColumn(5,Qt::AscendingOrder);
    aem->repaintAll();
}

void
SBTabCurrentPlaylist::startRadio()
{
    MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;
    CurrentPlaylistModel* aem=dynamic_cast<CurrentPlaylistModel *>(tv->model());
    PlayerController* pc=Context::instance()->getPlayerController();
    const int firstBatchNumber=10;
    bool firstBatchLoaded=false;

    qDebug() << SB_DEBUG_INFO;
    this->clearPlaylist();

    QMap<int,SBID> playList;
    QList<int> indexCovered;

    int progressStep=0;
    QProgressDialog pd("Starting Auto DJ",QString(),0,11);
    pd.setWindowModality(Qt::WindowModal);
    pd.show();
    pd.raise();
    pd.activateWindow();
    QCoreApplication::processEvents();
    pd.setValue(0);
    QCoreApplication::processEvents();

    SBSqlQueryModel* qm=SBModelCurrentPlaylist::getAllOnlineSongs();
    pd.setValue(++progressStep);
    QCoreApplication::processEvents();

    int numSongs=qm->rowCount();
    const int maxNumberAttempts=50;
    int songInterval=numSongs/10;

    qDebug() << SB_DEBUG_INFO << "randomizing " << numSongs << "songs";
    bool found=1;
    int i=0;
    for(;found && i<numSongs;i++)
    {
        found=0;
        int j=maxNumberAttempts;
        int idx=-1;
        while(!found && --j)
        {
            idx=Common::randomOldestFirst(numSongs);
            if(indexCovered.contains(idx)==0)
            {
                found=1;
                indexCovered.append(idx);
            }
            if(j==0)
            {
                qDebug() << "too many attempts -- quitting";
                return;
            }
        }

        if(found)
        {
            SBID item=SBID(SBID::sb_type_song,qm->record(idx).value(0).toInt());

            item.songTitle=qm->record(idx).value(1).toString();
            item.sb_performer_id=qm->record(idx).value(2).toInt();
            item.performerName=qm->record(idx).value(3).toString();
            item.sb_album_id=qm->record(idx).value(4).toInt();
            item.albumTitle=qm->record(idx).value(5).toString();
            item.sb_position=qm->record(idx).value(6).toInt();
            item.path=qm->record(idx).value(7).toString();
            item.duration=qm->record(idx).value(8).toTime();

            playList[i]=item;

            //if(i%songInterval==0)
            {
                pd.setValue(++progressStep);
                QCoreApplication::processEvents();
                //qDebug() << SB_DEBUG_INFO << "Populated" << i << "songs in" << maxNumberAttempts-j << "attempts";
                //qDebug() << SB_DEBUG_INFO << "aa" << i << playList[i];
            }

            if(i==firstBatchNumber)
            {
                firstBatchLoaded=true;
                aem->populate(playList);
            }
        }
    }

    qDebug() << SB_DEBUG_INFO << "Populated" << playList.count() << "of" << numSongs;
    qDebug() << SB_DEBUG_INFO << "indexCovered.count" << indexCovered.count();
    qDebug() << SB_DEBUG_INFO << "firstBatchLoaded" << firstBatchLoaded;

    if(!firstBatchLoaded)
    {
        qDebug() << SB_DEBUG_INFO;
        firstBatchLoaded=true;
        aem->populate(playList);
    }
    this->_populatePost(SBID());
    QCoreApplication::processEvents();

    //	This code could be reused in other situations.
    //	Stop player, tell playerController that we have a new playlist and start player.
    pc->playerStop();
    pc->loadPlaylist();
    pc->playerPlay();
    //	End reuseable


    QString allIDX=" ";
    for(int i=0;i<indexCovered.count();i++)
    {
        if(indexCovered.contains(i))
        allIDX+=QString("%1 ").arg(i);
    }
    qDebug() << SB_DEBUG_INFO << "allIDX=" << allIDX;

    //	Populate the rest of list in sequential order
    for(i=0;i<numSongs;i++)
    {
        if(indexCovered.contains(i)==0)
        {
            indexCovered.append(i);

            SBID item=SBID(SBID::sb_type_song,qm->record(i).value(0).toInt());

            item.songTitle=qm->record(i).value(1).toString();
            item.sb_performer_id=qm->record(i).value(2).toInt();
            item.performerName=qm->record(i).value(3).toString();
            item.sb_album_id=qm->record(i).value(4).toInt();
            item.albumTitle=qm->record(i).value(5).toString();
            item.sb_position=qm->record(i).value(6).toInt();
            item.path=qm->record(i).value(7).toString();
            item.duration=qm->record(i).value(8).toTime();

            playList[playList.count()]=item;

//            //if(i%songInterval==0)
//            {
//                qDebug() << SB_DEBUG_INFO << "bb" << i << playList[i];
//                qDebug() << SB_DEBUG_INFO << "Populated" << i << "idxCvd" << indexCovered.count()
//                         << "playlist" << playList.count();
//                //pd.setValue(++progressStep);
//                QCoreApplication::processEvents();
//            }
        }
    }

//    for(int i=0;i<playList.count();i++)
//    {
//        qDebug() << SB_DEBUG_INFO << i << playList[i];
//    }

    aem->populate(playList,firstBatchLoaded,firstBatchNumber+1);
    //tv->resizeColumnsToContents();
    //aem->populate(playList);

    pd.setValue(++progressStep);
    qDebug() << SB_DEBUG_INFO << "Populated" << playList.count() << "of" << numSongs;
    qDebug() << SB_DEBUG_INFO << "indexCovered.count" << indexCovered.count();
}

void
SBTabCurrentPlaylist::tableViewCellClicked(QModelIndex idx)
{
    qDebug() << SB_DEBUG_INFO << idx.column() << idx.row();
    if((CurrentPlaylistModel::sb_column_type)idx.column()==CurrentPlaylistModel::sb_column_playlistid)
    {
        qDebug() << SB_DEBUG_INFO;
    }
    else
    {
        SBID item=getSBIDSelected(idx);
        qDebug() << SB_DEBUG_INFO << item;
        Context::instance()->getNavigator()->openScreenByID(item);
    }
}
void
SBTabCurrentPlaylist::tableViewCellDoubleClicked(QModelIndex idx)
{
    qDebug() << SB_DEBUG_INFO << idx.row();
    Context::instance()->getPlayerController()->playerPlayNow(idx.row());
}

///	Private methods

void
SBTabCurrentPlaylist::init()
{
    _pm=NULL;
    _playlistLoadedFlag=0;
    if(_initDoneFlag==0)
    {
        MainWindow* mw=Context::instance()->getMainWindow();
        QTableView* tv=mw->ui.currentPlaylistDetailSongList;

        //	Actions on tableview
        connect(tv, SIGNAL(clicked(QModelIndex)),
                this, SLOT(tableViewCellClicked(QModelIndex)));
        connect(tv,SIGNAL(doubleClicked(QModelIndex)),
                this, SLOT(tableViewCellDoubleClicked(QModelIndex)));

        //	Buttons
        connect(mw->ui.pbClearPlaylist, SIGNAL(clicked(bool)),
                this, SLOT(clearPlaylist()));
        connect(mw->ui.pbShufflePlaylist, SIGNAL(clicked(bool)),
                this, SLOT(shufflePlaylist()));
        connect(mw->ui.pbStartRadio, SIGNAL(clicked(bool)),
                this, SLOT(startRadio()));

        //	If playerController changes song, we want to update our view.
        connect(Context::instance()->getPlayerController(),SIGNAL(songChanged(int)),
                this, SLOT(songChanged(int)));

        //	Context menu
        tv->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(tv, SIGNAL(customContextMenuRequested(const QPoint&)),
                this, SLOT(showContextMenuPlaylist(QPoint)));

        //	Delete playlist
        deletePlaylistItemAction = new QAction(tr("Delete Item From Playlist "), this);
        deletePlaylistItemAction->setStatusTip(tr("Delete Item From Playlist"));
        connect(deletePlaylistItemAction, SIGNAL(triggered()),
                this, SLOT(deletePlaylistItem()));
        _initDoneFlag=1;
    }
}


//	Due to the nature of drag/drop, this view differs from others.
SBID
SBTabCurrentPlaylist::getSBIDSelected(const QModelIndex &idx)
{
    SBID id;

    MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;
    CurrentPlaylistModel* aem=dynamic_cast<CurrentPlaylistModel *>(tv->model());
    SBID::sb_type itemType=SBID::sb_type_invalid;
    QStandardItem* item;
    int itemID=-1;

    switch((CurrentPlaylistModel::sb_column_type)idx.column())
    {
    case CurrentPlaylistModel::sb_column_deleteflag:
    case CurrentPlaylistModel::sb_column_albumid:
    case CurrentPlaylistModel::sb_column_displayplaylistid:
    case CurrentPlaylistModel::sb_column_songid:
    case CurrentPlaylistModel::sb_column_performerid:
    case CurrentPlaylistModel::sb_column_playlistid:
    case CurrentPlaylistModel::sb_column_position:
    case CurrentPlaylistModel::sb_column_path:
        break;

    case CurrentPlaylistModel::sb_column_songtitle:
    case CurrentPlaylistModel::sb_column_duration:
        itemType=SBID::sb_type_song;
        item=aem->item(idx.row(),CurrentPlaylistModel::sb_column_songid);
        itemID=(item!=NULL)?item->text().toInt():-1;
        break;

    case CurrentPlaylistModel::sb_column_performername:
        itemType=SBID::sb_type_performer;
        item=aem->item(idx.row(),CurrentPlaylistModel::sb_column_performerid);
        itemID=(item!=NULL)?item->text().toInt():-1;
        break;

    case CurrentPlaylistModel::sb_column_albumtitle:
        itemType=SBID::sb_type_album;
        item=aem->item(idx.row(),CurrentPlaylistModel::sb_column_albumid);
        itemID=(item!=NULL)?item->text().toInt():-1;
        break;

    }

    id.assign(itemType,itemID);
    qDebug() << SB_DEBUG_INFO << id;
    return id;
}

SBID
SBTabCurrentPlaylist::_populate(const SBID& id)
{
    Q_UNUSED(id);
    init();
    const MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;

    //	Populate playlist

    CurrentPlaylistModel* plm=dynamic_cast<CurrentPlaylistModel *>(tv->model());
    if(plm==NULL)
    {
        plm=new CurrentPlaylistModel();
        plm->populate();
        tv->setModel(plm);
    }

    _playlistLoadedFlag=1;
    return SBID(SBID::sb_type_current_playlist,-1);
}

void
SBTabCurrentPlaylist::_populatePost(const SBID &id)
{
    Q_UNUSED(id);
    const MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;

    //	2.	Set up view
    tv->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tv->setDragEnabled(1);
    tv->setAcceptDrops(1);
    tv->viewport()->setAcceptDrops(1);
    tv->setDropIndicatorShown(1);
    tv->setDragDropMode(QAbstractItemView::InternalMove);
    tv->setDefaultDropAction(Qt::MoveAction);
    tv->setDragDropOverwriteMode(false);
    tv->setColumnHidden(0,1);	//	sb_column_deleteflag
    tv->setColumnHidden(1,1);	//	sb_column_albumid
    tv->setColumnHidden(3,1);	//	sb_column_songid
    tv->setColumnHidden(4,1);	//	sb_column_performerid
    tv->setColumnHidden(5,1);	//	sb_column_playlistid
    tv->setColumnHidden(6,1);	//	sb_column_position
    tv->setColumnHidden(7,1);	//	sb_column_path

    QHeaderView* hv=NULL;
    hv=tv->horizontalHeader();
    hv->setSectionResizeMode(QHeaderView::ResizeToContents);
    hv->setStretchLastSection(1);

    hv=tv->verticalHeader();
    hv->hide();
    hv->setDefaultSectionSize(18);

    tv->setEditTriggers(QAbstractItemView::AllEditTriggers);
}
