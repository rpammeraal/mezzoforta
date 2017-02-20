BEGIN;

ALTER TABLE rock.toplay RENAME TO toplay_old;


CREATE TABLE rock.toplay 
( 
	toplay_detail_id      INT PRIMARY KEY AUTOINCREMENT NOT NULL, 
	online_performance_id INT NOT NULL, 
	insert_order          INT NOT NULL, 
	play_order            INT, 
	last_play_date        TIMESTAMP WITHOUT TIME ZONE, 
	CONSTRAINT fk_toplay_record_performance FOREIGN KEY (online_performance) REFERENCES rock.online_performance(online_performance_id) 
); 

COMMIT;
