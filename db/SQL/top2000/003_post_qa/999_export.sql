\copy (SELECT * FROM top2000.newcomers_removed WHERE type='REMOVED')                                                                                                                            TO 'csv/001_removed.csv' WITH CSV HEADER;
\copy (SELECT * FROM top2000.newcomers_removed WHERE type='NEWCOMER')                                                                                                                           TO 'csv/002_newcomers.csv' WITH CSV HEADER;
\copy (SELECT rank AS position,song_title AS "song title",performer,appears_in AS "appears in" FROM top2000.top2000all ORDER BY position LIMIT 100)                                             TO 'csv/003_top100.csv' WITH CSV HEADER;
\copy (SELECT DISTINCT         song_title AS "song title",performer,appears_in AS "appears in" FROM top2000.top2000all ORDER BY 2,1)                                                            TO 'csv/004_allsongs.csv' WITH CSV HEADER;
\copy (SELECT ROW_NUMBER() OVER(ORDER BY total_points DESC,appears_in), song_title AS "song title",performer,appears_in AS "appears in" FROM top2000.top2000all  WHERE num_lists=1 ORDER BY 1)  TO 'csv/005_appearsOnce.csv' WITH CSV HEADER;
\copy (SELECT decade,"rank" AS position,song_title,performer                                   FROM top2000.top2000bydecade)                                                                    TO 'csv/006top2000bydecade.csv' WITH CSV HEADER;
\copy (SELECT rank AS position,song_title AS "song title",performer,appears_in AS "appears in" FROM top2000.top2000all ORDER BY position LIMIT 2000)                                            TO 'csv/007_top2000allertijden.csv' WITH CSV HEADER;

\copy (SELECT                  song_title AS "song title",performer                            FROM top2000.top2000all WHERE appears_in NOT LIKE '%2023%' ORDER BY 1,2)                         TO 'csv/008_afkicklijst.csv' WITH CSV HEADER;
\copy (SELECT rank AS position,song_title AS "song title",performer                            FROM top2000.dutch_language_top2000 ORDER BY 1,2) 			                                    TO 'csv/009_dutch_language.csv' WITH CSV HEADER;

