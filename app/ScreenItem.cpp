#include <QDebug>

#include "Common.h"
#include "ScreenItem.h"

///	Ctors
ScreenItem::ScreenItem()
{
    _init();
}

ScreenItem::ScreenItem(SBKey key):_key(key)
{
    _init();
    _screenType=ScreenItem::screen_type_sbidbase;
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
ScreenItem::compare(const ScreenItem &i, bool ignoreEditFlag) const
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
            if(ignoreEditFlag)
            {
                return this->_key==i._key;
            }
            return (this->_key==i._key) && (this->_editFlag==i._editFlag);

        case ScreenItem::screen_type_songsearch:
            return this->_searchCriteria==i._searchCriteria;
        }
    }

    return 0;

}

void
ScreenItem::updateSBIDBase(SBKey key)
{
    if(this->screenType()==ScreenItem::screen_type_sbidbase)
    {
        this->_key=key;
    }
    else
    {
        qDebug() << SB_DEBUG_ERROR << "Incompatible screenType: " << this->screenType();
    }
}

///	Public methods
bool
ScreenItem::operator ==(const ScreenItem& i) const
{
    return this->compare(i);
}

bool
ScreenItem::operator !=(const ScreenItem& i) const
{
    return !(this->compare(i));
}

QDebug
operator<<(QDebug dbg, const ScreenItem& si)
{
    dbg.nospace() << "ScreenItem:";

    switch(si._screenType)
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
        dbg.nospace() << "SBIDBase [" << si.key().toString() << "]";
        break;

    case ScreenItem::screen_type_songsearch:
        dbg.nospace() << "search [" << si._searchCriteria << "]";
        break;
    }

    dbg.nospace()
            << " :subtabID=" << si._subtabID
            << " :sortColumn=" << si._sortColumn
            << " :editFlag=" << si._editFlag
    ;

    return dbg.space();
}

///	Private methods

void
ScreenItem::_init()
{
    _screenType=ScreenItem::screen_type_invalid;
    _editFlag=0;
    _searchCriteria=QString();
    _subtabID=INT_MAX;
    _sortColumn=INT_MAX;
    _errorMsg=QString();
}
