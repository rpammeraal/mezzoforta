BEGIN;

ALTER TABLE rock.performance RENAME TO performance_old;

CREATE TABLE rock.performance 
( 
	performance_id                  SERIAL PRIMARY KEY NOT NULL, 
	song_id                         INT NOT NULL, 
	artist_id                       INT NOT NULL, 
	preferred_record_performance_id INT NULL,
	role_id                         SMALLINT NOT NULL, 
	year                            INT, 
	notes                           TEXT, 
	CONSTRAINT cc_performance_role_id_check CHECK (((role_id >= 0) AND (role_id <= 1))), 
	CONSTRAINT cc_performance_year_check CHECK (((year IS NULL) OR (year >= 1900))), 
	CONSTRAINT fk_performance_song_id_song_song_id FOREIGN KEY (song_id) REFERENCES rock.song(song_id) 
	CONSTRAINT fk_performance_preferred_record_performance_id_record_performance_record_performance_id FOREIGN KEY (preferred_record_performance_id) REFERENCES rock.record_performance(record_performance_id);
); 

INSERT INTO rock.performance (song_id,artist_id,role_id,year,notes)

WITH prefRecordPerformance AS
(
	SELECT
		performance_id,
		MIN(record_performance_id)
		OVER (PARTITION BY performance_id
			ORDER BY record_performance_id) AS record_performance_id

	FROM
		rock.record_performance p
)
UPDATE rock.performance AS p
SET
	preferred_record_performance_id=t.record_performance_id
FROM
	prefRecordPerformance t
WHERE
	p.performance_id=t.performance_id
;
SELECT * FROM rock.performance_old;

SELECT COUNT(*) FROM rock.performance_old;
SELECT COUNT(*) FROM rock.performance;


COMMIT;
