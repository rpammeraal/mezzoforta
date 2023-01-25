DELETE FROM rock.playlist_detail
WHERE playlist_id IN (SELECT playlist_id FROM rock.playlist WHERE name='Top 2000 Of All Times')
;

INSERT INTO rock.playlist (name,created,play_mode)
SELECT 
	'Top 2000 Of All Times',
	NOW(),
	0
WHERE
	NOT EXISTS
	(
		SELECT 
			NULL
		FROM
			rock.playlist WHERE name='Top 2000 Of All Times'
	)
;

INSERT INTO rock.playlist_detail(playlist_id,playlist_position,online_performance_id,timestamp)
SELECT
	p.playlist_id,
	t.rank,
	t.online_performance_id,
	NOW()
FROM
	rock.playlist  p,
	top2000.top2000all t
WHERE 
	p.name='Top 2000 Of All Times' AND
	t.rank <= 2000
ORDER BY
	t.rank
;

