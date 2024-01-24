SELECT DISTINCT
	artist_name,
	record_title
FROM
	{{ref('conseq_songs')}}
WHERE
	max_id_by_album - (min_id_by_album - 1) = num_songs_on_record 

