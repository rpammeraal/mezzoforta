#!/bin/bash

#AUTOID=INTEGER	#	sqlite
AUTOID=SERIAL	#	postgres

SQLITE_DST_DB=/Volumes/Home/roy/Projects/qt/songbase/db/actual\ databases/20170610.QT.sqlite
SQLITE_EXEC="sqlite3 -echo $SQLITE_DST_DB"

PSQL_EXEC="psql -p 22500 ROY -h mars -e "
PSQL_SCHEMA_NAME='rock'

SQL_SCHEMA="$PSQL_SCHEMA_NAME."
DB_EXEC=$PSQL_EXEC
echo $DB_EXEC $SQL_SCHEMA

for f in 00_setup.sql 01_artist.sql 02_artist_rel.sql 03_song.sql 04_lyrics.sql 05_performance.sql 06_record.sql 07_record_performance.sql 08_online_performance.sql 09_playlist.sql 10_playlist_detail.sql 11_toplay.sql 12_chart.sql 13_chart_performance.sql 99_finish.sql
do
	echo "Processing $f"
	cat $f | \
		sed s/---SQL_SCHEMA_NAME---/$SQL_SCHEMA/g | \
		sed s/---AUTOID---/$AUTOID/ | \
		$DB_EXEC
		#cat
	echo
done

