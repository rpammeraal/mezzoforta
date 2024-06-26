#include "DBManager.h"

#include "DataAccessLayerSQLite.h"

#include "Common.h"
#include "Context.h"
#include "Network.h"
#include "SBMessageBox.h"
#include "SqlQuery.h"

DataAccessLayerSQLite::DataAccessLayerSQLite()
{
}

DataAccessLayerSQLite::DataAccessLayerSQLite(const QString& connectionName) : DataAccessLayer(connectionName)
{
}

DataAccessLayerSQLite::~DataAccessLayerSQLite()
{
}

//	Implemented virtual functions
QStringList
DataAccessLayerSQLite::availableSchemas() const
{
    QStringList sl;
    return sl;
}

QString
DataAccessLayerSQLite::getLeft(const QString& str,const QString& len) const
{
    qDebug() << SB_DEBUG_INFO;
    return QString("SUBSTR(%1,0,%2)").arg(str).arg(len);
}

void
DataAccessLayerSQLite::logSongPlayed(bool radioModeFlag,SBIDOnlinePerformancePtr opPtr) const
{
    SB_RETURN_VOID_IF_NULL(opPtr);

    QString q=QString
            (
                "INSERT INTO play_history "
                "( "
                    "artist_name, "
                    "record_title, "
                    "record_position, "
                    "song_title, "
                    "path, "
                    "played_by_radio_flag, "
                    "play_datetime "
                ") "
                "VALUES "
                "( "
                    "'%1', "
                    "'%2', "
                    "%3, "
                    "'%4', "
                    "'%5', "
                    "%6::BOOL, "
                    "DATETIME('now','localtime','-13 seconds')"
                ") "
            )
            //	SELECT datetime('now');

                .arg(Common::escapeSingleQuotes(opPtr->songPerformerName()))
                .arg(Common::escapeSingleQuotes(opPtr->albumTitle()))
                .arg(opPtr->albumPosition())
                .arg(Common::escapeSingleQuotes(opPtr->songTitle()))
                .arg(Common::escapeSingleQuotes(opPtr->path()))
                .arg(radioModeFlag?"1":"0")
    ;


    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    dal->customize(q);
    SqlQuery insert(q,db);
    Q_UNUSED(insert);
}

QString
DataAccessLayerSQLite::retrieveLastInsertedKeySQL() const
{
    return QString("SELECT last_insert_rowid();");
}


bool
DataAccessLayerSQLite::supportSchemas() const
{
    return 0;
}

