SELECT
	s.song_id,
	s.title					AS song_title,
	a.name					AS performer,
	MIN(cp.chart_position)	AS min_chart_position,
	SUM(CASE WHEN cp.chart_position <= 100 THEN 1 ELSE 0 END ) num_below_100
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
	SUM(CASE WHEN cp.chart_position <= 100 THEN 1 ELSE 0 END ) =1
	
ORDER BY 3,2
