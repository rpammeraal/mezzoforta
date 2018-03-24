SELECT 
	s.song_id,
	s.title,
	INITCAP(s.title),
	a.name,
	c.name,
	r.title
FROM 
	rock.song s
		JOIN rock.performance p USING(song_id)
		JOIN rock.artist a USING(artist_id)
		JOIN rock.chart_performance cp USING(performance_id)
		JOIN rock.chart c USING(chart_id)
		LEFT JOIN rock.record_performance r_p USING(performance_id)
		LEFT JOIN rock.record r USING(record_id)
WHERE 
	s.title!=INITCAP(s.title) AND 
	c.chart_id > 59 AND
	r_p.record_performance_id IS NULL
ORDER BY 
	a.name,
	s.title
