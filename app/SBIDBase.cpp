#include <QDebug>

#include "Cache.h"
#include "CacheManager.h"
#include "Context.h"
#include "SBIDBase.h"
#include "SBIDAlbum.h"
#include "SBIDChart.h"
#include "SBIDChartPerformance.h"
#include "SBIDPerformer.h"
#include "SBIDPlaylist.h"
#include "SBIDPlaylistDetail.h"
#include "SBIDOnlinePerformance.h"
#include "SBIDSong.h"


SBIDBase::SBIDBase(const SBIDBase &c)
{
    _copy(c);
}

SBIDBase::SBIDBase(SBKey::ItemType itemType, int itemID)
{
    _init();
    _key=SBKey(itemType,itemID);
}

SBIDBase::~SBIDBase()
{
}

///	Public methods

///	Public virtual methods (Methods that only apply to subclasseses)
void
SBIDBase::showDebug(const QString& title) const
{
    qDebug() << SB_DEBUG_INFO << title;
    qDebug() << SB_DEBUG_INFO << "key" << key();
}

bool
SBIDBase::operator ==(const SBIDBase& i) const
{
    return i.key()==this->key();
}

bool
SBIDBase::operator !=(const SBIDBase& i) const
{
    return !(this->operator==(i));
}

SBIDBase::operator QString() const
{
    return this->key().toString() + ":" + this->genericDescription();
}

void
SBIDBase::setToReloadFlag()
{
    SB_RETURN_VOID_IF_NULL(_owningCache);
    qDebug() << SB_DEBUG_INFO << this->key();
    _owningCache->addToReloadList(this->key());
}


///	Protected
void
SBIDBase::clearChangedFlag()
{
    _changedFlag=0;
}

void
SBIDBase::clearReloadFlag()
{
    _reloadFlag=0;
}

void
SBIDBase::rollback()
{
    clearChangedFlag();
}

void
SBIDBase::setChangedFlag()
{
    _changedFlag=1;
    SB_RETURN_VOID_IF_NULL(_owningCache);
    _owningCache->addChangedKey(this->key());
}

void
SBIDBase::setDeletedFlag()
{
    _deletedFlag=1;
    setChangedFlag();
}

void
SBIDBase::_copy(const SBIDBase &c)
{
    _errorMsg=c._errorMsg;
    _deletedFlag=c._deletedFlag;
    _changedFlag=c._changedFlag;
    _id=-2;	//	do NOT copy -- identifies copy
    _sb_mbid=c._sb_mbid;
    _sb_model_position=c._sb_model_position;
    _reloadFlag=c._reloadFlag;
    _url=c._url;
    _wiki=c._wiki;
    _owningCache=NULL;	//	do NOT copy -- identifies copy
    _key=c._key;
}

void
SBIDBase::getSemaphore()
{
    _mutex.lock();
}

void
SBIDBase::releaseSemaphore()
{
    _mutex.unlock();
}

void
SBIDBase::setReloadFlag()
{
    _reloadFlag=1;
}

///	PRIVATE
SBIDBase::SBIDBase()
{
    _init();
}

void
SBIDBase::_init()
{
    QString e;

    //	Private
    _key=SBKey();
    _changedFlag=0;
    _deletedFlag=0;
    _id=Common::nextID();
    _sb_mbid=e;
    _sb_model_position=-1;
    _reloadFlag=0;
    _url=e;
    _wiki=e;

    //	Protected
    _errorMsg=e;
}
