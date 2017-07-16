DROP TABLE IF EXISTS conversion
;

CREATE TABLE conversion 
(
	artist_id INT, 
	song_id INT, 
	record_id INT,
	record_position INT,
	path VARCHAR,
	performance_id INT,
	record_performance_id INT,
	online_performance_id INT,
	inserted_by VARCHAR
);

--	online_performance
INSERT INTO conversion
(
	song_id,
	artist_id,
	record_id,
	record_position,
	path,
	inserted_by
)
SELECT DISTINCT
	song_id,
	artist_id,
	record_id,
	record_position,
	path,
	'online_performance'
FROM
	---SQL_SCHEMA_NAME---online_performance
;

--	playlist_performance
INSERT INTO conversion
(
	artist_id,
	song_id,
	record_id,
	record_position,
	inserted_by
)
SELECT DISTINCT
	pp.artist_id,
	pp.song_id,
	pp.record_id,
	pp.record_position,
	'playlist_performance'
FROM
	---SQL_SCHEMA_NAME---playlist_performance pp
WHERE
	NOT EXISTS
	(
		SELECT
			NULL
		FROM
			conversion c
		WHERE
			c.song_id=pp.song_id AND
			c.artist_id=pp.artist_id AND
			c.record_id=pp.record_id AND
			c.song_id=pp.song_id
	)
;

--	record_performance	1
INSERT INTO conversion
(
	song_id,
	artist_id,
	record_id,
	record_position,
	inserted_by
)
SELECT DISTINCT
	rp.song_id,
	rp.artist_id,
	rp.record_id,
	rp.record_position,
	'record_performance'
FROM
	---SQL_SCHEMA_NAME---record_performance rp
WHERE
	NOT EXISTS
	(
		SELECT
			NULL
		FROM
			conversion c
		WHERE
			c.song_id=rp.song_id AND
			c.artist_id=rp.artist_id AND
			c.record_id=rp.record_id AND
			c.song_id=rp.song_id
	)
;

INSERT INTO conversion
(
	song_id,
	artist_id,
	record_id,
	record_position,
	inserted_by
)
SELECT DISTINCT
	op_song_id,
	op_artist_id,
	op_record_id,
	op_record_position,
	'record_performance2'
FROM
	---SQL_SCHEMA_NAME---record_performance rp
WHERE
	NOT EXISTS
	(
		SELECT
			NULL
		FROM
			conversion c
		WHERE
			c.song_id=rp.op_song_id AND
			c.artist_id=rp.op_artist_id AND
			c.record_id=rp.op_record_id AND
			c.song_id=rp.op_song_id
	)
;


--	chart_performance
INSERT INTO conversion
(
	artist_id,
	song_id,
	inserted_by
)
SELECT DISTINCT
	cp.artist_id,
	cp.song_id,
	'chart_performance'
FROM
	---SQL_SCHEMA_NAME---chart_performance cp
WHERE
	NOT EXISTS
	(
		SELECT
			NULL
		FROM
			conversion c
		WHERE
			c.song_id=cp.song_id AND
			c.artist_id=cp.artist_id 
	)

;

--	performance
INSERT INTO conversion
(
	artist_id,
	song_id,
	inserted_by
)
SELECT DISTINCT
	p.artist_id,
	p.song_id,
	'performance'
FROM
	---SQL_SCHEMA_NAME---performance p
WHERE
	NOT EXISTS
	(
		SELECT
			NULL
		FROM
			conversion c
		WHERE
			c.song_id=p.song_id AND
			c.artist_id=p.artist_id 
	)
;


--	artists
INSERT INTO conversion
(
	artist_id,
	inserted_by
)
SELECT DISTINCT
	a.artist_id,
	'artist'
FROM
	---SQL_SCHEMA_NAME---artist a
WHERE
	NOT EXISTS
	(
		SELECT
			NULL
		FROM
			conversion c
		WHERE
			c.artist_id=a.artist_id 
	)
;
	
--	songs
INSERT INTO conversion
(
	song_id
)
SELECT DISTINCT
	s.song_id
FROM 
	---SQL_SCHEMA_NAME---song s
WHERE
	NOT EXISTS
	(
		SELECT
			NULL
		FROM
			conversion c
		WHERE
			c.song_id=s.song_id 
	)
;


CREATE INDEX conversion_online_performance_id ON conversion(online_performance_id);

--	compare
SELECT 'artist',COUNT(DISTINCT artist_id) FROM ---SQL_SCHEMA_NAME---artist
UNION
SELECT 'conversion',COUNT(DISTINCT artist_id) FROM conversion
;
	
SELECT 'song',COUNT(DISTINCT song_id) FROM ---SQL_SCHEMA_NAME---song
UNION
SELECT 'conversion',COUNT(DISTINCT song_id) FROM conversion
;
	
SELECT 'record',COUNT(DISTINCT record_id) FROM ---SQL_SCHEMA_NAME---record
UNION
SELECT 'conversion',COUNT(DISTINCT record_id) FROM conversion
;
	
--SELECT 'playlist',COUNT(DISTINCT playlist_id) FROM ---SQL_SCHEMA_NAME---playlist
--UNION
--SELECT 'conversion',COUNT(DISTINCT playlist_Id) FROM conversion
--;
	
--SELECT 'chart',COUNT(DISTINCT chart_id) FROM ---SQL_SCHEMA_NAME---chart
--UNION
--SELECT 'conversion',COUNT(DISTINCT chart_id) FROM conversion
--;
