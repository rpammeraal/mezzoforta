SELECT
	r.title,
	rp.record_position,
	a.name,
	s.title
FROM
	rock.record_performance rp
		JOIN rock.performance p USING(performance_id)
		JOIN rock.song s USING(song_id)
		JOIN rock.artist a USING(artist_id)
		JOIN rock.record r USING(record_id)
WHERE
 	rp.preferred_online_performance_id IS NULL
ORDER BY
	1,2,3
;
