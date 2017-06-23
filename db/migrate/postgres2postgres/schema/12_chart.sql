BEGIN;

ALTER TABLE ---SQL_SCHEMA_NAME---chart RENAME TO chart_old;

CREATE TABLE ---SQL_SCHEMA_NAME---chart 
( 
	chart_id      ---AUTOID--- PRIMARY KEY NOT NULL, 
	name          CHARACTER VARYING NOT NULL, 
	release_date  DATE NOT NULL,
	notes         TEXT 
); 

INSERT INTO ---SQL_SCHEMA_NAME---chart 
(
	chart_id,
	name,
	release_date,
	notes
)
SELECT 
	chart_id,
	name,
	release_date,
	notes 
FROM 
	---SQL_SCHEMA_NAME---chart_old;

SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---chart;
SELECT COUNT(*) FROM ---SQL_SCHEMA_NAME---chart_old;

COMMIT;
