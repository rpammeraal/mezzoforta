#ifndef SBTABCHOOSER_H
#define SBTABCHOOSER_H

#include "SBTab.h"

//	SBTabChooser is a base model for all SBTab classes that operate from the
//	Chooser class (e.g.: SBTabPlaylist, SBTabChart, ...)
class SBTabChooser : public SBTab
{
    Q_OBJECT

public:
    SBTabChooser(QWidget* parent=0);
    virtual QTableView* subtabID2TableView(int subtabID) const =0;

public slots:
    void showContextMenuLabel(const QPoint &p);
    void showContextMenuView(const QPoint &p);

protected:
    //	PlaylistItem class is used to transfer the four items
    //	between the different methods in this class.
    class PlaylistItem
    {
    public:
        PlaylistItem() { _init(); }

        //	Store key instead of itemPtr -- this will make it easier
        //	to purge items from cache later on
        SBKey   key;
        int     playlistPosition;
        QString text;

    private:
        void _init()
        {
            key=SBKey();
            playlistPosition=-1;
            text="";
        }
    };

    QAction* _deleteItemAction;

    PlaylistItem _getSelectedItem(QAbstractItemModel* aim, const QModelIndex& idx);
    virtual void _init()=0;
};

#endif // SBTABCHOOSER_H
