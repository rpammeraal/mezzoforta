BEGIN;

ALTER TABLE ---SQL_SCHEMA_NAME---performance RENAME TO performance_old;

CREATE TABLE ---SQL_SCHEMA_NAME---performance 
( 
	performance_id                  ---AUTOID--- PRIMARY KEY NOT NULL, 
	song_id                         INT NOT NULL, 
	artist_id                       INT NOT NULL, 
	preferred_record_performance_id INT NULL,
	year                            INT, 
	notes                           TEXT, 
	CONSTRAINT cc_performance_year_check CHECK ((year IS NULL) OR (year >= 1900)), 
	CONSTRAINT fk_performance_song_id_song FOREIGN KEY (song_id) REFERENCES ---SQL_SCHEMA_NAME---song(song_id) 
); 

INSERT INTO ---SQL_SCHEMA_NAME---performance 
(
	song_id,
	artist_id,
	year,
	notes
)
SELECT 
	song_id,
	artist_id,
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

--	set song.orgiginal_performer_id
;WITH orgPerformer
AS
(
	SELECT
		c.song_id,
		c.performance_id
	FROM
		conversion c
			JOIN ---SQL_SCHEMA_NAME---performance_old p_o ON
				c.artist_id=p_o.artist_id AND
				c.song_id=p_o.song_id AND
				p_o.role_id=0
)
UPDATE rock.song AS s
SET
	original_performance_id=op.performance_id
FROM
	orgPerformer op
WHERE
	op.song_id=s.song_id
;

SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---performance_old;
SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---performance;
SELECT COUNT(DISTINCT performance_id) FROM conversion;


COMMIT;
