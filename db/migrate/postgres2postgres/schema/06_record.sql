BEGIN;

ALTER TABLE ---SQL_SCHEMA_NAME---record RENAME TO record_old;

CREATE TABLE ---SQL_SCHEMA_NAME---record 
( 
	record_id     ---AUTOID--- PRIMARY KEY NOT NULL, 
	artist_id     INT NOT NULL, 
	title         CHARACTER VARYING NOT NULL, 
	media         CHARACTER VARYING(10) NOT NULL, 
	year          INT, 
	genre         CHARACTER VARYING, 
	cddb_id       CHARACTER(8), 
	cddb_category CHARACTER VARYING(20), 
	notes         TEXT, 
	CONSTRAINT cc_record_media_check CHECK (((media) <> '')), 
	CONSTRAINT cc_record_record_id_check CHECK ((record_id >= 0)), 
	CONSTRAINT cc_record_title_check CHECK (((title) <> '')), 
	CONSTRAINT fk_record_artist_id_artist_artist_id FOREIGN KEY (artist_id) REFERENCES ---SQL_SCHEMA_NAME---artist(artist_id) 
); 

INSERT INTO ---SQL_SCHEMA_NAME---record (record_id,artist_id,title,media,year,genre,cddb_id,cddb_category,notes)
SELECT record_id,artist_id,title,media,year,genre,cddb_id,cddb_category,notes FROM ---SQL_SCHEMA_NAME---record_old;

SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---record;
SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---record_old;
SELECT COUNT(DISTINCT record_id) FROM conversion;

COMMIT;
