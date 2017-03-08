#include "SBIDOnlinePerformance.h"

#include "Common.h"
#include "Context.h"
#include "SBSqlQueryModel.h"

///	Ctors, dtors
SBIDOnlinePerformance::SBIDOnlinePerformance(const SBIDOnlinePerformance& p):SBIDAlbumPerformance(p)
{
    _onlinePerformanceID=p._onlinePerformanceID;
    _duration           =p._duration;
    _path               =p._path;
}

SBIDOnlinePerformance::~SBIDOnlinePerformance()
{
}

//	Inherited methods
SBIDBase::sb_type
SBIDOnlinePerformance::itemType() const
{
    return SBIDBase::sb_type_online_performance;
}

QString
SBIDOnlinePerformance::genericDescription()
{
    return QString("Online Performance - %1")
    	.arg(this->_path)
    ;
}

void
SBIDOnlinePerformance::sendToPlayQueue(bool enqueueFlag)
{
    
}
