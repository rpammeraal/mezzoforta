WITH everything AS
(
	SELECT
		*
	FROM
		top2000.top2000all
	WHERE 
		song_title ILIKE '%(live)%'
)
SELECT
	'Check for songs that includes the (live) text',
	performer,
	string_agg(song_title,','),
	COUNT(*)
FROM
	everything
GROUP BY
	performer
HAVING
	COUNT(*) >0
ORDER BY 1
;


