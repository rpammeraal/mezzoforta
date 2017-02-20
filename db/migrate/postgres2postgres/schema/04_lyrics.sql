BEGIN;

ALTER TABLE classical.lyrics RENAME TO lyrics_old;

CREATE TABLE classical.lyrics 
( 
	song_id INT PRIMARY KEY NOT NULL, 
	lyrics  TEXT, 
	CONSTRAINT fk_lyrics_song_id_song_song_id FOREIGN KEY(song_id) REFERENCES classical.song(song_id) 
); 


INSERT INTO classical.lyrics (song_id,lyrics)
SELECT * FROM classical.lyrics_old;

SELECT COUNT(*) FROM classical.lyrics_old;
SELECT COUNT(*) FROM classical.lyrics;

COMMIT;
