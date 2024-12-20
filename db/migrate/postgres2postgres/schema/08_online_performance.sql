BEGIN;

ALTER TABLE ---SQL_SCHEMA_NAME---online_performance RENAME TO online_performance_old;

CREATE TABLE ---SQL_SCHEMA_NAME---online_performance 
( 
	online_performance_id ---AUTOID--- PRIMARY KEY NOT NULL, 
	record_performance_id INT NOT NULL, 
	path                  CHARACTER VARYING NOT NULL, 
	last_play_date        TIMESTAMP without time zone, 
	CONSTRAINT fk_online_performance_record_performance_id_record_performance_record_performance_id FOREIGN KEY (record_performance_id) REFERENCES ---SQL_SCHEMA_NAME---record_performance(record_performance_id) 
); 

INSERT INTO ---SQL_SCHEMA_NAME---online_performance 
(
	record_performance_id,
	path,
	last_play_date
)
SELECT
	rp.record_performance_id,
	opo.path,
	opo.last_play_date
FROM
	---SQL_SCHEMA_NAME---online_performance_old opo
		JOIN ---SQL_SCHEMA_NAME---performance p ON
			opo.artist_id=p.artist_id AND
			opo.song_id=p.song_id
		JOIN ---SQL_SCHEMA_NAME---record_performance rp ON
			p.performance_id=rp.performance_id AND
			opo.record_id=rp.record_id AND
			opo.record_position=rp.record_position
ORDER BY
	opo.insert_order
;

UPDATE conversion AS c
SET
	online_performance_id=op.online_performance_id
FROM
	---SQL_SCHEMA_NAME---online_performance op
WHERE
	c.path=op.path
;

UPDATE ---SQL_SCHEMA_NAME---online_performance SET path=REPLACE(path,'\','') WHERE path LIKE '%\\%';

SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---online_performance;
SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---online_performance_old;
SELECT COUNT(DISTINCT online_performance_id) FROM conversion;

COMMIT;
