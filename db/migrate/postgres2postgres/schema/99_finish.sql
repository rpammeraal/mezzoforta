BEGIN;

--	1.	DROP OLD TABLES
ALTER TABLE ---SQL_SCHEMA_NAME---record_performance_old
DROP CONSTRAINT record_perf_online_performance;
ALTER TABLE ---SQL_SCHEMA_NAME---record_performance_old
DROP CONSTRAINT record_perf_performance;
ALTER TABLE ---SQL_SCHEMA_NAME---record_performance_old
DROP CONSTRAINT record_perf_record_performance;

ALTER TABLE ---SQL_SCHEMA_NAME---online_performance_old
DROP CONSTRAINT online_perf_record_performance;

ALTER TABLE ---SQL_SCHEMA_NAME---collection_performance
DROP CONSTRAINT collection_perf_dc_id;
ALTER TABLE ---SQL_SCHEMA_NAME---collection_performance
DROP CONSTRAINT collection_perf_performance;
ALTER TABLE ---SQL_SCHEMA_NAME---collection_performance
DROP CONSTRAINT record_perf_fi;

ALTER TABLE ---SQL_SCHEMA_NAME---playlist_performance
DROP CONSTRAINT playlist_perf_col_performance;

DROP VIEW  ---SQL_SCHEMA_NAME---v_performance
;
DROP VIEW  ---SQL_SCHEMA_NAME---v_online_performance
;
DROP TABLE ---SQL_SCHEMA_NAME---collection_performance
;
DROP TABLE ---SQL_SCHEMA_NAME---collection
;
DROP TABLE ---SQL_SCHEMA_NAME---playlist_composite
;
DROP TABLE ---SQL_SCHEMA_NAME---chart_performance_old
;
DROP TABLE ---SQL_SCHEMA_NAME---chart_old
;
DROP TABLE ---SQL_SCHEMA_NAME---toplay_old
;
DROP TABLE ---SQL_SCHEMA_NAME---playlist_performance
;
DROP TABLE ---SQL_SCHEMA_NAME---playlist_old
;
DROP TABLE ---SQL_SCHEMA_NAME---online_performance_old
;
DROP TABLE ---SQL_SCHEMA_NAME---record_performance_old
;
DROP TABLE ---SQL_SCHEMA_NAME---category_record
;
DROP TABLE ---SQL_SCHEMA_NAME---record_old
;
DROP TABLE ---SQL_SCHEMA_NAME---performance_old
;
DROP TABLE ---SQL_SCHEMA_NAME---lyrics_old
;
DROP TABLE ---SQL_SCHEMA_NAME---song_old
;
DROP TABLE ---SQL_SCHEMA_NAME---artist_rel_old
;
DROP TABLE ---SQL_SCHEMA_NAME---artist_old
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

--	song.preferred_performance_id
		
--DROP TABLE conversion;

SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---performance WHERE preferred_record_performance_id IS NULL;
SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---record_performance WHERE preferred_online_performance_id IS NULL;

COMMIT;
