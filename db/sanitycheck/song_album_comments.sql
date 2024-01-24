WITH all_titles AS
(
	SELECT
		s.title			AS song_title,
		r.title			AS record_title,
		a.artist_id,
		a.name			AS performer
	FROM
		rock.song s
			JOIN rock.performance p USING(song_id)
			JOIN rock.artist a USING(artist_id)
			JOIN rock.record_performance USING(performance_id)
			JOIN rock.record r USING(record_id)
)
SELECT
	at1.song_title,
	at2.song_title,
	at1.performer,
	at1.record_title,
	at2.record_title
FROM
	all_titles at1
		JOIN all_titles at2 ON
			at1.artist_id=at2.artist_id AND
			at1.song_title != at2.song_title AND
			at1.song_title || ' (' = LEFT(at2.song_title,LENGTH(at1.song_title)+2)
ORDER BY
	at1.performer,
	at1.song_title


;
