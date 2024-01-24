--RM 1565
--CREATE TEMP TABLE tobedeleted
--AS
SELECT
	r.record_id,
	rp.record_position,
	p.song_id,
	r.title,
	p.performance_id,
	p.preferred_record_performance_id,
	record_performance_id,
	preferred_online_performance_id,
	online_performance_id,
	path
FROM
	rock.record r
		LEFT JOIN rock.record_performance rp USING(record_id)
		LEFT JOIN rock.online_performance op USING(record_performance_id)
		LEFT JOIN rock.performance p USING(performance_id)
WHERE
	r.title='Maha Maya - Shri Durga Remixed'
ORDER BY
	2,1

/*


*/
