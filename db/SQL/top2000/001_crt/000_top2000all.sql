DROP SCHEMA IF EXISTS top2000 CASCADE
;
CREATE SCHEMA top2000
;
DROP TABLE IF EXISTS top2000.top2000all
;
CREATE TABLE top2000.top2000all AS
WITH calc AS 
(
	SELECT
		cp.performance_id,
		SUM(2001 - cp.chart_position) AS points,
		STRING_AGG(REPLACE(c.name, 'Radio 2 Top 2000 of ',''), ',' ORDER BY c.name) AS years,
		COUNT(c.name) AS num_lists
	FROM
		rock.chart c
			JOIN rock.chart_performance cp USING(chart_id)
	WHERE
		c.name ILIKE 'Radio 2 Top 2000%'
	GROUP BY
		cp.performance_id
)
SELECT
	ROW_NUMBER() OVER(ORDER BY c.points DESC) AS "rank",
	s.title AS song_title,
	a.name AS performer,
	c.points AS total_points,
	--LEFT(c.years, 49) AS appears_in,
	c.years AS appears_in,
	p.year AS year_released,
	num_lists,
	c.performance_id,
	o_p.online_performance_id
FROM
	calc c
		JOIN rock.performance p USING(performance_id)
		JOIN rock.song s USING(song_id)
		JOIN rock.artist a USING(artist_id)
		LEFT JOIN rock.record_performance r_p ON
			p.preferred_record_performance_id=r_p.record_performance_id
		LEFT JOIN rock.online_performance o_p ON
			r_p.preferred_online_performance_id=o_p.online_performance_id
ORDER BY 1
;
