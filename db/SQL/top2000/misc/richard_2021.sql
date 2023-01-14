SELECT 
	rank,
	song_title,
	performer 
FROM 
	top2000all
WHERE
	(lower(song_title) = lower('A Whiter Shade Of Pale') AND lower(performer)=lower('procol harum'))
	OR
	(lower(song_title) ILIKE '%Unforgettable%' AND lower(performer)=lower('u2'))
	OR
	(lower(song_title) ILIKE '%eagle%' AND lower(performer)=lower('abba'))
	OR
	(lower(song_title) ILIKE '%thunderstruck%' AND lower(performer)=lower('ac/dc'))
	OR
	(lower(song_title) ILIKE '%proud%' AND lower(performer) ILIKE '%creedence%')
	OR
	(lower(song_title) ILIKE '%telegraph%' AND lower(performer) ILIKE '%dire%')
	OR
	(lower(song_title) ILIKE '%brabant%' AND lower(performer) ILIKE '%meeuwis%')
	OR
	(lower(song_title) ILIKE '%maasweg%' AND lower(performer) ILIKE '%amazing%')
	OR
	(lower(song_title) ILIKE '%dag of%' AND lower(performer) ILIKE '%doe%')
	OR
	(lower(song_title) ILIKE '%den haag%' AND lower(performer) ILIKE '%klork%')
	OR
	(lower(song_title) ILIKE '%stairway%' AND lower(performer) ILIKE '%zeppelin%')
	OR
	(lower(song_title) ILIKE '%air%' AND lower(performer) ILIKE '%collins%')
	OR
	(lower(song_title) ILIKE '%feeling%' AND lower(performer) ILIKE '%boston%')
	OR
	(lower(song_title) ILIKE '%matters%' AND lower(performer) ILIKE '%metallica%')
	OR
	(lower(song_title) ILIKE '%disco%' AND lower(performer) ILIKE '%trammps%')
	OR
	(lower(song_title) ILIKE '%wish%' AND lower(performer) ILIKE '%pink floyd%')
	OR
	(lower(song_title) ILIKE '%black%' AND lower(performer) ILIKE '%ram%')
	OR
	(lower(song_title) ILIKE '%not there%' AND lower(performer) ILIKE '%santana%')
	OR
	(lower(song_title) ILIKE '%hallelujah%' AND lower(performer) ILIKE '%buckley%')
	OR
	(lower(song_title) ILIKE '%night fever%' AND lower(performer) ILIKE '%gees%')
	OR
	(lower(song_title) ILIKE '%park%' AND lower(performer) ILIKE '%donna%')
	OR
	(lower(song_title) ILIKE '%yourself%' AND lower(performer) ILIKE '%eminem%')
	OR
	(lower(song_title) ILIKE '%sells%' AND lower(performer) ILIKE '%cult%')
	OR
	(lower(song_title) ILIKE '%cairo%' AND lower(performer) ILIKE '%madness%')
	OR
	(lower(song_title) ILIKE '%wake%' AND lower(performer) ILIKE '%avicii%')
	OR
	(lower(song_title) ILIKE '%wake%' AND lower(performer) ILIKE '%wham%')
	OR
	(lower(song_title) ILIKE '%school%' AND lower(performer) ILIKE '%supertramp%')
	OR
	(lower(song_title) ILIKE '%min%' AND lower(performer) ILIKE '%armand%')
	OR
	(lower(song_title) ILIKE '%belfast%' AND lower(performer) ILIKE '%simple%')
	OR
	(lower(song_title) ILIKE '%enola%' AND lower(performer) ILIKE '%dark%')
	OR
	(lower(song_title) ILIKE '%jean%' AND lower(performer) ILIKE '%michael%')
	OR
	(lower(song_title) ILIKE '%man%' AND lower(performer) ILIKE '%r.e.m%')
	OR
	(lower(song_title) ILIKE '%pastorale%' AND lower(performer) ILIKE '%liesbeth%')
	OR
	(lower(song_title) ILIKE '%radar%' AND lower(performer) ILIKE '%golden%')
	OR
	(lower(song_title) ILIKE '%somebody%' AND lower(performer) ILIKE '%blues%')

;
