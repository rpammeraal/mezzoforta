#include "Common.h"
#include "DataAccessLayer.h"
#include "SBModelPlaylist.h"

SBModelPlaylist::SBModelPlaylist(DataAccessLayer* d) : SBModel(d)
{
    applyFilter(QString(),0);
}

SBModelPlaylist::~SBModelPlaylist()
{
}

void
SBModelPlaylist::applyFilter(const QString& filter, const bool doExactSearch)
{
    SB_UNUSED(filter);
    SB_UNUSED(doExactSearch);

    QString q=QString("SELECT playlist_id, name AS \"title\", duration as \"duration\" FROM %1playlist ORDER BY name").arg(dal->_getSchemaName());

    QSqlQueryModel::setQuery(q,QSqlDatabase::database(dal->getConnectionName()));
    while(QSqlQueryModel::canFetchMore())
    {
        QSqlQueryModel::fetchMore();
    }

//    QSqlQueryModel* m=NULL;
//    if(_aim==NULL)
//    {
//        m=new QSqlQueryModel();
//    }
//    else
//    {
//        m=dynamic_cast<QSqlQueryModel *>(_aim);
//    }
//
//    m->setQuery(q,dal->db);
//    while(m->canFetchMore())
//    {
//        m->fetchMore();
//    }
//
//    _aim=m;
}

void
SBModelPlaylist::resetFilter()
{
    applyFilter(QString(),0);
}

//bool
//SBModelPlaylist::setData(const QModelIndex &index, const QVariant &value, int role)
//{
//    qDebug() << "SBModelPlaylist:setData called:index=" << index << ":value=" << value << ":role=" << role;
//    return 0;
//}
