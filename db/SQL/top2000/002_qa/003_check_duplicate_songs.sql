SELECT
	'check for duplicate songs',
	song_title,
	STRING_AGG(performer, ',' ORDER BY performer),
	COUNT(*)
FROM
	top2000.top2000all
GROUP BY
	song_title
ORDER BY 
	4 DESC
;
