#include "DataAccessLayer.h"

#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>

//	Public methods
DataAccessLayer*
DataAccessLayer::createDataAccessLayer()
{
    DataAccessLayer* dal=new DataAccessLayer();
    lastRecentSQLError=dal->initDb();

    prevFilter='_';
    if (lastRecentSQLError.type() != QSqlError::NoError)
    {
        dal=NULL;
    }
    return dal;
}

QSqlQueryModel*
DataAccessLayer::getSonglist(const int playlistID,const QString& filter)
{
    QString query=
        "SELECT  "
            "SB_KEYWORDS, "
            "SB_SONG_ID, "
            "songTitle, "
            "SB_ARTIST_ID, "
            "artistName, "
            "SB_RECORD_ID, "
            "recordTitle  "
        "FROM "
            "( "
                "SELECT "
                    "s.song_id AS SB_SONG_ID, "
                    "s.title AS songTitle, "
                    "a.artist_id AS SB_ARTIST_ID, "
                    "a.name AS artistName, "
                    "r.record_id AS SB_RECORD_ID, "
                    "r.title AS recordTitle, "
                    "s.title || ' ' || a.name || ' ' || r.title  AS SB_KEYWORDS "
                "FROM "
                    "record_performance rp  "
                        "JOIN artist a ON  "
                            "rp.artist_id=a.artist_id "
                        "JOIN record r ON  "
                            "rp.record_id=r.record_id "
                        "JOIN song s ON  "
                            "rp.song_id=s.song_id "
                        "___SB_SQL_QUERY_PLAYLIST_JOIN___ "
            ") a "
        "___SB_SQL_QUERY_WHERECLAUSE___ "
        "ORDER BY 1 ";

    QString whereClause="";
    QString playlistJoin="";
    QString f=filter;
    QStringList fl;

    f.replace(QString("  "),QString(" "));	//	CWIP: replace with regex

    if(f.length()>=2)
    {
        fl=f.split(" ");

        QStringList::const_iterator constIterator;
        for (constIterator = fl.constBegin(); constIterator != fl.constEnd(); ++constIterator)
        {
            QString word=(*constIterator);

            word.replace(QString("  "),QString(" "));
            word.replace(QString("  "),QString(" "));
            if(word.length()>0)
            {
                if(whereClause.length()!=0)
                {
                    whereClause+=" AND ";
                }
                whereClause+=" SB_KEYWORDS LIKE '%"+word+"%' ";
            }
        }
    }

    if(playlistID>=0)
    {
        playlistJoin=QString(
            "JOIN playlist_performance pp ON "
                "a.artist_id=pp.artist_id AND "
                "r.record_id=pp.record_id AND "
                "s.song_id=pp.song_id "
            "JOIN playlist p ON "
                "pp.playlist_id=p.playlist_id AND "
                "p.playlist_id= %1 ").arg(playlistID);
        qDebug() << "playlistID=" << playlistID;
        qDebug() << "playlistJoin=" << playlistJoin;
    }
    if(whereClause.length()>0)
    {
        whereClause="WHERE "+whereClause;
    }

    query.replace("___SB_SQL_QUERY_WHERECLAUSE___",whereClause);
    query.replace("___SB_SQL_QUERY_PLAYLIST_JOIN___",playlistJoin);
    prevFilter=f;

    qDebug() << "query=" << query;

    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery(query);

    while (model->canFetchMore())
        model->fetchMore();

    return model;
}


QSqlQueryModel*
DataAccessLayer::getAllPlaylists()
{
    qDebug() << "getAllPlaylists:start";

    QString query=
        "SELECT  "
            "SB_KEYWORDS, "
            "SB_PLAYLIST_ID, "
            "playListName "
        "FROM "
            "( "
                "SELECT "
                    "pl.playlist_id AS SB_PLAYLIST_ID, "
                    "pl.name AS playListName, "
                    "pl.name AS SB_KEYWORDS  "
                "FROM "
                    "playlist pl  "
            ") a "
        "ORDER BY 1 ";

    qDebug() << "query final=" << query;
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery(query);

    while (model->canFetchMore())
        model->fetchMore();

    return model;
}

QSqlError DataAccessLayer::getLastRecentSQLError()
{
    return lastRecentSQLError;
}

