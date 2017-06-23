BEGIN;

DROP TABLE IF EXISTS ---SQL_SCHEMA_NAME---playlist_detail
;

CREATE TABLE ---SQL_SCHEMA_NAME---playlist_detail 
( 
	playlist_detail_id    ---AUTOID--- PRIMARY KEY NOT NULL, 
	playlist_id           INT NOT NULL, 
	playlist_position     INT NOT NULL, 

	online_performance_id INT NULL, 

	child_playlist_id     INT NULL, 
	chart_id              INT NULL, 
	record_id             INT NULL, 
	artist_id             INT NULL, 
	timestamp             DATE NOT NULL, 
	notes                 TEXT, 
	CONSTRAINT cc_playlist_detail_playlist_position_check CHECK ((playlist_position > 0)),
	CONSTRAINT fk_playlist_detail_playlist_playlist_id FOREIGN KEY(playlist_id) REFERENCES ---SQL_SCHEMA_NAME---playlist(playlist_id), 
	CONSTRAINT fk_playlist_detail_online_performance_online_performance_id FOREIGN KEY (online_performance_id) REFERENCES ---SQL_SCHEMA_NAME---online_performance(online_performance_id), 
	CONSTRAINT fk_playlist_detail_playlist_child_playlist_id FOREIGN KEY(child_playlist_id) REFERENCES ---SQL_SCHEMA_NAME---playlist(playlist_id), 
	CONSTRAINT fk_playlist_detail_record_record_id FOREIGN KEY (record_id) REFERENCES ---SQL_SCHEMA_NAME---record(record_id), 
	CONSTRAINT fk_playlist_detail_artist_artist_id FOREIGN KEY (artist_id) REFERENCES ---SQL_SCHEMA_NAME---artist(artist_id) 
); 

--	Insert from playlist_composite
INSERT INTO ---SQL_SCHEMA_NAME---playlist_detail 
(
	playlist_id,
	playlist_position,
	timestamp,
	notes,
	child_playlist_id,
	chart_id,
	record_id,
	artist_id
)
SELECT DISTINCT
	playlist_id,
	playlist_position,
	timestamp,
	notes,
	playlist_playlist_id,
	playlist_chart_id,
	playlist_record_id,
	playlist_artist_id
FROM 
	---SQL_SCHEMA_NAME---playlist_composite
WHERE 
	playlist_id IS NOT NULL;
;

SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---playlist_composite WHERE playlist_id IS NOT NULL;
SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---playlist_detail WHERE online_performance_id IS NULL;
SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---playlist_detail WHERE online_performance_id IS NOT NULL;

--	Insert from playlist_performance
--	Non-translated record_performances
INSERT INTO ---SQL_SCHEMA_NAME---playlist_detail 
(
	playlist_id,
	playlist_position,
	online_performance_id,
	notes,
	timestamp
)
SELECT DISTINCT
	pp.playlist_id,
	pp.playlist_position,
	c.online_performance_id,
	pp.notes,
	pp.timestamp
FROM 
	---SQL_SCHEMA_NAME---playlist_performance pp
		JOIN conversion c ON
			pp.song_id=c.song_id AND
			pp.artist_id=c.artist_id AND
			pp.record_id=c.record_id AND
			pp.record_position=c.record_position
WHERE
	c.online_performance_id IS NOT NULL
;

INSERT INTO ---SQL_SCHEMA_NAME---playlist_detail 
(
	playlist_id,
	playlist_position,
	online_performance_id,
	notes,
	timestamp
)
SELECT DISTINCT
	pp.playlist_id,
	pp.playlist_position,
	c.online_performance_id,
	pp.notes,
	pp.timestamp
FROM 
	---SQL_SCHEMA_NAME---playlist_performance pp
		JOIN ---SQL_SCHEMA_NAME---record_performance_old rp ON
			pp.song_id=rp.song_id AND
			pp.artist_id=rp.artist_id AND
			pp.record_id=rp.record_id AND
			pp.record_position=rp.record_position
		JOIN conversion c ON
			rp.op_song_id=c.song_id AND
			rp.op_artist_id=c.artist_id AND
			rp.op_record_id=c.record_id AND
			rp.op_record_position=c.record_position
WHERE
	c.online_performance_id IS NOT NULL AND
	NOT EXISTS
	(
		SELECT 
			NULL
		FROM
			---SQL_SCHEMA_NAME---playlist_detail pd
		WHERE
			pp.playlist_id=pd.playlist_id AND
			pp.playlist_position=pd.playlist_position
	)
;
			

SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---playlist_composite;
SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---playlist_performance;
SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---playlist_detail WHERE online_performance_id IS NULL;
SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---playlist_detail WHERE online_performance_id IS NOT NULL;

SELECT DISTINCT
	pp.playlist_id,
	pp.playlist_position,
	c.online_performance_id,
	pp.notes,
	pp.timestamp
FROM 
	---SQL_SCHEMA_NAME---playlist_performance pp
		JOIN conversion c ON
			pp.song_id=c.song_id AND
			pp.artist_id=c.artist_id AND
			pp.record_id=c.record_id AND
			pp.record_position=c.record_position
WHERE
	c.online_performance_id IS NULL
LIMIT 5
;
COMMIT
;
