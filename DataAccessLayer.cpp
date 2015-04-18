#include "DataAccessLayer.h"

#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>

#include "Common.h"

//	Public methods
DataAccessLayer*
DataAccessLayer::createDataAccessLayer()
{
    DataAccessLayer* dal=new DataAccessLayer();
    lastRecentSQLError=dal->initDb();

    if (lastRecentSQLError.type() != QSqlError::NoError)
    {
        dal=NULL;
    }
    return dal;
}

QSqlQueryModel*
DataAccessLayer::getSonglist(const int playlistID, const QStringList& genres, const QString& filter,const bool doExactSearch)
{
    //	Main query
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
                            "___SB_SQL_QUERY_GENRE_JOIN___ "
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

    //	Handle filter
    if(doExactSearch==0)
    {
        //	search based on keywords
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
                    Common::escapeSingleQuotes(word);
                    if(whereClause.length()!=0)
                    {
                        whereClause+=" AND ";
                    }
                    whereClause+=" SB_KEYWORDS LIKE '%"+word+"%' ";
                }
            }
        }
    }
    else
    {
        //	exact search based on title or name
        QString arg=filter;
        Common::escapeSingleQuotes(arg);
        whereClause=QString(" songTitle='%1' OR artistName='%1' OR recordTitle='%1' ").arg(arg);
    }

    QString genreJoin="";

    QStringListIterator i(genres);
    while(i.hasNext())
    {
        QString arg=i.next().toLower();
        Common::escapeSingleQuotes(arg);
        genreJoin += QString("AND "
        "("
            "LOWER(LTRIM(RTRIM(genre)))=       '%1'   OR "  // complete match
            "LOWER(LTRIM(RTRIM(genre))) LIKE '%|%1'   OR "  // <genre>..|old genre
            "LOWER(LTRIM(RTRIM(genre))) LIKE   '%1|%' OR "  // old genre|<genre>..
            "LOWER(LTRIM(RTRIM(genre))) LIKE '%|%1|%'  "    // <genre>..|old genre|<genre>..
        ") ").arg(arg);

    }

    //	Handle playlist
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
    }

    //	Complete where clause
    if(whereClause.length()>0)
    {
        whereClause="WHERE "+whereClause;
    }

    query.replace("___SB_SQL_QUERY_WHERECLAUSE___",whereClause);
    query.replace("___SB_SQL_QUERY_PLAYLIST_JOIN___",playlistJoin);
    query.replace("___SB_SQL_QUERY_GENRE_JOIN___",genreJoin);

    qDebug() << "query=" << query;

    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery(query);

    while (model->canFetchMore())
    {
        model->fetchMore();
    }

    return model;
}

QSqlTableModel*
DataAccessLayer::getAllPlaylists()
{
    qDebug() << "getAllPlaylists:start";
    QSqlTableModel* m=new QSqlTableModel(this);
    m->setTable("playlist");
    m->setEditStrategy(QSqlTableModel::OnFieldChange);
    m->setSort(1,Qt::AscendingOrder);
    m->select();

    return m;
}

QSqlError DataAccessLayer::getLastRecentSQLError()
{
    return lastRecentSQLError;
}

QStandardItemModel*
DataAccessLayer::getGenres()
{
    _retrieveGenres();
    return &genreList;
}

void
DataAccessLayer::updateGenre(QModelIndex i)
{
    const int row=i.row();
    QString newGenre=genreList.item(row,0)->data(Qt::DisplayRole).toString();
    QString oldGenre=genreList.item(row,1)->data(Qt::DisplayRole).toString().toLower();

    Common::escapeSingleQuotes(newGenre);
    Common::escapeSingleQuotes(oldGenre);

    qDebug() << "dal:updateGenre"
        << ":oldGenre=" << oldGenre
        << ":newGenre=" << newGenre
    ;

    //	Update database
    QString s=QString(
        "UPDATE "
            "record "
        "SET "
            "genre=REPLACE(LOWER(genre),LOWER('%1'),'%2') "
        "WHERE "
            "LOWER(LTRIM(RTRIM(genre)))=       '%1'   OR "  // complete match
            "LOWER(LTRIM(RTRIM(genre))) LIKE '%|%1'   OR "  // <genre>..|old genre
            "LOWER(LTRIM(RTRIM(genre))) LIKE   '%1|%' OR "  // old genre|<genre>..
            "LOWER(LTRIM(RTRIM(genre))) LIKE '%|%1|%'  "    // <genre>..|old genre|<genre>..
    );


    s=s.arg(oldGenre).arg(newGenre);
    qDebug() << s;

    QSqlQuery q;
    q.exec(s);

    //	Update model
    QStandardItem *n=new QStandardItem(newGenre);
    genreList.setItem(row,1,n);
}

QSqlQueryModel*
DataAccessLayer::getCompleterModel()
{
    QString query=
        "SELECT DISTINCT title FROM song UNION "
        "SELECT DISTINCT title FROM record UNION "
        "SELECT DISTINCT name  FROM artist";

    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery(query);

    while (model->canFetchMore())
    {
        model->fetchMore();
    }

    return model;
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

void
DataAccessLayer::_retrieveGenres()
{
    //	Perform some data cleanup
    //	1.	Use '|' as separator
    QSqlQuery c1;
    c1.exec("UPDATE record SET genre=replace(genre,',','|') ");
    c1.exec("UPDATE record SET genre=replace(genre,'/','|') ");
    c1.exec("UPDATE record SET genre=replace(genre,'| ','|') ");
    c1.exec("UPDATE record SET genre=LTRIM(RTRIM(genre))");

    const QString q("SELECT DISTINCT genre FROM record");
    QSqlQuery query(q);

    //	Retrieve all possible tags, split by '/' and create list
    QStringList sl;
    while(query.next())
    {
        QString tag=query.value(0).toString().toLower();
        sl << tag.split('|');
    }
    sl.sort(Qt::CaseInsensitive);

    //	Iterate through list, add to model
    genreList.clear();

    QStringListIterator i(sl);
    int index=0;

    while(i.hasNext())
    {
        QString toInsert=i.next().trimmed();
        Common::toTitleCase(toInsert);
        if(toInsert.length()>0 && toInsert.at(0).isNull()==0)
        {
            //	Find if exists, otherwise insert
            QList<QStandardItem *> r=genreList.findItems(toInsert,Qt::MatchExactly);
            if(r.count()==0)
            {
                QStandardItem* item1=new QStandardItem(toInsert);
                QStandardItem* item2=new QStandardItem(toInsert);
                genreList.setItem(index,0,item1);
                genreList.setItem(index,1,item2);	//	maintain old value in here
                index++;
            }
        }
    }

    QVariant headerTitle("genre");
    genreList.setHeaderData(0,Qt::Horizontal,headerTitle);

    //	Hide 2nd column
}
