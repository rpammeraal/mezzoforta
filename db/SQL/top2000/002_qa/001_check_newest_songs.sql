SELECT 
	'check newest songs -- songs may already exist',
	song_title,
	performer,
	year_released
FROM 
	top2000.top2000all 
ORDER BY 
	year_released, 
	performer, 
	song_title
;
