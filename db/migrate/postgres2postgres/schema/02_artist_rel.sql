BEGIN;

ALTER TABLE ---SQL_SCHEMA_NAME---artist_rel RENAME TO artist_rel_old;

CREATE TABLE ---SQL_SCHEMA_NAME---artist_rel 
( 
	artist_rel_id SERIAL PRIMARY KEY, 
	artist1_id    INT NOT NULL, 
	artist2_id    INT NOT NULL, 
	CONSTRAINT fk_artist_rel_artist1_id_artist_artist_id FOREIGN KEY (artist1_id) REFERENCES ---SQL_SCHEMA_NAME---artist(artist_id), 
	CONSTRAINT fk_artist_rel_artist2_id_artist_artist_id FOREIGN KEY (artist2_id) REFERENCES ---SQL_SCHEMA_NAME---artist(artist_id) 
); 

INSERT INTO ---SQL_SCHEMA_NAME---artist_rel 
(
	artist1_id,
	artist2_id
)
SELECT 
	artist1_id,
	artist2_id 
FROM 
	---SQL_SCHEMA_NAME---artist_rel_old
;


CREATE INDEX idx_artist_rel_a1 ON ---SQL_SCHEMA_NAME---artist_rel   (artist1_id);
CREATE INDEX idx_artist_rel_a2 ON ---SQL_SCHEMA_NAME---artist_rel   (artist2_id);


SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---artist_rel_old
;
SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---artist_rel
;
COMMIT;
