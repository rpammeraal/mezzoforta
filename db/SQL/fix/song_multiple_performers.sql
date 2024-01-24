SELECT
	s.title,
	a.artist_id,
	p.performance_id,
	a.name,
	a.artist_id,
	c.name,
	cp.chart_performance_id
FROM
	rock.song s
		JOIN rock.performance p USING (song_id)
		JOIN rock.artist a USING(artist_id)
		JOIN rock.chart_performance cp USING(performance_id)
		JOIN rock.chart c USING(chart_id)
WHERE
	s.title ILIKE '%boogie wonderland%';
;
