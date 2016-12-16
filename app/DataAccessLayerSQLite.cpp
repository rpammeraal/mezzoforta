#include "DBManager.h"

#include "DataAccessLayerSQLite.h"

#include "Common.h"
#include "Configuration.h"
#include "Context.h"
#include "Network.h"
#include "SBMessageBox.h"

DataAccessLayerSQLite::DataAccessLayerSQLite()
{
}

bool
DataAccessLayerSQLite::createDatabase(const struct DBManager::DatabaseCredentials& credentials,const QString& musicLibraryPath)
{
    //	Check parameters
    QString errorString;
    bool errorFlag=0;

    //	1.	Length of path
    if(!errorFlag && credentials.sqlitePath.length()==0)
    {
        errorString="SQLite path parameter empty.";
        errorFlag=1;
    }
    qDebug() << SB_DEBUG_INFO << errorFlag << errorString;

    //	2.	Clear if exists
    QFileInfo fi(credentials.sqlitePath);
    if(!errorFlag && fi.exists())
    {
        QFile f(fi.absoluteFilePath());

        if(!f.remove())
        {
            errorString="Unable to remove.";
            errorFlag=1;
        }
    }
    qDebug() << SB_DEBUG_INFO << errorFlag << errorString;

    //	3.	Create actual database
    if(!errorFlag)
    {
        QSqlDatabase tmpdb = QSqlDatabase::addDatabase("QSQLITE",SB_TEMPORARY_CONNECTION_NAME);
        tmpdb.setDatabaseName(credentials.sqlitePath);

        if(!tmpdb.open())
        {
            errorString=QString("Cannot open database at %1").arg(credentials.sqlitePath);
            errorFlag=1;
        }
        qDebug() << SB_DEBUG_INFO;

        if(!errorFlag)
        {
            //	Create schema.
            DataAccessLayer dal(SB_TEMPORARY_CONNECTION_NAME);
        qDebug() << SB_DEBUG_INFO;

            QStringList SQL;

            SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE artist \n"
                "( \n"
                    "artist_id integer NOT NULL, \n"
                    "name character varying NOT NULL, \n"
                    "sort_name character varying NOT NULL, \n"
                    "www text, \n"
                    "notes text, \n"
                    "mbid character varying, \n"
                    "soundex character varying, \n"
                    "CONSTRAINT cc_artist_artist_id_check CHECK ((artist_id >= 0)), \n"
                    "CONSTRAINT cc_artist_name_check CHECK (((name) <> '')), \n"
                    "CONSTRAINT pk_artist PRIMARY KEY(artist_id) \n"
                ");\n"
            ));

            SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE artist_rel \n"
                "( \n"
                    "artist1_id integer NOT NULL, \n"
                    "artist2_id integer NOT NULL, \n"
                    "CONSTRAINT fk_artist_rel_artist1_id_artist_artist_id FOREIGN KEY (artist1_id) REFERENCES artist(artist_id), \n"
                    "CONSTRAINT fk_artist_rel_artist2_id_artist_artist_id FOREIGN KEY (artist2_id) REFERENCES artist(artist_id), \n"
                    "CONSTRAINT pk_artist_rel PRIMARY KEY(artist1_id,artist2_id) \n"
                "); \n"
            ));
            SQL.append("CREATE INDEX idx_artist_rel_a1 ON artist_rel   (artist1_id); ");
            SQL.append("CREATE INDEX idx_artist_rel_a2 ON artist_rel   (artist2_id); ");

            SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE song \n"
                "( \n"
                    "song_id integer NOT NULL, \n"
                    "title character varying NOT NULL, \n"
                    "notes text, \n"
                    "soundex character varying, \n"
                    "CONSTRAINT cc_song_song_id_check CHECK ((song_id >= 0)), \n"
                    "CONSTRAINT cc_song_title_check CHECK (((title) <> '')), \n"
                    "CONSTRAINT pk_song PRIMARY KEY(song_id) \n"
                "); \n"
            ));
            SQL.append("CREATE INDEX idx_song_title ON song   (title  ); ");

            SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE lyrics \n"
                "( \n"
                    "song_id integer NOT NULL, \n"
                    "lyrics text, \n"
                    "CONSTRAINT fk_lyrics_song_id_song_song_id FOREIGN KEY(song_id) REFERENCES song(song_id), \n"
                    "CONSTRAINT pk_lyrics PRIMARY KEY(song_id) \n"
                "); \n"
            ));
            SQL.append("CREATE INDEX idx_lyrics_song_id ON lyrics   (song_id); ");

            SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE performance \n"
                "( \n"
                    "song_id integer NOT NULL, \n"
                    "artist_id integer NOT NULL, \n"
                    "role_id smallint NOT NULL, \n"
                    "year integer, \n"
                    "notes text, \n"
                    "CONSTRAINT cc_performance_role_id_check CHECK (((role_id >= 0) AND (role_id <= 1))), \n"
                    "CONSTRAINT cc_performance_year_check CHECK (((year IS NULL) OR (year >= 1900))), \n"
                    "CONSTRAINT fk_performance_song_id_song_song_id FOREIGN KEY (song_id) REFERENCES song(song_id),"
                    "CONSTRAINT fk_performance_artist_id_artist_artist_id FOREIGN KEY (artist_id) REFERENCES artist(artist_id),"
                    "CONSTRAINT pk_performance PRIMARY KEY(song_id,artist_id) \n"
                "); \n"
            ));
            SQL.append("CREATE INDEX idx_performance_artist_id ON performance (artist_id); ");
            SQL.append("CREATE INDEX idx_performance_song_id ON performance (song_id); ");
            SQL.append("CREATE INDEX idx_performance_year ON performance (year); ");

            SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE record \n"
                "( \n"
                    "record_id integer NOT NULL, \n"
                    "artist_id integer NOT NULL, \n"
                    "title character varying NOT NULL, \n"
                    "media character varying(10) NOT NULL, \n"
                    "year integer, \n"
                    "genre character varying, \n"
                    "cddb_id character(8), \n"
                    "cddb_category character varying(20), \n"
                    "notes text, \n"
                    "CONSTRAINT cc_record_media_check CHECK (((media) <> '')), \n"
                    "CONSTRAINT cc_record_record_id_check CHECK ((record_id >= 0)), \n"
                    "CONSTRAINT cc_record_title_check CHECK (((title) <> '')), \n"
                    "CONSTRAINT fk_record_artist_id_artist_artist_id FOREIGN KEY (artist_id) REFERENCES artist(artist_id), \n"
                    "CONSTRAINT pk_record PRIMARY KEY(record_id) \n"
                "); \n"
            ));
            SQL.append("CREATE INDEX idx_record_artist_id ON record   (artist_id); ");
            SQL.append("CREATE INDEX idx_record_genre ON record   (genre  ); ");

            SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE record_performance \n"
                "( \n"
                    "song_id integer NOT NULL, \n"
                    "artist_id integer NOT NULL, \n"
                    "record_id integer NOT NULL, \n"
                    "record_position smallint NOT NULL, \n"
                    "op_song_id integer, \n"
                    "op_artist_id integer, \n"
                    "op_record_id integer, \n"
                    "op_record_position smallint, \n"
                    "duration time without time zone NOT NULL, \n"
                    "notes text, \n"
                    "CONSTRAINT cc_record_performance_song_id_check CHECK (((op_song_id = NULL) OR (op_song_id = song_id))), \n"
                    "CONSTRAINT cc_record_performance_artist_idcheck CHECK (((op_artist_id = NULL) OR (op_artist_id = artist_id))), \n"
                    "CONSTRAINT cc_record_performance_record_position_check CHECK ((record_position >= 0)), \n"
                    "CONSTRAINT cc_record_performance_song_id_song_song_id FOREIGN KEY (song_id) REFERENCES song(song_id), \n"
                    "CONSTRAINT fk_record_performance_artist_id_artist_artist_id FOREIGN KEY (artist_id) REFERENCES artist(artist_id), \n"
                    "CONSTRAINT fk_record_performance_record_id_record_record_id FOREIGN KEY (record_id) REFERENCES record(record_id), \n"

                    "CONSTRAINT fk_record_performance_record_performance FOREIGN KEY (op_song_id,op_artist_id,op_record_id,op_record_position) REFERENCES record_performance(song_id,artist_id,record_id,record_position), \n"
                    "CONSTRAINT fk_record_performance_online_performance FOREIGN KEY (op_song_id,op_artist_id,op_record_id,op_record_position) REFERENCES online_performance(song_id,artist_id,record_id,record_position), \n"
                    "CONSTRAINT pk_record_performance PRIMARY KEY(song_id,artist_id,record_id,record_position) \n"
                "); \n"
            ));
            SQL.append("CREATE INDEX idx_record_performance_artist_id ON record_performance (artist_id); ");
            SQL.append("CREATE INDEX idx_record_performance_record_id ON record_performance (record_id); ");

            SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE online_performance \n"
                "( \n"
                    "song_id integer NOT NULL, \n"
                    "artist_id integer NOT NULL, \n"
                    "record_id integer NOT NULL, \n"
                    "record_position smallint NOT NULL, \n"
                    "format_id integer NOT NULL, \n"
                    "path character varying NOT NULL, \n"
                    "source_id integer NOT NULL, \n"
                    "last_play_date timestamp without time zone, \n"
                    "play_order integer, \n"
                    "insert_order integer NOT NULL, \n"
                    "CONSTRAINT cc_online_performance_record_position_check CHECK ((record_position >= 0)), \n"
                    "CONSTRAINT fk_online_performance_online_performance FOREIGN KEY (song_id,artist_id,record_id,record_position) REFERENCES record_performance(song_id,artist_id,record_id,record_position), \n"
                    "CONSTRAINT pk_online_performance PRIMARY KEY(song_id,artist_id,record_id,record_position) \n"
                "); \n"
            ));
            SQL.append("CREATE INDEX idx_online_performance ON online_performance   (song_id, artist_id); ");
            SQL.append("CREATE INDEX idx_online_performance_a ON online_performance   (artist_id); ");
            SQL.append("CREATE INDEX idx_online_performance_a_lpd ON online_performance   (artist_id, last_play_date); ");
            SQL.append("CREATE UNIQUE INDEX idx_online_performance_io ON online_performance   (insert_order); ");
            SQL.append("CREATE INDEX idx_online_performance_lpd ON online_performance   (last_play_date); ");
            SQL.append("CREATE INDEX idx_online_performance_p ON online_performance   (path  ); ");
            SQL.append("CREATE INDEX idx_online_performance_po ON online_performance   (play_order); ");
            SQL.append("CREATE INDEX idx_online_performance_ri ON online_performance   (record_id); ");
            SQL.append("CREATE INDEX idx_online_performance_s ON online_performance   (song_id); ");

            SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE playlist \n"
                "( \n"
                    "playlist_id integer NOT NULL, \n"
                    "name character varying NOT NULL, \n"
                    "notes text, \n"
                    "created date NOT NULL, \n"
                    "duration interval, \n"
                    "updated date, \n"
                    "play_mode integer NOT NULL, \n"
                    "CONSTRAINT cc_playlist_play_mode_check CHECK (((play_mode >= 0) AND (play_mode <= 2))), \n"
                    "CONSTRAINT cc_playlist_playlist_id_check CHECK ((playlist_id >= 0)), \n"
                    "CONSTRAINT pk_playlist PRIMARY KEY(playlist_id) \n"
                "); \n"
            ));

            SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE playlist_composite \n"
                "( \n"
                    "playlist_id integer, \n"
                    "playlist_position integer, \n"
                    "timestamp date NOT NULL, \n"
                    "notes text, \n"
                    "playlist_playlist_id integer, \n"
                    "playlist_chart_id integer, \n"
                    "playlist_record_id integer, \n"
                    "playlist_artist_id integer, \n"
                    "CONSTRAINT cc_playlist_composite_check CHECK ((playlist_playlist_id <> playlist_id)), \n"
                    "CONSTRAINT cc_playlist_composite_playlist_position_check CHECK ((playlist_position > 0)), \n"
                    "CONSTRAINT fk_playlist_composite_playlist_playlist_id FOREIGN KEY(playlist_id) REFERENCES playlist(playlist_id), \n"
                    "CONSTRAINT pk_playlist_composite PRIMARY KEY(playlist_id,playlist_position) \n"
                "); \n"
            ));

            SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE playlist_performance \n"
                "( \n"
                    "playlist_id integer, \n"
                    "playlist_position integer, \n"
                    "song_id integer NOT NULL, \n"
                    "artist_id integer NOT NULL, \n"
                    "record_id integer, \n"
                    "record_position smallint, \n"
                    "collection_id integer, \n"
                    "c_position smallint, \n"
                    "timestamp date NOT NULL, \n"
                    "notes text, \n"
                    "CONSTRAINT cc_playlist_performance_playlist_position_check CHECK ((playlist_position > 0)) \n"
                    "CONSTRAINT fk_playlist_performance_playlist_playlist_id FOREIGN KEY(playlist_id) REFERENCES playlist(playlist_id), \n"
                    "CONSTRAINT fk_playlist_performance_record_performance FOREIGN KEY (song_id,artist_id,record_id,record_position) REFERENCES record_performance(song_id,artist_id,record_id,record_position), \n"
                    "CONSTRAINT pk_playlist_performance PRIMARY KEY(playlist_id,playlist_position) \n"
                "); \n"
            ));
            SQL.append("CREATE INDEX idx_playlist_performance_song_id ON playlist_performance   (song_id); ");

            SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE toplay \n"
                "( \n"
                    "song_id integer NOT NULL, \n"
                    "artist_id integer NOT NULL, \n"
                    "record_id integer NOT NULL, \n"
                    "record_position smallint NOT NULL, \n"
                    "insert_order integer NOT NULL, \n"
                    "play_order integer, \n"
                    "last_play_date timestamp without time zone, \n"
                    "CONSTRAINT fk_toplay_record_performance FOREIGN KEY (song_id,artist_id,record_id,record_position) REFERENCES record_performance(song_id,artist_id,record_id,record_position), \n"
                    "CONSTRAINT pk_toplay PRIMARY KEY(song_id,artist_id,record_id,record_position) \n"
                "); \n"
            ));
            SQL.append("CREATE INDEX idx_toplay_a ON toplay (artist_id); ");
            SQL.append("CREATE INDEX idx_toplay_lpd ON toplay (last_play_date); ");
            SQL.append("CREATE INDEX idx_toplay_op ON toplay (song_id, artist_id, record_id, record_position); ");
            SQL.append("CREATE INDEX idx_toplay_po ON toplay (play_order); ");

            SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE config_host \n"
                "( \n"
                    "host_id integer NOT NULL, \n"
                    "hostname varchar NOT NULL, \n"
                    "local_data_path integer NOT NULL, \n"
                    "CONSTRAINT pk_config_host PRIMARY KEY(host_id) \n"
                "); \n"
            ));

            SQL.append(QString(
                "INSERT INTO config_host "
                "( "
                    "host_id, "
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
                "CREATE TABLE article \n"
                "( \n"
                    "word varchar NOT NULL, \n"
                    "CONSTRAINT pk_article PRIMARY KEY(word) \n"
                "); \n"
            ));

            SQL.append("INSERT INTO article VALUES ('the'),('de'),('het'),('een'),('el'),('la'),('der'),('die'),('das') ");

            //	Ideally, go thru regular path to have this done.
            SQL.append("INSERT INTO ___SB_SCHEMA_NAME___artist (artist_id,name,sort_name) VALUES(0,'VARIOUS ARTISTS','VARIOUS ARTISTS')");
            SQL.append("INSERT INTO ___SB_SCHEMA_NAME___record (record_id,artist_id,title,media) VALUES(0,0,'UNKNOWN ALBUM','UNKNOWN ALBUM')");

            SQL.append(Common::escapeSingleQuotes(
                "CREATE TABLE digital_format \n"
                "( \n"
                    "format_id int NOT NULL, \n"
                    "name varchar NOT NULL, \n"
                    "extension varchar NOT NULL, \n"
                    "CONSTRAINT pk_digital_format PRIMARY KEY(format_id) \n"
                "); \n"
            ));

            SQL.append("INSERT INTO digital_format VALUES (0,'MP3 format','mp3')");
            SQL.append("INSERT INTO digital_format VALUES (1,'Wave format','wav')");
            SQL.append("INSERT INTO digital_format VALUES (2,'Ogg/Vorbis','ogg')");
            SQL.append("INSERT INTO digital_format VALUES (4,'Flac','flac')");

            if(dal.executeBatch(SQL,1,0,"Creating Database")==0)
            {
                errorFlag=1;
                errorString="Unable to create database";
            }

            Properties properties(&dal);
            properties.debugShow("createDatabase");
            properties.setConfigValue(Properties::sb_version,"20170101");
            properties.setConfigValue(Properties::sb_various_performer_id,"0");
            properties.setConfigValue(Properties::sb_unknown_album_id,"0");
            properties.setConfigValue(Properties::sb_performer_album_directory_structure_flag,"1");
            properties.setMusicLibraryDirectory(musicLibraryPath);
        }

        QSqlDatabase::removeDatabase(SB_TEMPORARY_CONNECTION_NAME);

        qDebug() << SB_DEBUG_INFO << errorFlag << errorString;
        if(!errorFlag)
        {
            //	Open the database system wide
            DBManager* dbm=Context::instance()->getDBManager();
            dbm->openDatabase(credentials);
            qDebug() << SB_DEBUG_INFO << dbm->databaseOpened();
        }
        qDebug() << SB_DEBUG_INFO;
    }
    if(errorFlag)
    {

    }
    qDebug() << SB_DEBUG_INFO << errorFlag;
    return errorFlag;
}
