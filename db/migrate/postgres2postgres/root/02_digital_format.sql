BEGIN;

ALTER TABLE digital_format RENAME TO digital_format_old;

CREATE TABLE digital_format 
( 
	digital_format_id SERIAL PRIMARY KEY NOT NULL, 
	name              VARCHAR NOT NULL, 
	extension         VARCHAR NOT NULL 
); 

INSERT INTO digital_format (digital_format_id,name,extension)
SELECT format_id,name,extension FROM digital_format_old;

SELECT COUNT(*) FROM digital_format_old;
SELECT COUNT(*) FROM digital_format;

COMMIT;
