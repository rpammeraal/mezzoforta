WITH top2020 AS
(
	SELECT
		p.performance_id
	FROM
		rock.chart c
			JOIN rock.chart_performance cp USING(chart_id)
			JOIN rock.performance p USING(performance_id)
			JOIN rock.song s USING(song_id)
			JOIN rock.artist a USING(artist_id)
	WHERE
		c.name='Radio 2 Top 2000 of 2020'
)
SELECT

	s.song_id,
	s.title					AS song_title,
	a.name					AS performer,
	MIN(cp.chart_position)	AS min_chart_position
FROM
	rock.chart c
		JOIN rock.chart_performance cp USING(chart_id)
		JOIN rock.performance p USING(performance_id)
		JOIN rock.song s USING(song_id)
		JOIN rock.artist a USING(artist_id)
WHERE
	c.name ILIKE 'Radio 2 Top 2000 of%' AND
	c.name!='Radio 2 Top 2000 of 2020' AND
	p.performance_id NOT IN (SELECT performance_id FROM top2020)
GROUP BY
	s.song_id,
	s.title,
	a.name
HAVING
	MIN(cp.chart_position)<=100
	
ORDER BY 3,2
