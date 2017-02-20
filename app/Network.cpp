#include "Network.h"

#include <QHostInfo>
#include <QNetworkConfiguration>
#include <QNetworkConfigurationManager>
#include <QNetworkInterface>
#include <QNetworkSession>
#include <QString>


#include "Common.h"

Network::Network()
{

}


QString
Network::hostName()
{
    return QHostInfo::localHostName();
}
