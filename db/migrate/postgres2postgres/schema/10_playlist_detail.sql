BEGIN;

CREATE TABLE rock.playlist_detail 
( 
	playlist_detail_id    SERIAL PRIMARY KEY NOT NULL, 
	playlist_id           INT NOT NULL, 
	playlist_position     INT NOT NULL, 
	record_performance_id INT NULL, 
	child_playlist_id     INT NULL, 
	chart_id              INT NULL, 
	record_id             INT NULL, 
	artist_id             INT NULL, 
	collection_id         INT, 
	c_position            SMALLINT, 
	timestamp             DATE NOT NULL, 
	notes                 TEXT, 
	CONSTRAINT cc_playlist_detail_playlist_position_check CHECK ((playlist_position > 0)),
	CONSTRAINT fk_playlist_detail_playlist_playlist_id FOREIGN KEY(playlist_id) REFERENCES rock.playlist(playlist_id), 
	CONSTRAINT fk_playlist_detail_record_performance_record_performance_id FOREIGN KEY (record_performance_id) REFERENCES rock.record_performance(record_performance_id), 
	CONSTRAINT fk_playlist_detail_playlist_child_playlist_id FOREIGN KEY(child_playlist_id) REFERENCES rock.playlist(playlist_id), 
	CONSTRAINT fk_playlist_detail_record_record_id FOREIGN KEY (record_id) REFERENCES rock.record(record_id), 
	CONSTRAINT fk_playlist_detail_artist_artist_id FOREIGN KEY (artist_id) REFERENCES rock.artist(artist_id) 
); 

INSERT INTO rock.playlist_detail (playlist_id,playlist_position,timestamp,notes,child_playlist_id,chart_id,record_id,artist_id)
SELECT playlist_id,playlist_position,timestamp,notes,playlist_playlist_id,playlist_chart_id,playlist_record_id,playlist_artist_id
FROM rock.playlist_composite
WHERE playlist_id IS NOT NULL;

INSERT INTO rock.playlist_detail (playlist_id,playlist_position,c_position,timestamp,notes,record_performance_id)
SELECT
	pp.playlist_id,
	pp.playlist_position,
	pp.c_position,
	pp.timestamp,
	pp.notes,
	rp.record_performance_id
FROM 
	rock.playlist_performance pp
		JOIN rock.performance p ON
			pp.artist_id=p.artist_id AND
			pp.song_id=p.song_id
		JOIN rock.record_performance rp ON
			p.performance_id=rp.performance_id AND
			pp.record_id=rp.record_id AND
			pp.record_position=rp.record_position;
;

SELECT COUNT(*) FROM rock.playlist_composite WHERE playlist_id IS NOT NULL;
SELECT COUNT(*) FROM rock.playlist_performance WHERE playlist_id IS NOT NULL;
SELECT COUNT(*) FROM rock.playlist_detail;

COMMIT;

