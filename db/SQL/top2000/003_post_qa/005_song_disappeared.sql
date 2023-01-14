DROP TABLE IF EXISTS top2000.newcomers_removed
;
CREATE TABLE top2000.newcomers_removed AS
WITH latest_top2000 AS
(
	SELECT 
		name, 
		release_date,
		EXTRACT(year FROM release_date)::INT		AS year_int,
		EXTRACT(year FROM release_date)::VARCHAR	AS year_txt
	FROM 
		rock.chart 
	WHERE 
		name ILIKE 'Radio 2 Top 2000 of%' 
	ORDER BY 
		release_date DESC 
	LIMIT 1
),
everything AS
(
	SELECT
		*
	FROM
		top2000.top2000all
			CROSS JOIN latest_top2000 lt2
	WHERE 
		appears_in NOT LIKE '%' || lt2.year_txt || '%'
),
only_this_year AS
(
	SELECT
		s.title AS song_title,
		a.name AS performer
	FROM
		rock.chart c
			JOIN rock.chart_performance cp USING(chart_id)
			JOIN rock.performance p USING(performance_id)
			JOIN rock.artist a USING(artist_id)
			JOIN rock.song s USING(song_id)
			CROSS JOIN latest_top2000 lt2
	WHERE
		c.name=lt2.name
)
SELECT 
	'REMOVED'					AS type,
	song_title,
	performer,
	regexp_replace(appears_in, '.*,', '')::VARCHAR 	AS comments
FROM 
	everything o
WHERE
	NOT EXISTS
	(
		SELECT
			NULL
		FROM
			only_this_year e
		WHERE
			e.song_title=o.song_title AND
			e.performer=o.performer
	)
