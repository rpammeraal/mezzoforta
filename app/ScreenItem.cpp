#include <QDebug>

#include "Common.h"
#include "ScreenItem.h"

///	Ctors
ScreenItem::ScreenItem()
{
    _init();
}

ScreenItem::ScreenItem(const SBIDPtr& ptr)
{
    _init();
    _screenType=ScreenItem::screen_type_sbidbase;
    _ptr=ptr;
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

void
ScreenItem::updateSBIDBase(const SBIDPtr &ptr)
{
    if(this->screenType()==ScreenItem::screen_type_sbidbase)
    {
        this->_ptr=ptr;
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
            return (*(this->_ptr)==*(i._ptr)) && (this->_editFlag==i._editFlag);

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
operator<<(QDebug dbg, const ScreenItem& screenItem)
{
    dbg.nospace() << "ScreenItem:";

    switch(screenItem._screenType)
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
        dbg.nospace() << "SBIDBase [" << screenItem._ptr->operator QString() << "]";
        break;

    case ScreenItem::screen_type_songsearch:
        dbg.nospace() << "search [" << screenItem._searchCriteria << "]";
        break;
    }

    dbg.nospace()
            << " :subtabID=" << screenItem._subtabID
            << " :sortColumn=" << screenItem._sortColumn
            << " :editFlag=" << screenItem._editFlag
    ;

    return dbg.space();
}

///	Private methods

void
ScreenItem::_init()
{
    _screenType=ScreenItem::screen_type_invalid;
    _editFlag=0;
    _ptr=std::make_shared<SBIDBase>(SBIDBase());
    _searchCriteria=QString();
    _subtabID=INT_MAX;
    _sortColumn=INT_MAX;
    _errorMsg=QString();
}
