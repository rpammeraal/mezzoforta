--	Merge src album to dst
--	****** Assumed that online records can be removed from src *****
--	playlist performances to be moved over from src to dst
BEGIN;

--	Holds src, dst record_id's
CREATE TEMP TABLE parameters AS
SELECT
	'src'			AS type,
	record_id,
	title
FROM
	rock.record
WHERE
	title='Supernatural' AND
	artist_id=684
UNION
SELECT
	'dst'			AS type,
	record_id,
	title
FROM
	rock.record
WHERE
	title='Supernatural2' AND
	artist_id=684
;

SELECT 'parameters',* FROM parameters ORDER BY type DESC
;

--	Hold all performance_id's from both albums
CREATE TEMP TABLE all_performance_id AS
SELECT DISTINCT
	rp.performance_id,
	title					AS song_title
FROM
	rock.record_performance rp
		JOIN rock.performance USING(performance_id)
		JOIN rock.song s USING(song_id)
WHERE
	rp.record_id IN (SELECT record_id FROM parameters)
;
SELECT 'all_performance_id',* FROM all_performance_id ORDER BY performance_id
;

--	Hold translation, but have the subsequent insert FAIL if performance_id from 
--	either src or dst is NULL. 
CREATE TEMP TABLE src_dst_performance_id
(
	performance_id				INT NOT NULL PRIMARY KEY,
	src_record_performance_id		INT,	--	 NOT NULL,	--	uncomment if src contains less items
	src_preferred_online_performance_id	INT,
	dst_record_performance_id		INT NOT NULL,
	dst_preferred_online_performance_id	INT
);

INSERT INTO src_dst_performance_id
SELECT
	ap.performance_id,
	rp_src.record_performance_id		AS src_record_performance_id,
	rp_src.preferred_online_performance_id	AS src_preferred_online_performance_id,
	rp_dst.record_performance_id		AS dst_record_performance_id,
	rp_dst.preferred_online_performance_id	AS dst_preferred_online_performance_id
FROM
	all_performance_id ap
		LEFT JOIN rock.record_performance rp_src ON
			ap.performance_id=rp_src.performance_id AND
			rp_src.record_id IN (SELECT record_id FROM parameters WHERE type='src')
		LEFT JOIN rock.record_performance rp_dst ON
			ap.performance_id=rp_dst.performance_id AND
			rp_dst.record_id IN (SELECT record_id FROM parameters WHERE type='dst')
;
SELECT 'src_dst_performance_id',* FROM src_dst_performance_id
;
SELECT DISTINCT * FROM rock.performance WHERE preferred_record_performance_id IN (SELECT src_record_performance_id FROM src_dst_performance_id UNION SELECT dst_record_performance_id FROM src_dst_performance_id)
;

--	Update playlist_detail
UPDATE
	rock.playlist_detail AS p
SET
	online_performance_id=tt.dst_preferred_online_performance_id
FROM
	src_dst_performance_id tt 
WHERE
	p.online_performance_id=tt.src_preferred_online_performance_id
;

--	NULL out preferred_online_performance_id in record_performances for src,
--	so we can remove online_performances
UPDATE 
	rock.record_performance
SET
	preferred_online_performance_id=NULL
WHERE
	record_performance_id IN (SELECT src_record_performance_id FROM src_dst_performance_id)
;

--	Update preferred_online_performance_id for all other record_performances
UPDATE
	rock.record_performance AS p
SET
	preferred_online_performance_id=dst_preferred_online_performance_id
FROM
	src_dst_performance_id tt
WHERE
	p.preferred_online_performance_id=tt.src_preferred_online_performance_id
;

--SELECT 
--	'interim state of rock.record_performance',
--	* 
--FROM
--	rock.record_performance
--WHERE
--	record_performance_id IN (13685,13687,95334)
--;

SELECT * 
FROM
	rock.online_performance
WHERE
	online_performance_id IN (SELECT src_preferred_online_performance_id FROM src_dst_performance_id)
;
--	Remove online_performances
DELETE FROM
	rock.online_performance
WHERE
	online_performance_id IN (SELECT src_preferred_online_performance_id FROM src_dst_performance_id)
;

----	Adjust preferred_record_performance_id 
UPDATE
	rock.performance AS p
SET
	preferred_record_performance_id=tt.dst_record_performance_id
FROM
	src_dst_performance_id tt 
WHERE
	p.preferred_record_performance_id=tt.src_record_performance_id
;

SELECT DISTINCT * FROM rock.performance WHERE preferred_record_performance_id IN (SELECT src_record_performance_id FROM src_dst_performance_id UNION SELECT dst_record_performance_id FROM src_dst_performance_id)
;

SELECT DISTINCT * FROM rock.performance WHERE preferred_record_performance_id IN (SELECT src_record_performance_id FROM src_dst_performance_id )
;

DELETE FROM 
	rock.record_performance
WHERE 
	record_id IN (SELECT record_id FROM parameters WHERE type='src')
;

DELETE FROM
	rock.record
WHERE
	record_id IN (SELECT record_id FROM parameters WHERE type='src')
;

SELECT 
	'resulting rock.online_performance',
	* 
FROM 
	rock.online_performance 
WHERE 
	online_performance_id IN (SELECT preferred_online_performance_id FROM rock.record_performance WHERE record_id IN (SELECT record_id FROM parameters)) OR 
	record_performance_id IN (SELECT record_performance_id FROM rock.record_performance WHERE record_id IN (SELECT record_id FROM parameters)) OR
	online_performance_id IN (SELECT preferred_online_performance_id FROM rock.record_performance WHERE performance_id IN (SELECT performance_id FROM all_performance_id))  OR
	record_performance_id IN (SELECT record_performance_id FROM rock.record_performance WHERE performance_id IN (SELECT performance_id FROM all_performance_id)) 
;

COMMIT;

