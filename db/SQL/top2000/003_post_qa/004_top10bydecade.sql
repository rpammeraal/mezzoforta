DROP TABLE IF EXISTS top2000.top2000bydecade
;

CREATE TABLE top2000.top2000bydecade
AS
WITH everything AS
(
	SELECT
		song_title,
		performer,
		year_released,
		total_points,
		'Jaren ' || CASE
			WHEN year_released < 2000 THEN ((year_released - 1900)/10)::VARCHAR || '0'
			ELSE '20' || ((year_released - 2000)/10)::VARCHAR || '0'
		END													AS decade,
		((year_released - 1900)/10)*10										AS numerical_decade
	FROM
		top2000.top2000all
	ORDER BY
		year_released DESC
),
point_count AS
(
	SELECT DISTINCT
		song_title,
		performer,
		decade,
		numerical_decade,
		SUM(total_points) OVER(PARTITION BY song_title,performer,decade) 					AS total_points
	FROM
		everything
),
rank_by_decade AS
(
	SELECT
		song_title,
		performer,
		decade,
		numerical_decade,
		total_points,
		ROW_NUMBER() OVER(PARTITION BY decade ORDER BY total_points DESC) 					AS rank
	FROM
		point_count
)
SELECT
	decade,
	rank,
	song_title,
	performer
FROM
	rank_by_decade
WHERE
	rank<=10 AND
	numerical_decade>40
ORDER BY
	numerical_decade,
	rank
;
