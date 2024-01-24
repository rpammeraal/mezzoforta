SELECT
	id,
	artist_name,
	record_title,
	MIN(id) OVER(PARTITION BY artist_name,record_title ORDER BY play_datetime) 		AS min_id_by_album,
	MAX(id) OVER(PARTITION BY artist_name,record_title ORDER BY play_datetime DESC) AS max_id_by_album,
	num_songs_on_record
FROM
	{{ref('eligible_songs')}}

