BEGIN;

ALTER TABLE ---SQL_SCHEMA_NAME---record_performance RENAME TO record_performance_old;

CREATE TABLE ---SQL_SCHEMA_NAME---record_performance 
( 
	record_performance_id           ---AUTOID--- PRIMARY KEY NOT NULL, 
	performance_id                  INT NOT NULL, 
	preferred_online_performance_id INT NULL,
	record_id                       INT NOT NULL, 
	record_position                 SMALLINT NOT NULL, 
	duration                        TIME WITHOUT TIME ZONE NOT NULL, 
	notes                           TEXT, 
	CONSTRAINT cc_record_performance_record_position_check CHECK ((record_position >= 0)), 
	CONSTRAINT fk_record_performance_record_id_record_record_id FOREIGN KEY (record_id) REFERENCES ---SQL_SCHEMA_NAME---record(record_id),
	CONSTRAINT fk_record_performance_performance_id_performance_performance_id FOREIGN KEY (performance_id) REFERENCES ---SQL_SCHEMA_NAME---performance(performance_id) 
); 

INSERT INTO ---SQL_SCHEMA_NAME---record_performance 
(
	performance_id,
	record_id,
	record_position,
	duration,
	notes
)
SELECT 
	p.performance_id,
	rpo.record_id,
	rpo.record_position,
	rpo.duration,
	rpo.notes
FROM
	---SQL_SCHEMA_NAME---record_performance_old rpo
		JOIN ---SQL_SCHEMA_NAME---performance p ON
			rpo.song_id=p.song_id AND
			rpo.artist_id=p.artist_id
;

UPDATE conversion AS c
SET
	record_performance_id=rp.record_performance_id
FROM
	---SQL_SCHEMA_NAME---record_performance rp
WHERE
	c.performance_id=rp.performance_id AND
	c.record_id=rp.record_id AND
	c.record_position=rp.record_position
;

SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---record_performance_old;
SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---record_performance;
SELECT COUNT(DISTINCT record_performance_id) FROM conversion;

COMMIT;
