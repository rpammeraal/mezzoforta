SELECT
	s.song_id,
	s.title					AS song_title,
	a.name					AS performer,
	MIN(cp.chart_position)	AS min_chart_position
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
	MIN(cp.chart_position)<=100
ORDER BY 3,2
