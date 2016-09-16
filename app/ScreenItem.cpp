#include <QDebug>
#include "ScreenItem.h"

///	Ctors
ScreenItem::ScreenItem()
{
    _init();
}

ScreenItem::ScreenItem(const SBIDBase &base)
{
    _init();
    _screenType=ScreenItem::screen_type_sbidbase;
    _base=base;
}

ScreenItem::ScreenItem(const QString &searchCriteria)
{
    _init();

    //	Only set to songsearch if parameter is populated.
    _screenType=(searchCriteria.length()?ScreenItem::screen_type_songsearch:ScreenItem::screen_type_invalid);
    _searchCriteria=searchCriteria;
}

ScreenItem::ScreenItem(ScreenItem::screen_type screenType)
{
    _init();
    _screenType=screenType;
}

ScreenItem::~ScreenItem()
{
}

///	Public methods
bool
ScreenItem::operator ==(const ScreenItem& i) const
{
    if(this->_screenType!=i._screenType)
    {
        return 0;
    }
    else
    {
        switch(this->_screenType)
        {
        case ScreenItem::screen_type_invalid:
        case ScreenItem::screen_type_allsongs:
        case ScreenItem::screen_type_current_playlist:
            return 1;

        case ScreenItem::screen_type_sbidbase:
            return (this->_base==i._base) && (this->_editFlag==i._editFlag);

        case ScreenItem::screen_type_songsearch:
            return this->_searchCriteria==i._searchCriteria;
        }
    }

    return 0;
}

bool
ScreenItem::operator !=(const ScreenItem& i) const
{
    return !(this->operator ==(i));
}

QDebug
operator<<(QDebug dbg, const ScreenItem& id)
{
    dbg.nospace() << "ScreenItem:";

    switch(id._screenType)
    {
    case ScreenItem::screen_type_allsongs:
        dbg.nospace() << "AllSongs";
        break;

    case ScreenItem::screen_type_current_playlist:
        dbg.nospace() << "CurrentPlaylist";
        break;

    case ScreenItem::screen_type_invalid:
        dbg.nospace() << "<INVALID>";
        break;

    case ScreenItem::screen_type_sbidbase:
        dbg.nospace() << "SBIDBase [" << id._base << "]";
        break;

    case ScreenItem::screen_type_songsearch:
        dbg.nospace() << "search [" << id._searchCriteria << "]";
        break;
    }

    dbg.nospace() << ":subtabID=" << id._subtabID << ":sortColumn=" << id._sortColumn;

    return dbg.space();
}

///	Private methods

void
ScreenItem::_init()
{
    _screenType=ScreenItem::screen_type_invalid;
    _base=SBIDBase();
    _editFlag=0;
    _searchCriteria=QString();
    _subtabID=INT_MAX;
    _sortColumn=INT_MAX;
    _errorMsg=QString();
}
