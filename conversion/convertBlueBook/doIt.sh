#!/bin/sh

TMPDBNAME=bluebook
TMPFILENAME=/tmp/master.out

#	convert to UNIX format, remove 1st line
perl -pe 's/\r\n|\n|\r/\n/g' \
	| sed '1d' \
	| iconv -c -f utf-8 -t ascii \
	| sed 's/	/|/g' \
	| sed 's/\"//g' \
	> $TMPFILENAME

#	check for database to exist, if it does, remove
EXISTS=`psql --list|grep $TMPDBNAME|wc -l`

if [ $EXISTS == 1 ] 
then
	echo Dropping database $TMPBNAME
	dropdb $TMPDBNAME
fi

echo Creating database $TMPBNAME
createdb $TMPDBNAME

echo Create songbase tables
psql $TMPDBNAME -f sql/00.schema.sql

echo Create staging table
psql $TMPDBNAME -f sql/01.createTables.sql

echo Importing data
psql $TMPDBNAME -c "copy staging from '$TMPFILENAME' with (format csv, delimiter E'|')"

echo Adding additional columns
psql $TMPDBNAME -f sql/02.alterStagingTable.sql

echo Determine IDs
psql $TMPDBNAME -f sql/03.determineIDs

echo Inserting data
psql $TMPDBNAME -f sql/04.populate

#echo Deleting staging table
#psql $TMPDBNAME -c "DROP TABLE staging"
