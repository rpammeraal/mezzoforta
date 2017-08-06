#!/bin/bash

AUTOID=SERIAL	#	postgres

PSQL_EXEC="psql -p 22500 songbase -h mars -e "
PSQL_SCHEMA_NAME='rock'

SQL_SCHEMA="$PSQL_SCHEMA_NAME."
DB_EXEC=$PSQL_EXEC
echo $DB_EXEC $SQL_SCHEMA

for f in 01_config.sql 02_digital_format.sql 03_greatest_hits_record.sql
do
	echo "Processing $f"
	cat $f | \
		sed s/---SQL_SCHEMA_NAME---/$SQL_SCHEMA/g | \
		sed s/---AUTOID---/$AUTOID/ | \
		$DB_EXEC
		#cat
	echo
done