//	Static functions
bool
DataAccessLayerSQLite::createDatabase(const struct DBManager::DatabaseCredentials& credentials,const QString& musicLibraryPath)
{
    Q_UNUSED(credentials);
    //	Check parameters
    QString errorMsg;
    bool errorFlag=0;

    //	1.	Length of path
    if(!errorFlag && credentials.sqlitePath.length()==0)
    {
        errorMsg="SQLite path parameter empty.";
        errorFlag=1;
    }

    //	2.	Clear if exists
    QFileInfo fi(credentials.sqlitePath);
    if(!errorFlag && fi.exists())
    {
        QFile f(fi.absoluteFilePath());

        if(!f.remove())
        {
            errorMsg="Unable to remove.";
            errorFlag=1;
            qDebug() << SB_DEBUG_ERROR << errorMsg;
			return 0;
        }
    }

    //	3.	Create actual database
    if(!errorFlag)
    {
        QSqlDatabase tmpdb = QSqlDatabase::addDatabase("QSQLITE",SB_TEMPORARY_CONNECTION_NAME);
        tmpdb.setDatabaseName(credentials.sqlitePath);

        if(!tmpdb.open())
        {
            errorMsg=QString("Cannot open database at %1").arg(credentials.sqlitePath);
            errorFlag=1;
        }

        if(!errorFlag)
        {
            //	Create schema.
            DataAccessLayerSQLite dal(SB_TEMPORARY_CONNECTION_NAME);

            QStringList SQL;

            SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE artist \n"
                "( \n"
                    "artist_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, \n"
                    "name      CHARACTER VARYING NOT NULL, \n"
                    "sort_name CHARACTER VARYING NOT NULL, \n"
                    "www       TEXT, \n"
                    "notes     TEXT, \n"
                    "mbid      CHARACTER VARYING, \n"
                    "soundex   CHARACTER VARYING, \n"
                    "CONSTRAINT cc_artist_artist_id_check CHECK ((artist_id >= 0)), \n"
                    "CONSTRAINT cc_artist_name_check CHECK (((name) <> '')) \n"
                ");\n"
            ));


            SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE artist_rel \n"
                "( \n"
                    "artist_rel_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, \n"
                    "artist1_id    INTEGER NOT NULL, \n"
                    "artist2_id    INTEGER NOT NULL, \n"
                    "CONSTRAINT fk_artist_rel_artist1_id_artist_artist_id FOREIGN KEY (artist1_id) REFERENCES artist(artist_id), \n"
                    "CONSTRAINT fk_artist_rel_artist2_id_artist_artist_id FOREIGN KEY (artist2_id) REFERENCES artist(artist_id) \n"
                "); \n"
            ));
            SQL.append("CREATE INDEX idx_artist_rel_a1 ON artist_rel   (artist1_id); ");
            SQL.append("CREATE INDEX idx_artist_rel_a2 ON artist_rel   (artist2_id); ");

            SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE song \n"
                "( \n"
                    "song_id                 INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, \n"
                    "original_performance_id INT NULL, \n"
                    "title                   CHARACTER VARYING NOT NULL, \n"
                    "notes                   TEXT NULL, \n"
                    "soundex                 CHARACTER VARYING NULL, \n"
                    "CONSTRAINT cc_song_song_id_check CHECK ((song_id >= 0)), \n"
                    "CONSTRAINT cc_song_title_check CHECK (((title) <> '')), \n"
                    "CONSTRAINT fk_song_original_performance_id_performance_performance_id FOREIGN KEY (original_performance_id) REFERENCES performance(performance_id)\n"
                "); \n"
            ));
            SQL.append("CREATE INDEX idx_song_title ON song (title); ");
            SQL.append("CREATE INDEX idx_song_soundex ON song (soundex); ");

            SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE lyrics \n"
                "( \n"
                    "song_id INTEGER PRIMARY KEY NOT NULL, \n"
                    "lyrics  TEXT, \n"
                    "CONSTRAINT fk_lyrics_song_id_song_song_id FOREIGN KEY(song_id) REFERENCES song(song_id) \n"
                "); \n"
            ));

            SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE performance \n"
                "( \n"
                    "performance_id                  INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, \n"
                    "song_id                         INTEGER NOT NULL, \n"
                    "artist_id                       INTEGER NOT NULL, \n"
                    "preferred_record_performance_id INTEGER NULL, \n"
                    "year                            INTEGER, \n"
                    "notes                           TEXT, \n"
                    "CONSTRAINT cc_performance_year_check CHECK (((year IS NULL) OR (year >= 1900))), \n"
                    "CONSTRAINT fk_performance_artist_id_artist_artist_id FOREIGN KEY (artist_id) REFERENCES artist(artist_id),\n"
                    "CONSTRAINT fk_performance_song_id_song_song_id FOREIGN KEY (song_id) REFERENCES song(song_id), \n"
                    "CONSTRAINT fk_performance_preferred_record_performance_id_record_performance_record_performance_id FOREIGN KEY (preferred_record_performance_id) REFERENCES performance(performance_id) \n"
                "); \n"
            ));

            SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE record \n"
                "( \n"
                    "record_id     INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, \n"
                    "artist_id     INTEGER NOT NULL, \n"
                    "title         CHARACTER VARYING NOT NULL, \n"
                    "media         CHARACTER VARYING(10) NOT NULL, \n"
                    "year          INTEGER, \n"
                    "genre         CHARACTER VARYING, \n"
                    "cddb_id       CHARACTER(8), \n"
                    "cddb_category CHARACTER VARYING(20), \n"
                    "notes         TEXT, \n"
                    "CONSTRAINT cc_record_media_check CHECK (((media) <> '')), \n"
                    "CONSTRAINT cc_record_record_id_check CHECK ((record_id >= 0)), \n"
                    "CONSTRAINT cc_record_title_check CHECK (((title) <> '')), \n"
                    "CONSTRAINT fk_record_artist_id_artist_artist_id FOREIGN KEY (artist_id) REFERENCES artist(artist_id) \n"
                "); \n"
            ));
            SQL.append("CREATE INDEX idx_record_title ON record (title); ");
            SQL.append("CREATE INDEX idx_record_genre ON record (genre); ");

            SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE record_performance \n"
                "( \n"
                    "record_performance_id           INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, \n"
                    "performance_id                  INTEGER NOT NULL, \n"
                    "preferred_online_performance_id INTEGER null, \n"
                    "record_id                       INTEGER NOT NULL, \n"
                    "record_position                 SMALLINT NOT NULL, \n"
                    "duration                        TIME WITHOUT TIME ZONE NOT NULL, \n"
                    "notes                           TEXT, \n"
                    "CONSTRAINT cc_record_performance_record_position_check CHECK ((record_position >= 0)), \n"
                    "CONSTRAINT fk_record_performance_record_id_record_record_id FOREIGN KEY (record_id) REFERENCES record(record_id), \n"
                    "CONSTRAINT fk_record_performance_preferred_online_performance_id_online_performance_online_performance_id FOREIGN KEY (preferred_online_performance_id) REFERENCES online_performance(online_performance_id) \n"
                "); \n"
            ));

            SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE online_performance \n"
                "( \n"
                    "online_performance_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, \n"
                    "record_performance_id INTEGER NOT NULL, \n"
                    "path                  CHARACTER VARYING NOT NULL, \n"
                    "last_play_date        TIMESTAMP without time zone, \n"
                    "CONSTRAINT fk_online_performance_record_performance_id_record_performance_record_performance_id FOREIGN KEY (record_performance_id) REFERENCES record_performance(record_performance_id) \n"
                "); \n"
            ));
            SQL.append("CREATE INDEX idx_online_performance_last_play_date ON online_performance (last_play_date); ");

			SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE playlist \n"
                "( \n"
                    "playlist_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, \n"
                    "name        CHARACTER VARYING NOT NULL, \n"
                    "notes       TEXT, \n"
                    "created     DATE NOT NULL, \n"
                    "duration    INTERVAL, \n"
                    "updated     DATE, \n"
                    "play_mode   INTEGER NOT NULL, \n"
                    "CONSTRAINT cc_playlist_play_mode_check CHECK (((play_mode >= 0) AND (play_mode <= 2))), \n"
                    "CONSTRAINT cc_playlist_playlist_id_check CHECK ((playlist_id >= 0)) \n"
                "); \n"
            ));

			SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE playlist_detail \n"
                "( \n"
                    "playlist_detail_id    INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, \n"
                    "playlist_id           INTEGER NOT NULL, \n"
                    "playlist_position     INTEGER NOT NULL, \n"
                    "online_performance_id INTEGER NULL, \n"
                    "child_playlist_id     INTEGER NULL, \n"
                    "chart_id              INTEGER NULL, \n"
                    "record_id             INTEGER NULL, \n"
                    "artist_id             INTEGER NULL, \n"
                    "timestamp             DATE NOT NULL, \n"
                    "notes                 TEXT, \n"
                    "CONSTRAINT cc_playlist_detail_playlist_position_check CHECK ((playlist_position > 0)) \n"
                    "CONSTRAINT fk_playlist_detail_playlist_playlist_id FOREIGN KEY(playlist_id) REFERENCES playlist(playlist_id), \n"
                    "CONSTRAINT fk_playlist_detail_online_performance_online_performance_id FOREIGN KEY (online_performance_id) REFERENCES online_performance(online_performance_id), \n"
                    "CONSTRAINT fk_playlist_detail_playlist_child_playlist_id FOREIGN KEY(child_playlist_id) REFERENCES playlist(playlist_id), \n"
                    "CONSTRAINT fk_playlist_detail_record_record_id FOREIGN KEY (record_id) REFERENCES record(record_id), \n"
                    "CONSTRAINT fk_playlist_detail_artist_artist_id FOREIGN KEY (artist_id) REFERENCES artist(artist_id) \n"
                "); \n"
            ));

			SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE chart \n"
                "( \n"
                    "chart_id     INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, \n"
                    "name         CHARACTER VARYING NOT NULL, \n"
                    "release_date DATE NOT NULL, \n"
                    "notes        TEXT \n"
                "); \n"
            ));

			SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE chart_performance \n"
                "( \n"
                    "chart_performance_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, \n"
                    "chart_id              INTEGER NOT NULL, \n"
                    "performance_id        INTEGER NOT NULL, \n"
                    "chart_position        INTEGER NOT NULL, \n"
                    "NOTES                 VARCHAR NULL, \n"
                    "CONSTRAINT fk_chart_id_chart_chart_id FOREIGN KEY (chart_id) REFERENCES chart(chart_id) \n"
                    "CONSTRAINT fk_performance_id_performance_performance_id FOREIGN KEY (performance_id) REFERENCES performance(performance_id) \n"
                "); \n"
            ));

			SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE config_host \n"
                "( \n"
                    "config_host_id  INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, \n"
                    "hostname        VARCHAR NOT NULL, \n"
                    "local_data_path INTEGER NOT NULL \n"
                "); \n"
            ));

			SQL.append(QString(
                "INSERT INTO config_host "
                "( "
                    "config_host_id, "
                    "hostname, "
                    "local_data_path "
                ") "
                "VALUES "
                "( "
                    "0, "
                    "'%1', "
                    "'%2' "
                ")"
            )
                .arg(Common::escapeSingleQuotes(Network::hostName()))
                .arg(Common::escapeSingleQuotes(musicLibraryPath))
            );

			SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE configuration \n"
                "( \n"
                    "keyword varchar NOT NULL, \n"
                    "value varchar NOT NULL, \n"
                    "CONSTRAINT pk_config_host PRIMARY KEY(keyword,value) \n"
                "); \n"
            ));

			SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE genre \n"
                "( \n"
                    "genrename varchar NOT NULL \n"
                "); \n"
            ));

            SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE article \n"
                "( \n"
                    "word varchar NOT NULL, \n"
                    "CONSTRAINT pk_article PRIMARY KEY(word) \n"
                "); \n"
            ));

            SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE play_history \n"
                "( \n"
                    "artist_name VARCHAR NOT NULL, \n"
                    "record_title VARCHAR NOT NULL, \n"
                    "record_position VARCHAR NOT NULL, \n"
                    "song_title VARCHAR NOT NULL, \n"
                    "path VARCHAR NOT NULL, \n"
                    "played_by_radio_flag BOOL NOT NULL, \n"
                    "play_datetime TIMESTAMP NOT NULL \n"
                "); \n"
            ));

            SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE match \n"
                "( \n"
                    "file_path                          VARCHAR,\n"
                    "parent_directory_name              VARCHAR,\n"
                    "parent_directory_path              VARCHAR,\n"
                    "absolute_parent_directory_path     VARCHAR,\n"
                    "extension                          VARCHAR,\n"

                    "record_title                       VARCHAR, \n"
                    "record_position                    INT, \n"
                    "record_id                          INTEGER, \n"
                    "record_match_score                 INTEGER, \n"

                    "record_artist_name                 VARCHAR, \n"
                    "record_artist_name_cmp             VARCHAR, \n"
                    "record_artist_id                   INTEGER, \n"
                    "record_artist_soundex              VARCHAR, \n"
                    "record_artist_match_score          INTEGER, \n"

                    "song_artist_name                   VARCHAR, \n"
                    "song_artist_name_cmp               VARCHAR, \n"
                    "song_artist_id                     INTEGER, \n"
                    "song_artist_soundex                VARCHAR, \n"
                    "song_artist_match_score            INTEGER, \n"

                    "song_title                         VARCHAR, \n"
                    "song_id                            INTEGER, \n"
                    "song_soundex                       VARCHAR, \n"
                    "song_match_score                   INTEGER \n"
                "); \n"
            ));

            SQL.append("INSERT INTO article VALUES ('the'),('de'),('het'),('een'),('el'),('la'),('der'),('die'),('das') ");

            //	Ideally, go thru regular path to have this done.
            SQL.append("INSERT INTO ___SB_SCHEMA_NAME___artist (artist_id,name,sort_name) VALUES(0,'VARIOUS ARTISTS','VARIOUS ARTISTS')");
            SQL.append("INSERT INTO ___SB_SCHEMA_NAME___record (record_id,artist_id,title,media) VALUES(0,0,'UNKNOWN ALBUM','UNKNOWN ALBUM')");

			SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE greatest_hits_record \n"
                "( \n"
                    "greatest_hits_record_id INTEGER PRIMARY KEY NOT NULL, \n"
                    "title                   VARCHAR NOT NULL \n"
                "); \n"
            ));

            SQL.append("INSERT INTO greatest_hits_record VALUES (0,'Greatest Hits')");
            SQL.append("INSERT INTO greatest_hits_record VALUES (1,'The Best Of')");
            SQL.append("INSERT INTO greatest_hits_record VALUES (2,'Best Of')");
            SQL.append("INSERT INTO greatest_hits_record VALUES (3,'The Very Best Of')");
            SQL.append("INSERT INTO greatest_hits_record VALUES (4,'Het Beste Van')");
            SQL.append("INSERT INTO greatest_hits_record VALUES (5,'The Ultimate Collection')");
            SQL.append("INSERT INTO greatest_hits_record VALUES (6,'Ultimate Collection')");
            SQL.append("INSERT INTO greatest_hits_record VALUES (7,'Gold')");
            SQL.append("INSERT INTO greatest_hits_record VALUES (8,'Unplugged')");
            SQL.append("INSERT INTO greatest_hits_record VALUES (9,'Collection')");
            SQL.append("INSERT INTO greatest_hits_record VALUES (10,'The Singles')");

			SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE artist_match \n"
                "( \n"
                    "artist_correct_name     VARCHAR NOT NULL, \n"
                    "artist_alternative_name VARCHAR NOT NULL \n"
                "); \n"
            ));
            SQL.append("CREATE UNIQUE INDEX ui_artist_match ON artist_match (artist_alternative_name,artist_correct_name);");

            //	Reset properties
            PropertiesPtr properties=Context::instance()->properties();
            properties.reset();

            if(dal.executeBatch(SQL,QString(),"Initializing Database",QString(),1,0)==0)
            {
                errorFlag=1;
                errorMsg="Unable to create database";
				qDebug() << SB_DEBUG_ERROR << "Unable to create database";
            }

            properties=Properties::createProperties(&dal);
            properties->debugShow("createDatabase");
            properties->setMusicLibraryDirectory(musicLibraryPath);

            Context::instance()->setProperties(properties);
        }

        QSqlDatabase::removeDatabase(SB_TEMPORARY_CONNECTION_NAME);

        if(!errorFlag)
        {
            //	Open the database system wide
            DBManager* dbm=Context::instance()->dbManager();
            dbm->openDatabase(credentials);
        }
    }
    if(errorFlag)
    {
        SBMessageBox::databaseErrorMessageBox("Something happened...!",errorMsg);
        qDebug() << SB_DEBUG_ERROR << errorMsg;
    }
    return !errorFlag;
}
