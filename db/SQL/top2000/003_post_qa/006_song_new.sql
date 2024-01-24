WITH latest_top2000 AS
(
	SELECT 
		name, 
		release_date,
		EXTRACT(year FROM release_date)::INT		AS year_int,
		EXTRACT(year FROM release_date)::VARCHAR	AS year_txt
	FROM 
		rock.chart 
	WHERE 
		name ILIKE 'Radio 2 Top 2000 of%' 
	ORDER BY 
		release_date DESC 
	LIMIT 1
),
prev_top2000 AS
(
	SELECT 
		name, 
		release_date,
		EXTRACT(year FROM release_date)::INT		AS year_int,
		EXTRACT(year FROM release_date)::VARCHAR	AS year_txt
	FROM 
		rock.chart 
	WHERE 
		name ILIKE 'Radio 2 Top 2000 of%' 
	ORDER BY 
		release_date DESC 
	LIMIT 1
	OFFSET 1
),
everything AS
(
	SELECT
		*
	FROM
		top2000.top2000all
			CROSS JOIN prev_top2000 pt2
	WHERE 
		appears_in NOT LIKE '%' || pt2.year_txt || '%'
),
only_this_year AS
(
	SELECT
		*
	FROM
		top2000.top2000all
			CROSS JOIN latest_top2000 lt2
			CROSS JOIN prev_top2000 pt2
	WHERE 
		appears_in NOT LIKE '%' || pt2.year_txt || '%' AND
		appears_in LIKE '%' || lt2.year_txt || '%'
)
INSERT INTO top2000.newcomers_removed
SELECT 
	'NEWCOMER'		AS type,
	o.song_title,
	o.performer,
	CASE
		WHEN LENGTH(e.appears_in)>4 THEN 'Laatst in de top 2000 van ' ||regexp_replace(REPLACE(e.appears_in,',2023',''),'.*,', '')
		ELSE 'Nieuw in ' || lt2.year_txt
	END 			AS comments
FROM 
	only_this_year o
		LEFT JOIN everything e ON
			o.song_title=e.song_title AND
			o.performer=e.performer
		CROSS JOIN latest_top2000 lt2

ORDER BY
	o.song_title,
	o.rank
;


