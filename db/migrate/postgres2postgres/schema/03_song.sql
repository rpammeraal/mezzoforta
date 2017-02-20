BEGIN ;

ALTER TABLE rock.song RENAME TO song_old;

CREATE TABLE rock.song 
( 
	song_id SERIAL PRIMARY KEY NOT NULL, 
	title   CHARACTER VARYING NOT NULL, 
	notes   TEXT, 
	soundex CHARACTER VARYING, 
	CONSTRAINT cc_song_song_id_check CHECK ((song_id >= 0)), 
	CONSTRAINT cc_song_title_check CHECK (((title) <> '')) 
); 

CREATE INDEX idx_song_title ON rock.song (title);
CREATE INDEX idx_song_soundex ON rock.song (soundex); 

INSERT INTO rock.song
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
	rock.song_old;

SELECT COUNT(*) FROM rock.song_old;
SELECT COUNT(*) FROM rock.song;

COMMIT;

