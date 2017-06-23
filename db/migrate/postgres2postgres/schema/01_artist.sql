BEGIN;

ALTER TABLE ---SQL_SCHEMA_NAME---artist RENAME TO artist_old;

CREATE TABLE ---SQL_SCHEMA_NAME---artist 
( 
	artist_id ---AUTOID--- PRIMARY KEY NOT NULL, 
	name      CHARACTER VARYING NOT NULL, 
	sort_name CHARACTER VARYING NOT NULL, 
	www       TEXT, 
	notes     TEXT, 
	mbid      CHARACTER VARYING, 
	soundex   CHARACTER VARYING, 
	CONSTRAINT cc_artist_artist_id_check CHECK ((artist_id >= 0)), 
	CONSTRAINT cc_artist_name_check CHECK (((name) <> '')) 
);

INSERT INTO ---SQL_SCHEMA_NAME---artist 
(
	artist_id,
	name,
	sort_name,
	www,
	notes,
	mbid,
	soundex)
SELECT 
	artist_id,
	name,
	sort_name,
	www,
	notes,
	mbid,
	soundex 
FROM 
	---SQL_SCHEMA_NAME---artist_old
;

SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---artist_old;
SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---artist;
SELECT COUNT(DISTINCT artist_id) FROM conversion;

COMMIT;
