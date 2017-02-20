BEGIN;

ALTER TABLE rock.performance RENAME TO performance_old;

CREATE TABLE rock.performance 
( 
	performance_id SERIAL PRIMARY KEY NOT NULL, 
	song_id        INT NOT NULL, 
	artist_id      INT NOT NULL, 
	role_id        SMALLINT NOT NULL, 
	year           INT, 
	notes          TEXT, 
	CONSTRAINT cc_performance_role_id_check CHECK (((role_id >= 0) AND (role_id <= 1))), 
	CONSTRAINT cc_performance_year_check CHECK (((year IS NULL) OR (year >= 1900))), 
	CONSTRAINT fk_performance_song_id_song_song_id FOREIGN KEY (song_id) REFERENCES rock.song(song_id) 
); 

INSERT INTO rock.performance (song_id,artist_id,role_id,year,notes)
SELECT * FROM rock.performance_old;

SELECT COUNT(*) FROM rock.performance_old;
SELECT COUNT(*) FROM rock.performance;

COMMIT;
