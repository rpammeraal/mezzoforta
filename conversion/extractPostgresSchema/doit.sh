#!/bin/sh

pg_dump -h songbase -p 22500 songbase -U wwwrun -n rock -O -s -t 'rock.*'  \
	| grep -v '^SET' \
	| grep -v '^\-\-' \
	| sed 's/::text//g' \
	| sed 's/::integer//g' \
	| sed 's/CREATE TABLE index/CREATE TABLE indexTable/' \
	| sed 's/CREATE TABLE password/CREATE TABLE passwordTable/' \
	| sed 's/JOIN public\./JOIN /' \
	| sed 's/DEFAULT now()/ /' \
	| sed 's/USING btree/ /' \
	| sed 's/varchar_ops/ /' \
	| sed 's/ON index/ /' \
	| grep -v 'public.firstchar' \
	> schema.txt

#	On remote
#	( echo "BEGIN ;" ; pg_dump songbase -p 22500 -U wwwrun --data-only --inserts -n rock ; echo "END;" ) | grep -v '^SET' | grep -v '^INSERT INTO index' | sed 's/, false)/, 0)/' > data.sql
