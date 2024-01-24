WITH all_songs AS
(
	SELECT DISTINCT
		a.artist_id,
		a.name,
		s.title,
		s.song_id
	FROM
		rock.chart c
			JOIN rock.chart_performance cp USING(chart_id)
			JOIN rock.performance p USING(performance_id)
			JOIN rock.artist a USING(artist_id)
			JOIN rock.song s USING(song_id)
	WHERE
		c.name ILIKE 'Radio 2 Top 2000 of%' AND
		c.name!='Radio 2 Top 2000 of 2023'
),
artists AS
(
	SELECT DISTINCT
		a.artist_id,
		a.name
	FROM
		all_songs a
)
SELECT 
	s.title,
	a.name,
	p.year
FROM
	artists a
		JOIN rock.performance p USING(artist_id)
		JOIN rock.song s USING(song_id)
		JOIN rock.chart_performance USING(performance_id)
		JOIN rock.chart c USING(chart_id)
		LEFT JOIN all_songs a_s USING(artist_id,song_id)
WHERE
	c.name='Radio 2 Top 2000 of 2023' AND
	p.year>=2020 AND
	a_s IS NULL
ORDER BY 2,1
;
