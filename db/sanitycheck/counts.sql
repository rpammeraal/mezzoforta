WITH a AS
(
	SELECT
		'artist',
		COUNT(*)
	FROM
		rock.artist
	UNION
	SELECT
		'song',
		COUNT(*)
	FROM
		rock.song
	UNION
	SELECT
		'performance',
		COUNT(*)
	FROM
		rock.performance
	UNION
	SELECT
		'record_performance',
		COUNT(*)
	FROM
		rock.record_performance
	UNION
	SELECT
		'record',
		COUNT(*)
	FROM
		rock.record
	UNION
	SELECT
		'online_performance',
		COUNT(*)
	FROM
		rock.online_performance
)
SELECT
	*
FROM
	a
ORDER BY
	2 DESC
;