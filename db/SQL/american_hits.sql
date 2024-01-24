--DROP TABLE IF EXISTS top2000all
--;
INSERT INTO rock.playlist_detail(playlist_id,playlist_position,online_performance_id,timestamp)
WITH all_top2000_performances AS 
(
	SELECT DISTINCT
		cp.performance_id,
		p.artist_id,
		op.online_performance_id
	FROM
		rock.chart c
			JOIN rock.chart_performance cp USING(chart_id)
			JOIN rock.performance p USING(performance_id)
			JOIN rock.record_performance rp ON
				p.preferred_record_performance_id=record_performance_id
			JOIN rock.online_performance op ON
				rp.preferred_online_performance_id=op.online_performance_id
	WHERE
		c.name ILIKE 'Radio 2 Top 2000%'
),
all_desired_performers  AS
(
	SELECT DISTINCT
		pd.artist_id
	FROM
		rock.playlist pl
			JOIN rock.playlist_detail pd USING(playlist_id)
	WHERE 
		pl.name IN ('Greatest American Rock Bands','Motown Classics')
)
SELECT
	211							AS playlist_id,
	ROW_NUMBER() OVER()			AS playlist_position,
	t2.online_performance_id,
	NOW()
FROM
	all_top2000_performances t2
		JOIN all_desired_performers ad USING(artist_id)

ORDER BY 1
;
