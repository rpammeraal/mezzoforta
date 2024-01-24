BEGIN;

DROP TABLE IF EXISTS to_delete;

CREATE TEMP TABLE to_delete AS
SELECT	
	s.song_id,
	p.performance_id,
	p.preferred_record_performance_id,
	rp.record_performance_id,
	rp.preferred_online_performance_id,
	op.online_performance_id,
	rp.record_id
FROM
	rock.record_performance rp
		LEFT JOIN rock.online_performance op ON
			rp.preferred_online_performance_id=op.online_performance_id
		LEFT JOIN rock.performance p ON
			p.preferred_record_performance_id=rp.record_performance_id
		LEFT JOIN rock.song s ON
			s.original_performance_id=p.performance_id
WHERE
	p.preferred_record_performance_id=rp.record_performance_id AND
	rp.preferred_online_performance_id=op.online_performance_id AND
	rp.record_performance_id  IN (14847,14849,14855,14857,14859,14861,14863,14865,14867,14869,14874);
;
SELECT 'to_delete',* FROM to_delete
;

SELECT 'all',* FROM rock.all WHERE record_id IN (SELECT record_id FROM to_delete) ORDER BY path
;

--	Update ref to online_performance
UPDATE 
	rock.record_performance 
SET 
	preferred_online_performance_id=NULL 
WHERE 
	preferred_online_performance_id IN (SELECT preferred_online_performance_id FROM to_delete)
;

--	Delete online_performance
DELETE FROM rock.online_performance WHERE record_performance_id IN (SELECT record_performance_id FROM to_delete)
;

--	Update performance
UPDATE
	rock.performance
SET
	preferred_record_performance_id=NULL
WHERE
	preferred_record_performance_id IN (SELECT record_performance_id FROM to_delete)
;

--	Delete record_performance
DELETE FROM rock.record_performance WHERE record_performance_id IN (SELECT record_performance_id FROM to_delete)
;

--	Update song
UPDATE 
	rock.song
SET
	original_performance_id=NULL
WHERE
	original_performance_id IN (SELECT performance_id FROM to_delete)
;

--	Delete performance
DELETE FROM rock.performance WHERE performance_id IN (SELECT performance_id FROM to_delete)
;

SELECT * FROM rock.performance WHERE song_id IN (SELECT song_id FROM to_delete);

--	Delete song
DELETE FROM rock.song WHERE song_id IN (SELECT song_id FROM to_delete)
;

SELECT 'all',* FROM rock.all WHERE record_id IN (SELECT record_id FROM to_delete) ORDER BY path;

--	
COMMIT;

