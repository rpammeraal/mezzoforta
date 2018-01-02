BEGIN;

--	1.	DROP OLD TABLES

DROP TABLE ---SQL_SCHEMA_NAME---artist_old CASCADE
;
DROP TABLE ---SQL_SCHEMA_NAME---artist_rel_old CASCADE
;
DROP TABLE ---SQL_SCHEMA_NAME---performance_old CASCADE
;
DROP TABLE ---SQL_SCHEMA_NAME---record_old CASCADE;

DROP TABLE ---SQL_SCHEMA_NAME---record_performance_old CASCADE
;
DROP VIEW  IF EXISTS ---SQL_SCHEMA_NAME---v_performance 
;
DROP VIEW  IF EXISTS ---SQL_SCHEMA_NAME---v_online_performance
;
DROP TABLE ---SQL_SCHEMA_NAME---collection_performance CASCADE
;
DROP TABLE ---SQL_SCHEMA_NAME---collection CASCADE
;
DROP TABLE ---SQL_SCHEMA_NAME---playlist_composite CASCADE
;
DROP TABLE ---SQL_SCHEMA_NAME---chart_performance_old CASCADE
;
DROP TABLE ---SQL_SCHEMA_NAME---chart_old CASCADE
;
DROP TABLE ---SQL_SCHEMA_NAME---toplay CASCADE
;
DROP TABLE ---SQL_SCHEMA_NAME---playlist_performance CASCADE
;
DROP TABLE ---SQL_SCHEMA_NAME---playlist_old CASCADE
;
DROP TABLE ---SQL_SCHEMA_NAME---online_performance_old CASCADE
;
DROP TABLE ---SQL_SCHEMA_NAME---category_record CASCADE
;
DROP TABLE ---SQL_SCHEMA_NAME---lyrics_old CASCADE
;
DROP TABLE ---SQL_SCHEMA_NAME---song_old CASCADE
;
DROP TABLE IF EXISTS ---SQL_SCHEMA_NAME---index
;
DROP TABLE IF EXISTS ---SQL_SCHEMA_NAME---category CASCADE
;
DROP TABLE IF EXISTS ---SQL_SCHEMA_NAME---category_index CASCADE
;
DROP TABLE IF EXISTS ---SQL_SCHEMA_NAME---radio_stat CASCADE
;
DROP TABLE IF EXISTS ---SQL_SCHEMA_NAME---to_be_indexed CASCADE
;

CREATE VIEW ---SQL_SCHEMA_NAME---all
AS
SELECT
	a.artist_id,
	a.name AS artist_name,
	s.song_id,
	s.title AS song_title,
	r.record_id,
	rp.record_position,
	r.title AS record_title,
	p.performance_id,
	rp.record_performance_id,
	op.online_performance_id,
	op.path
FROM
	---SQL_SCHEMA_NAME---online_performance op
		JOIN ---SQL_SCHEMA_NAME---record_performance rp USING(record_performance_id)
		JOIN ---SQL_SCHEMA_NAME---performance p USING(performance_id)
		JOIN ---SQL_SCHEMA_NAME---song s USING(song_id)
		JOIN ---SQL_SCHEMA_NAME---artist a USING(artist_id)
		JOIN ---SQL_SCHEMA_NAME---record r USING(record_id)
;


--	2.	constraints to be created
ALTER TABLE ---SQL_SCHEMA_NAME---song
ADD CONSTRAINT fk_song_original_performance_id_performance 
FOREIGN KEY (original_performance_id) 
REFERENCES ---SQL_SCHEMA_NAME---performance(performance_id) 
;

ALTER TABLE ---SQL_SCHEMA_NAME---performance
ADD CONSTRAINT fk_performance_preferred_record_performance_id_record_performance_record
FOREIGN KEY (preferred_record_performance_id) REFERENCES ---SQL_SCHEMA_NAME---record_performance(record_performance_id)
;

ALTER TABLE ---SQL_SCHEMA_NAME---record_performance
ADD CONSTRAINT fk_record_performance_preferred_online_performance_id_online_performance_online_performance_id
FOREIGN KEY (preferred_online_performance_id) REFERENCES ---SQL_SCHEMA_NAME---online_performance(online_performance_id) 
;

--	performance.preferred_record_performance_id
WITH prefRecordPerformance AS
(
	SELECT
		performance_id,
		MIN(record_performance_id) OVER(PARTITION BY performance_id ORDER BY record_performance_id) AS record_performance_id
	FROM
		---SQL_SCHEMA_NAME---record_performance p
)
UPDATE ---SQL_SCHEMA_NAME---performance AS p
SET
	preferred_record_performance_id=t.record_performance_id
FROM
	prefRecordPerformance t
WHERE
	p.performance_id=t.performance_id
;

--	Remove stale and dangling records
DELETE FROM 
	---SQL_SCHEMA_NAME---performance 
