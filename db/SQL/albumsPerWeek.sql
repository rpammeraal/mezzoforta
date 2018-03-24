DROP TABLE IF EXISTS albumsPerWeek
;
CREATE TEMP TABLE albumsPerWeek 
AS
SELECT
	(op.last_play_date::DATE - '2017-01-01'::DATE)/7 AS week,
	COUNT(DISTINCT a.record_id) AS num_albums
FROM
	rock.all a
		JOIN rock.online_performance op USING(online_performance_id)
WHERE
	a.record_id>2456 AND
	op.last_play_date IS NOT NULL AND
	op.last_play_date < NOW()
GROUP BY
	(op.last_play_date::DATE - '2017-01-01'::DATE)/7
;

SELECT * FROM albumsPerWeek
;

SELECT COUNT(week),SUM(num_albums)
FROM albumsPerWeek
;
