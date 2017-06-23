BEGIN;

ALTER TABLE ---SQL_SCHEMA_NAME---toplay RENAME TO toplay_old;


CREATE TABLE ---SQL_SCHEMA_NAME---toplay 
( 
	toplay_detail_id      ---AUTOID--- PRIMARY KEY NOT NULL, 
	online_performance_id INT NOT NULL, 
	insert_order          INT NOT NULL, 
	play_order            INT, 
	last_play_date        TIMESTAMP WITHOUT TIME ZONE, 
	CONSTRAINT fk_toplay_record_performance FOREIGN KEY (online_performance_id) REFERENCES ---SQL_SCHEMA_NAME---online_performance(online_performance_id) 
); 

COMMIT;
