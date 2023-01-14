DROP TABLE IF EXISTS top2000.top100
;
CREATE TABLE top2000.top100 AS
WITH calc AS 
(
	SELECT
		cp.performance_id,
		SUM(2001 - cp.chart_position) AS points,
		COUNT(c.name) AS num_lists
	FROM
		rock.chart c
			JOIN rock.chart_performance cp USING(chart_id)
	WHERE
		c.name ILIKE 'Radio 2 Top 2000%'
	GROUP BY
		cp.performance_id
),
raw_duration AS
(
	SELECT DISTINCT
		c.performance_id,
		(((EXTRACT(epoch FROM (duration + '15'::INTERVAL))::INT/30) * 30)::INT) * INTERVAL '1 second'	AS duration
	FROM
		calc c
			JOIN rock.record_performance rp USING(performance_id)
),
duration AS
(
	SELECT
		performance_id,
		STRING_AGG(duration::TEXT, ', ')							AS durations
	FROM
		raw_duration
	GROUP BY
		performance_id
)
SELECT
	ROW_NUMBER() OVER(ORDER BY c.points DESC) AS "rank",
	s.title AS song_title,
	a.name AS performer,
	d.durations
FROM
	calc c
		JOIN duration d USING(performance_id)
		JOIN rock.performance p USING(performance_id)
		JOIN rock.song s USING(song_id)
		JOIN rock.artist a USING(artist_id)
ORDER BY 1
LIMIT 100
;
