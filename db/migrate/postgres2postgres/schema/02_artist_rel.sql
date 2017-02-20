BEGIN;

ALTER TABLE rock.artist_rel RENAME TO artist_rel_old;

CREATE TABLE rock.artist_rel 
( 
	artist_rel_id SERIAL PRIMARY KEY, 
	artist1_id    INT NOT NULL, 
	artist2_id    INT NOT NULL, 
	CONSTRAINT fk_artist_rel_artist1_id_artist_artist_id FOREIGN KEY (artist1_id) REFERENCES rock.artist(artist_id), 
	CONSTRAINT fk_artist_rel_artist2_id_artist_artist_id FOREIGN KEY (artist2_id) REFERENCES rock.artist(artist_id) 
); 

INSERT INTO rock.artist_rel (artist1_id,artist2_id)
SELECT artist1_id,artist2_id FROM rock.artist_rel_old
;

DROP TABLE rock.artist_rel_old
;


CREATE INDEX idx_artist_rel_a1 ON rock.artist_rel   (artist1_id);
CREATE INDEX idx_artist_rel_a2 ON rock.artist_rel   (artist2_id);

COMMIT;
