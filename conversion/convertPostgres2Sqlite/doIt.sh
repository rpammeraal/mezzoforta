#!/bin/sh

NEWDBNAME=tmp.sqlite
TMPFILE=/tmp/dump

echo Remove $NEWDBNAME
/bin/rm -f $NEWDBNAME

echo Create schema
pg_dump -s -O bluebook \
	| grep -v '^SET' \
	| grep -v '^CREATE EXTENSION' \
	| grep -v '^COMMENT ON' \
	| grep -v '^ALTER TABLE ONLY' \
	| grep -v 'ADD CONSTRAINT' \
	| grep -v 'USING btree' \
	| grep -v '^REVOKE ALL' \
	| grep -v '^GRANT ALL' \
	| sed 's/::text//g' \
	| sed 's/::integer//g' \
	| grep -v '^\-\-' \
	| sed 's/CREATE TABLE index/CREATE TABLE "index"/' \
	> $TMPFILE

cat $TMPFILE |sqlite3 $NEWDBNAME

echo Insert data
echo "BEGIN;" > $TMPFILE
pg_dump -a --inserts bluebook \
	| grep -v '^SET' \
	| grep -v '^\-\-' \
	>> $TMPFILE
echo "END;" >> $TMPFILE

cat $TMPFILE |sqlite3 $NEWDBNAME
