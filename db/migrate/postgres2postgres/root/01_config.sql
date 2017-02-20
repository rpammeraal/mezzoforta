BEGIN;

ALTER TABLE config_host RENAME TO config_host_old;

CREATE TABLE config_host 
( 
	config_host_id  SERIAL PRIMARY KEY NOT NULL, 
	hostname        VARCHAR NOT NULL, 
	local_data_path VARCHAR NOT NULL 
); 

INSERT INTO config_host (config_host_id,hostname,local_data_path)
SELECT host_id,hostname,local_data_path FROM config_host_old;

SELECT COUNT(*) FROM config_host_old;
SELECT COUNT(*) FROM config_host;

COMMIT;
