SELECT
	s.song_id,
	s.title,
	a.name,
	p.performance_id,
	c.chart_id,
	c.name,
	cp.chart_position,
	cp.chart_performance_id
FROM
	rock.song s
		LEFT JOIN rock.performance p USING(song_id)
		LEFT JOIN rock.artist a USING(artist_id)
		LEFT JOIN rock.chart_performance cp USING(performance_id)
		LEFT JOIN rock.chart c USING(chart_id)
WHERE
	s.title='Like The Way I Do'
ORDER BY
	c.name,
	cp.chart_position

 /*

 preferred_online_performance_id | online_performance_id 
---------------------------------+-----------------------
UPDATE rock.record_performance SET preferred_online_performance_id=                           19584 WHERE preferred_online_performance_id=                 15940 ;
UPDATE rock.record_performance SET preferred_online_performance_id=                           19583 WHERE preferred_online_performance_id=                 15939 ;
UPDATE rock.record_performance SET preferred_online_performance_id=                           19589 WHERE preferred_online_performance_id=                 15945 ;
UPDATE rock.record_performance SET preferred_online_performance_id=                           19588 WHERE preferred_online_performance_id=                 15944 ;
UPDATE rock.record_performance SET preferred_online_performance_id=                           19590 WHERE preferred_online_performance_id=                 15946 ;
UPDATE rock.record_performance SET preferred_online_performance_id=                           19591 WHERE preferred_online_performance_id=                 15947 ;
UPDATE rock.record_performance SET preferred_online_performance_id=                           19592 WHERE preferred_online_performance_id=                 15948 ;
UPDATE rock.record_performance SET preferred_online_performance_id=                           19596 WHERE preferred_online_performance_id=                 15952 ;
UPDATE rock.record_performance SET preferred_online_performance_id=                           19595 WHERE preferred_online_performance_id=                 15951 ;
UPDATE rock.record_performance SET preferred_online_performance_id=                           19597 WHERE preferred_online_performance_id=                 15953 ;
UPDATE rock.record_performance SET preferred_online_performance_id=                           19586 WHERE preferred_online_performance_id=                 15942 ;
UPDATE rock.record_performance SET preferred_online_performance_id=                           19585 WHERE preferred_online_performance_id=                 15941 ;
UPDATE rock.record_performance SET preferred_online_performance_id=                           19594 WHERE preferred_online_performance_id=                 15950 ;
UPDATE rock.record_performance SET preferred_online_performance_id=                           19593 WHERE preferred_online_performance_id=                 15949 ;
UPDATE rock.record_performance SET preferred_online_performance_id=                           19587 WHERE preferred_online_performance_id=                 15943 

BEGIN;
UPDATE rock.performance SET preferred_record_performance_id=preferred_record_performance_id+1 WHERE performance_id IN ( 22235, 22159, 22237, 14329, 14330, 22240, 22241, 22242, 14328);


DELETE FROM rock.record_performance WHERE record_performance_id IN (19602, 19604, 19606, 19608, 19610, 19612, 19614, 19617, 23095);


*/
