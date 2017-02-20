BEGIN;

ALTER TABLE rock.online_performance RENAME TO online_performance_old;

CREATE TABLE rock.online_performance 
( 
	online_performance_id SERIAL PRIMARY KEY NOT NULL, 
	record_performance_id INT NOT NULL, 
	format_id             INT NOT NULL, 
	path                  CHARACTER VARYING NOT NULL, 
	source_id             INT NOT NULL, 
	last_play_date        TIMESTAMP without time zone, 
	play_order            INT, 
	insert_order          INT NOT NULL, 
	CONSTRAINT fk_online_performance_record_performance_id_record_performance_record_performance_id FOREIGN KEY (record_performance_id) REFERENCES rock.record_performance(record_performance_id) 
); 

INSERT INTO rock.online_performance (record_performance_id,format_id,path,source_id,last_play_date,play_order,insert_order)
SELECT
	rp.record_performance_id,
	opo.format_id,
	opo.path,
	opo.source_id,
	opo.last_play_date,
	opo.play_order,
	opo.insert_order
FROM
	rock.online_performance_old opo
		JOIN rock.performance p ON
			opo.artist_id=p.artist_id AND
			opo.song_id=p.song_id
		JOIN rock.record_performance rp ON
			p.performance_id=rp.performance_id AND
			opo.record_id=rp.record_id AND
			opo.record_position=rp.record_position;
;

SELECT COUNT(*) FROM rock.online_performance;
SELECT COUNT(*) FROM rock.online_performance_old;

COMMIT;
