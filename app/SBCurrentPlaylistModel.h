#ifndef SBCURRENTPLAYLISTMODEL_H
#define SBCURRENTPLAYLISTMODEL_H

#include <QStandardItemModel>

class SBCurrentPlaylistModel : public QStandardItemModel
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

    SBCurrentPlaylistModel(QObject* parent=0);

    //	Methods related to drag&drop
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QMimeData* mimeData(const QModelIndexList & indexes) const;
    virtual QStringList mimeTypes() const;
    virtual Qt::DropActions supportedDropActions() const;

    //	Methods unrelated to drag&drop
    QModelIndex addRow();
    virtual void clear();
    QString formatDisplayPlayID(int playID,bool isCurrent=0);
    void paintRow(int i);


    QMap<int,SBID> populate();
    void populate(QMap<int,SBID> newPlaylist,bool firstBatchHasLoadedFlag=0);
    void populateHeader();
    void reorderItems();
    virtual bool removeRows(int row, int count, const QModelIndex &parent);
    void repaintAll();

    ///	setSongPlaying() returns tableView row that is current.
    virtual QModelIndex setSongPlaying(int playID);
    void shuffle();

    ///	Debugging
    void debugShow(const QString& title=QString());

private:
    int _currentPlayID;             //	0-based
};

#endif // SBCURRENTPLAYLISTMODEL_H