#ifndef SCREENITEM_H
#define SCREENITEM_H

#include "SBIDBase.h"

class ScreenItem
{
public:
    enum screen_type
    {
        screen_type_invalid,
        screen_type_sbidbase,
        screen_type_allsongs,
        screen_type_songsearch,
        screen_type_current_playlist
    };

    ScreenItem();
    ScreenItem(const SBIDPtr& base);
    ScreenItem(const QString& searchCriteria);
    ScreenItem(ScreenItem::screen_type screenType);
    ~ScreenItem();

    inline bool editFlag() const { return _editFlag; }
    inline ScreenItem::screen_type screenType() const { return _screenType; }
    void setEditFlag(bool editFlag) { _editFlag=editFlag; }
    void setSortColumn(int sortColumn) { _sortColumn=sortColumn; }
    void setSubtabID(int subtabID) { _subtabID=subtabID; }
    inline int sortColumn() const { return _sortColumn; }
    inline int subtabID() const { return _subtabID; }
    SBIDBase base() const { return *_ptr; }	//	CWIP: remove
    SBIDPtr ptr() const { return _ptr; }

    bool operator==(const ScreenItem& i) const;
    bool operator!=(const ScreenItem& i) const;
    friend QDebug operator<<(QDebug dbg, const ScreenItem& id);

private:
    screen_type _screenType;
    SBIDPtr     _ptr;
    bool        _editFlag;
    QString     _searchCriteria;
    int         _subtabID;
    int         _sortColumn;
    QString     _errorMsg;

    void _init();
};

#endif // SCREENITEM_H
