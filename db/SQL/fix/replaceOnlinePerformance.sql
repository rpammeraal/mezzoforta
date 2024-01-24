BEGIN
;
DROP TABLE IF EXISTS parameters
;
CREATE TEMP TABLE parameters
AS 
SELECT
    1698             AS record_performance_id,
    'S/Simon\ \&\ Garfunkel/The\ Simon\ \&\ Garfunkel\ Collection/Simon\ \&\ Garfunkel\ \[The\ Simon\ \&\ Garfunkel\ Collection\]\ 17\ -\ Bridge\ Over\ Troubled\ Water.ogg'
                    AS path
;

INSERT INTO rock.online_performance (record_performance_id,path)
SELECT 
    record_performance_id,
    path
FROM
    parameters
;

UPDATE 
    rock.record_performance
SET    
    preferred_online_performance_id= 
        (
            SELECT 
                online_performance_id
            FROM   
                rock.online_performance
            ORDER BY 
                online_performance_id DESC
            LIMIT  
                1
        )
WHERE
    record_performance_id IN (SELECT record_performance_id FROM parameters)
;

\i fix/showRecord.sql
SELECT * FROM rock.online_performance WHERE online_performance_id IN (SELECT online_performance_id FROM parameters) ORDER BY 1 DESC LIMIT 1
;
SELECT * FROM rock.record_performance WHERE record_performance_id IN (SELECT record_performance_id FROM parameters)
;
