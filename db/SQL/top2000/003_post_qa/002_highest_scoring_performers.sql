DROP TABLE IF EXISTS top2000.highest_scoring_performers;

CREATE TABLE top2000.highest_scoring_performers
AS
WITH everything AS
(
	SELECT DISTINCT
		performer,
		SUM(total_points) OVER(PARTITION BY performer) 					AS total_points,
		STRING_AGG(song_title::TEXT, ', ') OVER(PARTITION BY performer)	AS songs
	FROM
		top2000.top2000all
)
SELECT
	ROW_NUMBER() OVER(ORDER BY total_points DESC)						AS score,
	performer,
	songs
FROM
	everything
ORDER BY
	1
LIMIT 100
;