QStandardItemModel*
DataAccessLayer::getGenres()
{
    const QString q("SELECT DISTINCT genre FROM record");
    QSqlQuery query(q);
    qDebug() << "query=" << q;

    //	Retrieve all possible tags, split by '/' and create list
    QStringList sl;
    while(query.next())
    {
        QString tag=query.value(0).toString();
        tag.replace(",","/");
        sl << tag.split('/');
    }
    sl.removeDuplicates();
    sl.sort(Qt::CaseInsensitive);
    qDebug() << "sl=" << sl;

    //	Iterate through list, add to model
    QStandardItemModel* genreList=new QStandardItemModel();
    QStringListIterator i(sl);
    while(i.hasNext())
    {
        QString toInsert=i.next().trimmed();
        toInsert=toInsert.at(0).toUpper()+toInsert.mid(1);	//	Poor man's Title Case...
        if(toInsert.length()>0 && toInsert.at(0).isNull()==0)
        {
            qDebug() << "toInsert.at(0)='" << toInsert.at(0).isNull();
            qDebug() << "toInsert='" << toInsert << "':length=" << toInsert.length();
            QStandardItem* item=new QStandardItem(toInsert);
            genreList->appendRow(item);
        }
    }

    return genreList;


    QStandardItemModel* genreTree=new QStandardItemModel();
    QStandardItem* parentItem=NULL;

    while(query.next())
    {
        QString i=query.value(0).toString();
        if(i.length()!=0)
        {
            parentItem=genreTree->invisibleRootItem();
            qDebug() << i << "start";

            QStringList l=i.split('/');

            for(int j=0;j<l.size();j++)
            {
                QString item=l.at(j);
                item=item.at(0).toUpper()+item.mid(1);	//	Poor man's Title Case...


                qDebug() << i << "looking for " << item << " at level " << j;

                if(j==0)
                {
                    //	Level 0
                    QList<QStandardItem *> found = genreTree->findItems(item,Qt::MatchExactly,j);

                    if(found.count()==0)
                    {
                        QStandardItem* toInsert=new QStandardItem(item);
                        toInsert->setText(item);
                        qDebug() << i << "to be inserted(0)=" << toInsert->data(Qt::DisplayRole);

                        parentItem->appendRow(toInsert);
                        parentItem=toInsert;
                        qDebug() << i << "insert(0)" << item << ":parentItem now set to" << parentItem->data(Qt::DisplayRole);
                    }
                    else
                    {
                        parentItem=found.at(0);
                        qDebug() << i << "found node(0)" << item << ":parentItem now set to" << parentItem->data(Qt::DisplayRole);
                    }
                }
                else
                {
                    //	Given a parentItem, start searching in parentItem for item
                    QStandardItem* c=NULL;
                    qDebug() << i << "start traversing " << parentItem->data(Qt::DisplayRole);
                    for(int k=0;k<parentItem->rowCount();k++)
                    {
                        c=parentItem->child(k,0);

                        //	NEED TO DO COMPARISON BETWEEN DATA OF C AND WHAT WE ARE LOOKING FOR.
                        //	IF FOUND, EXIT LOOP with BREAK

                        QVariant v=c->data(Qt::DisplayRole);
                        qDebug() << i <<  "current " << v.toString() << ", finding " << item ;
                        if(v.toString()==item)
                        {
                            qDebug() << i <<  "current " << v.toString() << ", FOUND";
                            break;
                        }
                    }
                    if(c==NULL)
                    {
                        QStandardItem* toInsert=new QStandardItem(item);
                        toInsert->setText(item);

                        parentItem->appendRow(toInsert);
                        parentItem=toInsert;
                        qDebug() << i << "insert(n)" << item;
                    }
                    else
                    {
                        parentItem=c;
                        qDebug() << i << "insert(n)" << item << ":parentItem now set to" << parentItem->data(Qt::DisplayRole);
                    }
                }
            }
        }
    }

    return genreTree;
}

DataAccessLayer::~DataAccessLayer()
{
}

//	Private methods
DataAccessLayer::DataAccessLayer()
{
}

QSqlError
DataAccessLayer::initDb()
{
    //	Find out previously used database.
    QSettings settings;
    QString databasePath=settings.value(SB_DATABASE_ENTRY).toString();
    qDebug() << "Initial DatabasePath:" << databasePath;

    //	Test if file exists
    QFile f(databasePath);

    if(databasePath=="" || f.exists()==0)
    {
        QMessageBox notification;
        if(databasePath=="")
        {
            notification.setText("Select a database to use");
        }
        else
        {
            notification.setText("The previously used database does not longer exists.");
        }
        notification.exec();
        databasePath=QFileDialog::getOpenFileName(NULL,tr("Open Database"));
    }
    qDebug() << "Selected DatabasePath:" << databasePath;

    //	Exit app if user does not select a database
    if(databasePath.length()==0)
    {
        return QSqlError(tr("No database selected"),tr(""),QSqlError::ConnectionError,tr("Whatever"));
    }
    qDebug() << "Continuing (databasePath.length=" << databasePath.length() << ")";

    //	Open database
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(databasePath);

    if (!db.open())
    {
        return db.lastError();
    }

    //	Set path in settings
    settings.setValue(SB_DATABASE_ENTRY,databasePath);

    databasePath=settings.value(SB_DATABASE_ENTRY).toString();
    qDebug() << "Saved DatabasePath:" << databasePath;

    return QSqlError();
}
