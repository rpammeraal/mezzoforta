BEGIN;

ALTER TABLE rock.playlist RENAME TO playlist_old;

CREATE TABLE rock.playlist 
( 
	playlist_id SERIAL PRIMARY KEY NOT NULL, 
	name        CHARACTER VARYING NOT NULL, 
	notes       TEXT, 
	created     DATE NOT NULL, 
	duration    INTERVAL, 
	updated     DATE, 
	play_mode   INT NOT NULL, 
	CONSTRAINT cc_playlist_play_mode_check CHECK (((play_mode >= 0) AND (play_mode <= 2))), 
	CONSTRAINT cc_playlist_playlist_id_check CHECK ((playlist_id >= 0))
); 

INSERT INTO rock.playlist (playlist_id,name,notes,created,duration,updated,play_mode)
SELECT  playlist_id,name,notes,created,duration,updated,play_mode FROM rock.playlist_old;

SELECT COUNT(*) FROM rock.playlist_old;
SELECT COUNT(*) FROM rock.playlist;

COMMIT;
