BEGIN;

ALTER TABLE ---SQL_SCHEMA_NAME---chart_performance RENAME TO chart_performance_old;

CREATE TABLE ---SQL_SCHEMA_NAME---chart_performance 
( 
	chart_performance_id            ---AUTOID--- PRIMARY KEY NOT NULL, 
	chart_id                        INT NOT NULL,
	performance_id                  INT NOT NULL, 
	chart_position                  INT NOT NULL,
	notes                           TEXT, 

	CONSTRAINT cc_chart_performance_chart_position_check CHECK ((chart_position >= 0)), 

	CONSTRAINT fk_chart_performance_chart_id_chart_chart_id FOREIGN KEY (chart_id) REFERENCES ---SQL_SCHEMA_NAME---chart(chart_id),
	CONSTRAINT fk_chart_performance_performance_id_performance_performance_id FOREIGN KEY (performance_id) REFERENCES ---SQL_SCHEMA_NAME---performance(performance_id) 
); 

INSERT INTO ---SQL_SCHEMA_NAME---chart_performance 
(
	chart_id,
	performance_id,
	chart_position,
	notes
)
SELECT DISTINCT
	chart_id,
	c.performance_id,
	cpo.chart_position,
	cpo.notes
FROM
	---SQL_SCHEMA_NAME---chart_performance_old cpo
		JOIN conversion c ON
			cpo.artist_id=c.artist_id AND
			cpo.song_id=c.song_id
WHERE
	c.performance_id IS NOT NULL
;

SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---chart_performance_old;
SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---chart_performance;

COMMIT;
