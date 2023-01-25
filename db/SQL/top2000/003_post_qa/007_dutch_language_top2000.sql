DROP TABLE IF EXISTS top2000.dutch_language_top2000
;
CREATE TABLE top2000.dutch_language_top2000
AS
SELECT 
	ROW_NUMBER() OVER(ORDER BY top2000.rank ) AS "rank",
	top2000.song_title,
	top2000.performer,
	top2000.total_points,
	top2000.performance_id 
FROM 
	top2000.top2000all top2000
		JOIN rock.performance_tag pt USING(performance_id) 
		JOIN rock.tag t USING(tag_id)
WHERE 
	t.name='Nederlandstalig'
ORDER BY 
	1
;
