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
    ScreenItem(const SBKey& key);
    ScreenItem(const QString& searchCriteria);
    ScreenItem(ScreenItem::screen_type screenType);
    ~ScreenItem();

    bool compare(const ScreenItem& i, bool ignoreEditFlag=0) const;
    inline bool editFlag() const { return _editFlag; }
    inline SBKey key() const { return _key; }
    inline ScreenItem::screen_type screenType() const { return _screenType; }
    inline QString searchCriteria() const { return _searchCriteria; }
    inline void setEditFlag(bool editFlag) { _editFlag=editFlag; }
    inline void setSortColumn(int sortColumn) { _sortColumn=sortColumn; }
    inline void setSubtabID(int subtabID) { _subtabID=subtabID; }
    inline int sortColumn() const { return _sortColumn; }
    inline int subtabID() const { return _subtabID; }
    void updateSBIDBase(const SBKey& key);	//	CWIP: rename to updateSBIDPtr

    bool operator==(const ScreenItem& i) const;
    bool operator!=(const ScreenItem& i) const;
    friend QDebug operator<<(QDebug dbg, const ScreenItem& id);

private:
    screen_type _screenType;
    bool    _editFlag;
    SBKey   _key;
    QString _searchCriteria;
    int     _subtabID;
    int     _sortColumn;
    QString _errorMsg;

    void _init();
};

#endif // SCREENITEM_H
