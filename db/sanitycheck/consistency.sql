SELECT
	'Duplicate records',
	r.record_id,
	r.title,
	a.name,
	COUNT(*)
FROM
	rock.record r
		JOIN rock.artist a USING(artist_id)
GROUP BY
	r.title,
	r.record_id,
	a.name
HAVING
	COUNT(*)>1
UNION
SELECT
	'Empty paths',
	op.online_performance_id,
	s.song,
	op.path,
	1
UNION
SELECT 
FROM 
	rock.online_performance op
		JOIN rock.record_performance rp USING(record_performance_id)
		JOIN rock.performance r USING(performance_id)
		JOIN rock.song s USING(song_id)
WHERE 
	COALESCE(op.path,'')='';
