BEGIN;

CREATE TABLE rock.play_history
(
	artist_name          VARCHAR NOT NULL,
	record_title         VARCHAR NOT NULL,
	record_position      INT NOT NULL,
	song_title           VARCHAR NOT NULL,
	path                 VARCHAR NOT NULL,
	played_by_radio_flag BOOL NOT NULL,
	play_datetime        TIMESTAMP NOT NULL
);

INSERT INTO rock.play_history
(
	artist_name,
	record_title,
	record_position,
	song_title,
	path,
	played_by_radio_flag,
	play_datetime
)
SELECT
	a.name,
	r.title,
	rp.record_position,
	s.title,
	op.path,
	'T'::BOOL,
	op.last_play_date
FROM
	rock.online_performance op
		JOIN rock.record_performance rp USING(record_performance_id)
		JOIN rock.performance p USING(performance_id)
		JOIN rock.song s USING(song_id)
		JOIN rock.artist a USING(artist_id)
		JOIN rock.record r USING(record_id)
		LEFT JOIN rock.play_history r_ph ON
			op.last_play_date = r_ph.play_datetime
WHERE
	op.last_play_date IS NOT NULL AND
	r_ph.play_datetime IS NULL
	
ORDER BY
	op.last_play_date
;

SELECT * FROM rock.play_history
;

COMMIT;
