BEGIN;

ALTER TABLE rock.record_performance RENAME TO record_performance_old;

CREATE TABLE rock.record_performance 
( 
	record_performance_id SERIAL PRIMARY KEY NOT NULL, 
	performance_id        INT NOT NULL, 
	record_id             INT NOT NULL, 
	record_position       SMALLINT NOT NULL, 
	duration              TIME WITHOUT TIME ZONE NOT NULL, 
	notes                 TEXT, 
	CONSTRAINT cc_record_performance_record_position_check CHECK ((record_position >= 0)), 
	CONSTRAINT fk_record_performance_record_id_record_record_id FOREIGN KEY (record_id) REFERENCES rock.record(record_id),
	CONSTRAINT fk_record_performance_performance_id_performance_performance_id FOREIGN KEY (performance_id) REFERENCES rock.performance(performance_id) 
); 

INSERT INTO rock.record_performance (performance_id,record_id,record_position,duration,notes)
SELECT 
	p.performance_id,
	rpo.record_id,
	rpo.record_position,
	rpo.duration,
	rpo.notes
FROM
	rock.record_performance_old rpo
		JOIN rock.performance p ON
			rpo.song_id=p.song_id AND
			rpo.artist_id=p.artist_id
;

SELECT COUNT(*) FROM rock.record_performance_old;
SELECT COUNT(*) FROM rock.record_performance;

COMMIT;
