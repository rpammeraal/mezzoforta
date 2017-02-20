BEGIN;

ALTER TABLE rock.artist RENAME TO artist_old;

CREATE TABLE rock.artist 
( 
	artist_id SERIAL PRIMARY KEY NOT NULL, 
	name      CHARACTER VARYING NOT NULL, 
	sort_name CHARACTER VARYING NOT NULL, 
	www       TEXT, 
	notes     TEXT, 
	mbid      CHARACTER VARYING, 
	soundex   CHARACTER VARYING, 
	CONSTRAINT cc_artist_artist_id_check CHECK ((artist_id >= 0)), 
	CONSTRAINT cc_artist_name_check CHECK (((name) <> '')) 
);

INSERT INTO rock.artist (artist_id,name,sort_name,www,notes,mbid,soundex)
SELECT artist_id,name,sort_name,www,notes,mbid,soundex FROM rock.artist_old;

SELECT COUNT(*) FROM rock.artist_old;
SELECT COUNT(*) FROM rock.artist;

COMMIT;
