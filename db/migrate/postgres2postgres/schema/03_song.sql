BEGIN ;

ALTER TABLE ---SQL_SCHEMA_NAME---song RENAME TO song_old;

CREATE TABLE ---SQL_SCHEMA_NAME---song 
( 
	song_id                 ---AUTOID--- PRIMARY KEY NOT NULL, 
	original_performance_id INT NULL,
	title                   CHARACTER VARYING NOT NULL, 
	notes                   TEXT, 
	soundex                 CHARACTER VARYING, 
	CONSTRAINT cc_song_song_id_check CHECK ((song_id >= 0)), 
	CONSTRAINT cc_song_title_check CHECK (((title) <> ''))
); 

CREATE INDEX idx_song_title ON ---SQL_SCHEMA_NAME---song (title);
CREATE INDEX idx_song_soundex ON ---SQL_SCHEMA_NAME---song (soundex); 

INSERT INTO ---SQL_SCHEMA_NAME---song
(
	song_id,
	title,
	notes,
	soundex
)
SELECT
	song_id,
	title,
	notes,
	soundex
FROM
	---SQL_SCHEMA_NAME---song_old
;

SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---song_old;
SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---song;
SELECT COUNT(DISTINCT song_id) FROM conversion;

COMMIT;

