#include "Network.h"

#include <QHostInfo>


#include "Common.h"

Network::Network()
{

}

QString
Network::hostName()
{
    return QHostInfo::localHostName() + '.' + QHostInfo::localDomainName();
}
