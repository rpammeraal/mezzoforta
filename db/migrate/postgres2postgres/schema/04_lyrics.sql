BEGIN;

ALTER TABLE ---SQL_SCHEMA_NAME---lyrics RENAME TO lyrics_old;

CREATE TABLE ---SQL_SCHEMA_NAME---lyrics 
( 
	song_id INT PRIMARY KEY NOT NULL, 
	lyrics  TEXT, 
	CONSTRAINT fk_lyrics_song_id_song_song_id FOREIGN KEY(song_id) REFERENCES ---SQL_SCHEMA_NAME---song(song_id) 
); 


INSERT INTO ---SQL_SCHEMA_NAME---lyrics 
(
	song_id,
	lyrics
)
SELECT 
	song_id,
	lyrics
FROM 
	---SQL_SCHEMA_NAME---lyrics_old;

SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---lyrics_old;
SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---lyrics;

COMMIT;
