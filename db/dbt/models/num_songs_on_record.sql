SELECT DISTINCT
	artist_name,
	record_title,
	num_songs_on_record
FROM
	{{ref('eligible_songs')}}

