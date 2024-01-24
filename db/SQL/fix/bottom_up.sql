DROP TABLE IF EXISTS parameters
;
CREATE TEMP TABLE parameters AS
SELECT
	online_performance_id
FROM
	rock.online_performance
WHERE
	online_performance_id IN (12862)
;

DROP TABLE IF EXISTS record_performance_id
;
CREATE TEMP TABLE record_performance_id AS
WITH all_data AS
(
	SELECT 
		online_performance_id,
		record_performance_id,
		'from op using op_id'						AS origin
	FROM
		rock.online_performance
			JOIN parameters USING(online_performance_id)
	UNION
	SELECT
		online_performance_id,
		record_performance_id,
		'from rp using pref_onp_id'
	FROM
		rock.record_performance rp
			JOIN parameters  p ON
				rp.preferred_online_performance_id=p.online_performance_id
),
aggregated AS
(
	SELECT
		online_performance_id,
		record_performance_id,
		COUNT(*)				AS count,
		STRING_AGG(origin,',')			AS origin
	FROM
		all_data
	GROUP BY
		online_performance_id,
		record_performance_id
)
SELECT
	rpi.online_performance_id,
	r.record_id,
	record_performance_id,
	rp.preferred_online_performance_id,
	r.title						AS album_title,
	rpi.origin
FROM
	rock.record_performance rp
		JOIN aggregated rpi USING(record_performance_id)
		JOIN rock.record r USING(record_id)
;

CREATE TEMP TABLE performance_id AS
WITH all_data AS
(
	SELECT
		p.performance_id,
		a_d.record_performance_id,
		'from rp using pref_rp_id'
							AS origin
	FROM
		record_performance_id a_d
			JOIN rock.performance p ON 
				a_d.record_performance_id=p.preferred_record_performance_id
	UNION
	SELECT
		p.performance_id,
		a_d.record_performance_id,
		'from p using p_id'
							AS origin
	FROM
		record_performance_id a_d
			JOIN rock.record_performance r_p USING(record_performance_id)
			JOIN rock.performance p USING(performance_id)
)
SELECT
	performance_id,
	record_performance_id,
	STRING_AGG(origin,',')			AS origin
FROM
	all_data
GROUP BY
	performance_id,
	record_performance_id
;

CREATE TEMP TABLE song_id AS
WITH all_data AS
(
	SELECT
		performance_id,
		p.preferred_record_performance_id,
		s.song_id,
		s.title							AS song_title,
		'ORIGINAL PERFORMANCE'					AS origin,
		a.name							AS artist_name
	FROM
		performance_id p_id
			JOIN rock.performance p USING(performance_id)
			JOIN rock.song s ON 
				p_id.performance_id=original_performance_id
			JOIN rock.artist a USING(artist_id)
	UNION
	SELECT
		performance_id,
		p.preferred_record_performance_id,
		s.song_id,
		s.title							AS song_title,
		'from song using song_id'				AS origin,
		a.name							AS artist_name
	FROM
		performance_id p_id
			JOIN rock.performance p USING(performance_id)
			JOIN rock.song s USING(song_id)
			JOIN rock.artist a USING(artist_id)
)
SELECT
	performance_id,
	song_id,
	song_title,
	artist_name,
	preferred_record_performance_id,
	STRING_AGG(origin,',')						AS origin
FROM
	all_data
GROUP BY
	performance_id,
	song_id,
	song_title,
	artist_name,
	preferred_record_performance_id
;
SELECT
	*
FROM
	song_id
;

SELECT
	rpi.online_performance_id,
	rpi.record_id,
	rpi.album_title,
	rpi.record_performance_id,
	rpi.preferred_online_performance_id,
	si.performance_id,
	si.preferred_record_performance_id,
	pi.origin							AS performance_record_performance,
	si.song_id,
	si.song_title,
	si.artist_name,
	si.origin							AS song_performance
FROM
	record_performance_id rpi
		JOIN performance_id pi USING(record_performance_id)
		JOIN song_id si USING(performance_id)
;

