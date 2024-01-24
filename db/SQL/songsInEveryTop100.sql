SELECT
	s.song_id,
	s.title					AS song_title,
	a.name					AS performer,
	SUM(CASE WHEN cp.chart_position <= 100 THEN 1 ELSE 0 END ) occurences
FROM
	rock.chart c
		JOIN rock.chart_performance cp USING(chart_id)
		JOIN rock.performance USING(performance_id)
		JOIN rock.song s USING(song_id)
		JOIN rock.artist a USING(artist_id)
WHERE
	c.name ILIKE 'Radio 2 Top 2000 of%'
GROUP BY
	s.song_id,
	s.title,
	a.name
HAVING
	SUM(CASE WHEN cp.chart_position <= 100 THEN 1 ELSE 0 END ) >=22
	
ORDER BY 3,2
