SELECT 
	record_title,
	COUNT(artist_name) OVER(PARTITION BY record_title) 								AS num_artists_on_record
FROM
	{{ref('num_songs_on_record')}}

