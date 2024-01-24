DROP TABLE IF EXISTS song_id
;

CREATE TEMP TABLE song_id 
AS
SELECT
	s.song_id,
	s.title						AS song_title,
    a.artist_id,
	a.name						AS performer,
    p.performance_id,
	r.record_id,
	r.title						AS album_title,
	rp.record_performance_id,
	rp.record_position,
	op.online_performance_id,
	op.path
FROM
	rock.song s
		LEFT JOIN rock.performance p USING(song_id)
		LEFT JOIN rock.artist a USING(artist_id)
		LEFT JOIN rock.record_performance rp USING(performance_id)
		LEFT JOIN rock.record r USING(record_id)
		LEFT JOIN rock.online_performance op USING(record_performance_id)
WHERE
  s.title ILIKE '%House Of The Rising Sun%'
--  song_id IN (64130)
--	song_id IN (SELECT song_id FROM rock.performance WHERE performance_id IN(
--		     (SELECT performance_id FROM rock.record_performance rp WHERE record_id IN (
--			(SELECT record_id FROM rock.record WHERE title ILIKE '2112%'))))) AND
--	r.title ILIKE '2112%'
--    r.record_id=217
ORDER BY
	rp.record_position
;

SELECT * FROM song_id;


