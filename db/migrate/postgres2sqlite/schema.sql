







CREATE TABLE artist (
    artist_id integer NOT NULL,
    name character varying NOT NULL,
    sort_name character varying NOT NULL,
    www text,
    notes text,
    mbid character varying,
    soundex character varying
);



CREATE TABLE artist_rel (
    artist1_id integer NOT NULL,
    artist2_id integer NOT NULL
);



CREATE TABLE category (
    category_id integer NOT NULL,
    name character varying NOT NULL,
    parent_category_id integer,
    expand boolean NOT NULL
);



CREATE TABLE category_index (
    keyword character varying NOT NULL,
    category_id integer NOT NULL
);



CREATE TABLE category_record (
    record_id integer NOT NULL,
    category_id integer NOT NULL
);



CREATE TABLE chart (
    chart_id integer NOT NULL,
    name character varying NOT NULL,
    release_date date NOT NULL,
    notes text
);



CREATE TABLE chart_performance (
    chart_id integer NOT NULL,
    artist_id integer NOT NULL,
    song_id integer NOT NULL,
    chart_position integer NOT NULL,
    notes text
);



CREATE TABLE collection (
    collection_id integer NOT NULL,
    name character varying NOT NULL,
    media character varying NOT NULL,
    notes text
);



CREATE TABLE collection_performance (
    collection_id integer NOT NULL,
    artist_id integer NOT NULL,
    song_id integer NOT NULL,
    c_position smallint NOT NULL,
    path character varying NOT NULL,
    file_name character varying NOT NULL,
    format_id integer NOT NULL,
    duration time without time zone NOT NULL,
    bit_rate integer,
    notes text
);



CREATE TABLE lyrics (
    song_id integer NOT NULL,
    lyrics text
);



CREATE TABLE online_performance (
    song_id integer NOT NULL,
    artist_id integer NOT NULL,
    record_id integer NOT NULL,
    record_position smallint NOT NULL,
    format_id integer NOT NULL,
    path character varying NOT NULL,
    source_id integer NOT NULL,
    last_play_date timestamp without time zone,
    play_order integer,
    insert_order integer NOT NULL
);



CREATE TABLE performance (
    song_id integer NOT NULL,
    artist_id integer NOT NULL,
    role_id smallint NOT NULL,
    year integer,
    notes text
);



CREATE TABLE playlist (
    playlist_id integer NOT NULL,
    name character varying NOT NULL,
    notes text,
    created date NOT NULL,
    duration interval,
    updated date,
    play_mode integer NOT NULL
);



CREATE TABLE playlist_composite (
    playlist_id integer,
    playlist_position integer,
    "timestamp" date NOT NULL,
    notes text,
    playlist_playlist_id integer,
    playlist_chart_id integer,
    playlist_record_id integer,
    playlist_artist_id integer
);



CREATE TABLE playlist_performance (
    playlist_id integer,
    playlist_position integer,
    song_id integer NOT NULL,
    artist_id integer NOT NULL,
    record_id integer,
    record_position smallint,
    collection_id integer,
    c_position smallint,
    "timestamp" date NOT NULL,
    notes text
);



CREATE TABLE radio_stat (
    random_number integer,
    total integer
);



CREATE TABLE record (
    record_id integer NOT NULL,
    artist_id integer NOT NULL,
    title character varying NOT NULL,
    media character varying(10) NOT NULL,
    year integer,
    genre character varying,
    cddb_id character(8),
    cddb_category character varying(20),
    notes text
);



CREATE TABLE record_performance (
    song_id integer NOT NULL,
    artist_id integer NOT NULL,
    record_id integer NOT NULL,
    record_position smallint NOT NULL,
    op_song_id integer,
    op_artist_id integer,
    op_record_id integer,
    op_record_position smallint,
    duration time without time zone NOT NULL,
    notes text
);



CREATE TABLE song (
    song_id integer NOT NULL,
    title character varying NOT NULL,
    notes text,
	soundex character varying
);



CREATE TABLE to_be_indexed (
    song_id integer,
    record_id integer,
    artist_id integer
);



CREATE TABLE toplay (
    song_id integer NOT NULL,
    artist_id integer NOT NULL,
    record_id integer NOT NULL,
    record_position smallint NOT NULL,
    insert_order integer NOT NULL,
    play_order integer,
    last_play_date timestamp without time zone
);





CREATE VIEW v_performance AS
    SELECT p.song_id, s.title, p.year, a.artist_id, a.name FROM performance p, artist a, song s WHERE (((p.role_id = 0) AND (p.artist_id = a.artist_id)) AND (p.song_id = s.song_id));





CREATE INDEX artist_rel_a1 ON artist_rel  (artist1_id);



CREATE INDEX artist_rel_a2 ON artist_rel  (artist2_id);









CREATE INDEX i_chart_performance ON chart_performance  (song_id, artist_id);



CREATE INDEX i_chart_performance_a ON chart_performance  (artist_id);



CREATE INDEX i_chart_performance_c ON chart_performance  (chart_id);



CREATE INDEX i_chart_performance_s ON chart_performance  (song_id);



CREATE INDEX i_collection_id ON collection  (collection_id);






CREATE INDEX i_collection_performance ON collection_performance  (collection_id);



CREATE INDEX i_collection_performance_sa ON collection_performance  (song_id, artist_id);





CREATE INDEX i_lyrics_song_id ON lyrics  (song_id);



CREATE INDEX i_online_performance ON online_performance  (song_id, artist_id);



CREATE INDEX i_online_performance_a ON online_performance  (artist_id);



CREATE INDEX i_online_performance_a_lpd ON online_performance  (artist_id, last_play_date);



CREATE UNIQUE INDEX i_online_performance_io ON online_performance  (insert_order);



CREATE INDEX i_online_performance_lpd ON online_performance  (last_play_date);



CREATE INDEX i_online_performance_p ON online_performance  (path );



CREATE INDEX i_online_performance_po ON online_performance  (play_order);



CREATE INDEX i_online_performance_ri ON online_performance  (record_id);



CREATE INDEX i_online_performance_s ON online_performance  (song_id);



CREATE INDEX i_performance_artist_id ON performance  (artist_id);



CREATE INDEX i_performance_song_id ON performance  (song_id);



CREATE INDEX i_performance_year ON performance  (year);



CREATE INDEX i_playlist_performance_song_id ON playlist_performance  (song_id);



CREATE INDEX i_record_artist_id ON record  (artist_id);



CREATE INDEX i_record_genre ON record  (genre );



CREATE INDEX i_record_performance_artist_id ON record_performance  (artist_id);



CREATE INDEX i_record_performance_record_id ON record_performance  (record_id);





CREATE INDEX i_song_title ON song  (title );



CREATE INDEX i_to_be_indexed ON to_be_indexed  (song_id, record_id, artist_id);



CREATE INDEX i_toplay_a ON toplay  (artist_id);



CREATE INDEX i_toplay_lpd ON toplay  (last_play_date);



CREATE INDEX i_toplay_op ON toplay  (song_id, artist_id, record_id, record_position);



CREATE INDEX i_toplay_po ON toplay  (play_order);
