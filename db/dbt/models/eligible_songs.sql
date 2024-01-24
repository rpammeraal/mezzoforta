SELECT
	ROW_NUMBER() OVER(ORDER BY last_play_date) 						AS id,
	a.name															AS artist_name,
	r.title															AS record_title,
	rp.record_position,
	s.title															AS song_title,
	op.last_play_date												AS play_datetime,
	MAX(record_position) OVER(PARTITION BY a.name,r.title) 			AS num_songs_on_record
FROM
	rock.online_performance op
		JOIN rock.record_performance rp USING(record_performance_id)
		JOIN rock.performance p USING(performance_id)
		JOIN rock.artist a USING(artist_id)
		JOIN rock.record r USING(record_id)
		JOIN rock.song s USING(song_id)
ORDER BY
	last_play_date DESC