WHERE 
	preferred_record_performance_id IS NULL AND 
	performance_id NOT IN 
	(
		SELECT 
			performance_id 
		FROM 
			---SQL_SCHEMA_NAME---chart_performance
		UNION
		SELECT 
			original_performance_id
		FROM
			---SQL_SCHEMA_NAME---song
	)
;

--	take care of what was taken care of op_*id's:
WITH t AS
(
	SELECT
		song_id,
		artist_id,
		online_performance_id
	FROM
		conversion c
	WHERE
		path IS NOT NULL
)
UPDATE
	conversion n
SET
	online_performance_id=t.online_performance_id
FROM
	t
WHERE
	n.online_performance_id IS NULL AND
	n.song_id=t.song_id AND
	n.artist_id=t.artist_id
;

--	record_performance.preferred_online_performance_id
WITH t AS
(
	SELECT 
		record_performance_id ,
		MIN(online_performance_id) OVER(PARTITION BY record_performance_id ORDER BY online_performance_id) AS online_performance_id
	FROM 
		conversion
	WHERE
		online_performance_id IS NOT NULL
)
UPDATE ---SQL_SCHEMA_NAME---record_performance AS rp
SET
	preferred_online_performance_id=t.online_performance_id
FROM
	t
WHERE
	rp.record_performance_id=t.record_performance_id
;

--	set year from record, if not set
WITH allyears
AS
(
	SELECT
		a.artist_id,
		s.song_id,
		a.name,
		s.title,
		MIN(r.year) OVER(PARTITION BY a.artist_id,s.song_id) AS year
	FROM
		rock.performance p
			JOIN ---SQL_SCHEMA_NAME---artist a USING(artist_id)
			JOIN ---SQL_SCHEMA_NAME---song s USING(song_id)
			JOIN ---SQL_SCHEMA_NAME---record_performance rp USING(performance_id)
			JOIN ---SQL_SCHEMA_NAME---record r USING(record_id)
	WHERE
		COALESCE(p.year,0)<1900
)
UPDATE ---SQL_SCHEMA_NAME---performance AS p
SET
	year=y.year
FROM
	allyears y
WHERE
	p.artist_id=y.artist_id AND
	p.song_id=y.song_id AND
	COALESCE(p.year,0)<1900 AND
	COALESCE(y.year,0)!=0
;
	
DROP TABLE conversion;

CREATE UNIQUE INDEX ---SQL_SCHEMA_NAME---_ui_performance_song_artist ON ---SQL_SCHEMA_NAME___performance(song_id,artist_id);
CREATE TABLE ---SQL_SCHEMA_NAME---artist_match (artist_alternative_name VARCHAR NOT NULL, artist_correct_name VARCHAR NOT NULL);
CREATE UNIQUE INDEX ui_artist_match ON ---SQL_SCHEMA_NAME---artist_match (artist_alternative_name,artist_correct_name);

SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---performance WHERE preferred_record_performance_id IS NULL;
SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---record_performance WHERE preferred_online_performance_id IS NULL;


--	Reset sequences
SELECT setval('---SQL_SCHEMA_NAME---artist_artist_id_seq', MAX(artist_id)+1, FALSE) FROM ---SQL_SCHEMA_NAME---artist;
SELECT setval('---SQL_SCHEMA_NAME---chart_chart_id_seq', MAX(chart_id)+1, FALSE) FROM ---SQL_SCHEMA_NAME---chart;
SELECT setval('---SQL_SCHEMA_NAME---chart_performance_chart_performance_id_seq', MAX(chart_performance_id)+1, FALSE) FROM ---SQL_SCHEMA_NAME---chart_performance;
SELECT setval('---SQL_SCHEMA_NAME---online_performance_online_performance_id_seq', MAX(online_performance_id)+1, FALSE) FROM ---SQL_SCHEMA_NAME---online_performance;
SELECT setval('---SQL_SCHEMA_NAME---performance_performance_id_seq', MAX(performance_id)+1, FALSE) FROM ---SQL_SCHEMA_NAME---performance;
SELECT setval('---SQL_SCHEMA_NAME---playlist_playlist_id_seq', MAX(playlist_id)+1, FALSE) FROM ---SQL_SCHEMA_NAME---playlist;
SELECT setval('---SQL_SCHEMA_NAME---playlist_detail_playlist_detail_id_seq', MAX(playlist_detail_id)+1, FALSE) FROM ---SQL_SCHEMA_NAME---playlist_detail;
SELECT setval('---SQL_SCHEMA_NAME---record_record_id_seq', MAX(record_id)+1, FALSE) FROM ---SQL_SCHEMA_NAME---record;
SELECT setval('---SQL_SCHEMA_NAME---record_performance_record_performance_id_seq', MAX(record_performance_id)+1, FALSE) FROM ---SQL_SCHEMA_NAME---record_performance;
SELECT setval('---SQL_SCHEMA_NAME---song_song_id_seq', MAX(song_id)+1, FALSE) FROM ---SQL_SCHEMA_NAME---song;

COMMIT;
