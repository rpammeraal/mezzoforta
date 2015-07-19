#!/bin/sh

pg_dump -p 22500 songbase -U wwwrun -n rock -O -s -t 'rock.*' \
	| grep -v '^SET' \
	| grep -v '^\-\-' \
	> schema.txt
