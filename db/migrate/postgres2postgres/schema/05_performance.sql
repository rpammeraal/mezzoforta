BEGIN;

ALTER TABLE ---SQL_SCHEMA_NAME---performance RENAME TO performance_old;

CREATE TABLE ---SQL_SCHEMA_NAME---performance 
( 
	performance_id                  ---AUTOID--- PRIMARY KEY NOT NULL, 
	song_id                         INT NOT NULL, 
	artist_id                       INT NOT NULL, 
	preferred_record_performance_id INT NULL,
	role_id                         SMALLINT NOT NULL, 
	year                            INT, 
	notes                           TEXT, 
	CONSTRAINT cc_performance_role_id_check CHECK ((role_id >= 0) AND (role_id <= 1)), 
	CONSTRAINT cc_performance_year_check CHECK ((year IS NULL) OR (year >= 1900)), 
	CONSTRAINT fk_performance_song_id_song FOREIGN KEY (song_id) REFERENCES ---SQL_SCHEMA_NAME---song(song_id) 
); 

INSERT INTO ---SQL_SCHEMA_NAME---performance 
(
	song_id,
	artist_id,
	role_id,
	year,
	notes
)
SELECT 
	song_id,
	artist_id,
	role_id,
	year,
	notes
FROM
	---SQL_SCHEMA_NAME---performance_old
;


UPDATE conversion AS c
SET
	performance_id=p.performance_id
FROM
	---SQL_SCHEMA_NAME---performance p
WHERE
	c.song_id=p.song_id AND
	c.artist_id=p.artist_id
;

SELECT * FROM conversion WHERE song_id=23654;
SELECT * FROM ---SQL_SCHEMA_NAME---performance WHERE song_id=23654;
SELECT * FROM ---SQL_SCHEMA_NAME---performance_old WHERE song_id=23654;

SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---performance_old;
SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---performance;
SELECT COUNT(DISTINCT performance_id) FROM conversion;


COMMIT;
