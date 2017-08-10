BEGIN;

--	1.	DROP OLD TABLES
DROP TABLE IF EXISTS config_daemon CASCADE;
DROP TABLE IF EXISTS config_host_old CASCADE;
DROP TABLE IF EXISTS genre;
DROP TABLE IF EXISTS config_host_old CASCADE;
DROP TABLE IF EXISTS digital_format_old CASCADE;
DROP TABLE IF EXISTS online_performance_backup;
DROP TABLE IF EXISTS search_result;
DROP TABLE IF EXISTS screen_stack;
DROP TABLE IF EXISTS todo;
DROP TABLE IF EXISTS status;
DROP TABLE IF EXISTS password;
DROP TABLE IF EXISTS match_song;
DROP TABLE IF EXISTS match_song_backup;
DROP TABLE IF EXISTS match_song_song_match;
DROP TABLE IF EXISTS match_song_artist_match;
DROP TABLE IF EXISTS config_host_digital_format;

COMMIT;
