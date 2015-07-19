





CREATE TABLE index (
    song_id integer,
    record_id integer,
    artist_id integer NOT NULL,
    keyword character varying NOT NULL,
    value smallint
);



CREATE TABLE artist (
    artist_id integer NOT NULL,
    name character varying NOT NULL,
    sort_name character varying NOT NULL,
    www text,
    notes text,
    mbid character varying,
    CONSTRAINT artist_artist_id_check CHECK ((artist_id >= 0)),
    CONSTRAINT artist_name_check CHECK (((name)::text <> ''::text))
);



CREATE TABLE artist_rel (
    artist1_id integer NOT NULL,
    artist2_id integer NOT NULL
);



CREATE TABLE category (
    category_id integer NOT NULL,
    name character varying NOT NULL,
    parent_category_id integer,
    expand boolean NOT NULL,
    CONSTRAINT category_name_check CHECK (((name)::text <> ''::text))
);



CREATE TABLE category_index (
    keyword character varying NOT NULL,
    category_id integer NOT NULL
);



CREATE TABLE category_record (
    record_id integer NOT NULL,
    category_id integer NOT NULL,
    CONSTRAINT category_record_record_id_check CHECK ((record_id >= 0))
);



CREATE TABLE chart (
    chart_id integer NOT NULL,
    name character varying NOT NULL,
    release_date date NOT NULL,
    notes text,
    CONSTRAINT chart_chart_id_check CHECK ((chart_id >= 0)),
    CONSTRAINT chart_name_check CHECK (((name)::text <> ''::text))
);



CREATE TABLE chart_performance (
    chart_id integer NOT NULL,
    artist_id integer NOT NULL,
    song_id integer NOT NULL,
    chart_position integer NOT NULL,
    notes text,
    CONSTRAINT chart_performance_chart_position_check CHECK ((chart_position > 0))
);



CREATE TABLE collection (
    collection_id integer NOT NULL,
    name character varying NOT NULL,
    media character varying NOT NULL,
    notes text,
    CONSTRAINT collection_collection_id_check CHECK ((collection_id >= 0)),
    CONSTRAINT collection_media_check CHECK (((media)::text <> ''::text)),
    CONSTRAINT collection_name_check CHECK (((name)::text <> ''::text))
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
    notes text,
    CONSTRAINT collection_performance_c_position_check CHECK ((c_position >= 0)),
    CONSTRAINT collection_performance_path_check CHECK (((path)::text <> ''::text)),
    CONSTRAINT collection_performance_path_check1 CHECK (((path)::text <> ''::text))
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
    insert_order integer NOT NULL,
    CONSTRAINT online_performance_record_position_check CHECK ((record_position >= 0))
);



CREATE TABLE performance (
    song_id integer NOT NULL,
    artist_id integer NOT NULL,
    role_id smallint NOT NULL,
    year integer,
    notes text,
    CONSTRAINT performance_role_id_check CHECK (((role_id >= 0) AND (role_id <= 1))),
    CONSTRAINT performance_year_check CHECK (((year IS NULL) OR (year >= 1900)))
);



CREATE TABLE playlist (
    playlist_id integer NOT NULL,
    name character varying NOT NULL,
    notes text,
    created date NOT NULL,
    duration interval,
    updated date,
    play_mode integer NOT NULL,
    CONSTRAINT playlist_play_mode_check CHECK (((play_mode >= 0) AND (play_mode <= 2))),
    CONSTRAINT playlist_playlist_id_check CHECK ((playlist_id >= 0))
);



CREATE TABLE playlist_composite (
    playlist_id integer,
    playlist_position integer,
    "timestamp" date NOT NULL,
    notes text,
    playlist_playlist_id integer,
    playlist_chart_id integer,
    playlist_record_id integer,
    playlist_artist_id integer,
    CONSTRAINT playlist_composite_check CHECK ((playlist_playlist_id <> playlist_id)),
    CONSTRAINT playlist_composite_playlist_position_check CHECK ((playlist_position > 0))
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
    notes text,
    CONSTRAINT playlist_performance_playlist_position_check CHECK ((playlist_position > 0))
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
    notes text,
    CONSTRAINT record_media_check CHECK (((media)::text <> ''::text)),
    CONSTRAINT record_record_id_check CHECK ((record_id >= 0)),
    CONSTRAINT record_title_check CHECK (((title)::text <> ''::text))
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
    notes text,
    CONSTRAINT record_performance_check CHECK (((op_song_id = NULL::integer) OR (op_song_id = song_id))),
    CONSTRAINT record_performance_check1 CHECK (((op_artist_id = NULL::integer) OR (op_artist_id = artist_id))),
    CONSTRAINT record_performance_record_position_check CHECK ((record_position >= 0))
);



CREATE TABLE song (
    song_id integer NOT NULL,
    title character varying NOT NULL,
    notes text,
    CONSTRAINT song_song_id_check CHECK ((song_id >= 0)),
    CONSTRAINT song_title_check CHECK (((title)::text <> ''::text))
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



CREATE VIEW v_online_performance AS
    SELECT op.song_id, s.title AS song_title, sa.artist_id AS song_artist_id, sa.name AS song_artist, r.record_id, r.title AS record_title, ra.name AS record_artist, op.record_position, op.path, df.extension FROM (((((online_performance op JOIN artist sa ON ((op.artist_id = sa.artist_id))) JOIN song s ON ((op.song_id = s.song_id))) JOIN record r ON ((op.record_id = r.record_id))) JOIN artist ra ON ((r.artist_id = ra.artist_id))) JOIN public.digital_format df ON ((op.format_id = df.format_id)));



CREATE VIEW v_performance AS
    SELECT p.song_id, s.title, p.year, a.artist_id, a.name FROM performance p, artist a, song s WHERE (((p.role_id = 0) AND (p.artist_id = a.artist_id)) AND (p.song_id = s.song_id));



ALTER TABLE ONLY category
    ADD CONSTRAINT category_p PRIMARY KEY (category_id);



ALTER TABLE ONLY chart_performance
    ADD CONSTRAINT chart_perf_primary_key PRIMARY KEY (song_id, artist_id, chart_id, chart_position);



ALTER TABLE ONLY collection_performance
    ADD CONSTRAINT collection_perf_primary_key PRIMARY KEY (song_id, artist_id, collection_id, c_position);



ALTER TABLE ONLY artist
    ADD CONSTRAINT i_artist_id PRIMARY KEY (artist_id);



ALTER TABLE ONLY chart
    ADD CONSTRAINT i_chart_id PRIMARY KEY (chart_id);



ALTER TABLE ONLY collection
    ADD CONSTRAINT i_collection PRIMARY KEY (collection_id);



ALTER TABLE ONLY record
    ADD CONSTRAINT i_record_id PRIMARY KEY (record_id);



ALTER TABLE ONLY song
    ADD CONSTRAINT i_song_id PRIMARY KEY (song_id);



ALTER TABLE ONLY lyrics
    ADD CONSTRAINT lyrics_pkey PRIMARY KEY (song_id);



ALTER TABLE ONLY online_performance
    ADD CONSTRAINT online_perf_primary_key PRIMARY KEY (song_id, artist_id, record_id, record_position);



ALTER TABLE ONLY playlist_composite
    ADD CONSTRAINT performance_composite_unique_ai UNIQUE (playlist_id, playlist_artist_id);



ALTER TABLE ONLY playlist_composite
    ADD CONSTRAINT performance_composite_unique_ci UNIQUE (playlist_id, playlist_chart_id);



ALTER TABLE ONLY playlist_composite
    ADD CONSTRAINT performance_composite_unique_pi UNIQUE (playlist_id, playlist_playlist_id);



ALTER TABLE ONLY playlist_composite
    ADD CONSTRAINT performance_composite_unique_ri UNIQUE (playlist_id, playlist_record_id);



ALTER TABLE ONLY performance
    ADD CONSTRAINT performance_unique UNIQUE (song_id, artist_id);



ALTER TABLE ONLY playlist
    ADD CONSTRAINT playlist_i PRIMARY KEY (playlist_id);



ALTER TABLE ONLY record_performance
    ADD CONSTRAINT record_performance_pkey PRIMARY KEY (song_id, artist_id, record_id, record_position);



ALTER TABLE ONLY toplay
    ADD CONSTRAINT toplay_insert_order_key UNIQUE (insert_order);



ALTER TABLE ONLY toplay
    ADD CONSTRAINT toplay_play_order_key UNIQUE (play_order);



ALTER TABLE ONLY toplay
    ADD CONSTRAINT toplay_primary_key PRIMARY KEY (song_id, artist_id, record_id, record_position);



CREATE INDEX artist_rel_a1 ON artist_rel USING btree (artist1_id);



CREATE INDEX artist_rel_a2 ON artist_rel USING btree (artist2_id);



CREATE INDEX i_artist_name_first ON artist USING btree (public.firstchar(name) varchar_ops);



CREATE INDEX i_chart_name_first ON chart USING btree (public.firstchar(name) varchar_ops);



CREATE INDEX i_chart_performance ON chart_performance USING btree (song_id, artist_id);



CREATE INDEX i_chart_performance_a ON chart_performance USING btree (artist_id);



CREATE INDEX i_chart_performance_c ON chart_performance USING btree (chart_id);



CREATE INDEX i_chart_performance_s ON chart_performance USING btree (song_id);



CREATE INDEX i_collection_id ON collection USING btree (collection_id);



CREATE INDEX i_collection_name_first ON collection USING btree (public.firstchar(name) varchar_ops);



CREATE INDEX i_collection_performance ON collection_performance USING btree (collection_id);



CREATE INDEX i_collection_performance_sa ON collection_performance USING btree (song_id, artist_id);



CREATE INDEX i_index_a ON index USING btree (artist_id);



CREATE INDEX i_index_k ON index USING btree (keyword);



CREATE INDEX i_index_p ON index USING btree (song_id, artist_id);



CREATE INDEX i_index_r ON index USING btree (record_id, artist_id);



CREATE INDEX i_lyrics_song_id ON lyrics USING btree (song_id);



CREATE INDEX i_online_performance ON online_performance USING btree (song_id, artist_id);



CREATE INDEX i_online_performance_a ON online_performance USING btree (artist_id);



CREATE INDEX i_online_performance_a_lpd ON online_performance USING btree (artist_id, last_play_date);



CREATE UNIQUE INDEX i_online_performance_io ON online_performance USING btree (insert_order);



CREATE INDEX i_online_performance_lpd ON online_performance USING btree (last_play_date);



CREATE INDEX i_online_performance_p ON online_performance USING btree (path varchar_ops);



CREATE INDEX i_online_performance_po ON online_performance USING btree (play_order);



CREATE INDEX i_online_performance_ri ON online_performance USING btree (record_id);



CREATE INDEX i_online_performance_s ON online_performance USING btree (song_id);



CREATE INDEX i_performance_artist_id ON performance USING btree (artist_id);



CREATE INDEX i_performance_song_id ON performance USING btree (song_id);



CREATE INDEX i_performance_year ON performance USING btree (year);



CREATE INDEX i_playlist_performance_song_id ON playlist_performance USING btree (song_id);



CREATE INDEX i_record_artist_id ON record USING btree (artist_id);



CREATE INDEX i_record_genre ON record USING btree (genre varchar_ops);



CREATE INDEX i_record_performance_artist_id ON record_performance USING btree (artist_id);



CREATE INDEX i_record_performance_record_id ON record_performance USING btree (record_id);



CREATE INDEX i_record_title_first ON record USING btree (public.firstchar(title) varchar_ops);



CREATE INDEX i_song_title ON song USING btree (title varchar_ops);



CREATE INDEX i_to_be_indexed ON to_be_indexed USING btree (song_id, record_id, artist_id);



CREATE INDEX i_toplay_a ON toplay USING btree (artist_id);



CREATE INDEX i_toplay_lpd ON toplay USING btree (last_play_date);



CREATE INDEX i_toplay_op ON toplay USING btree (song_id, artist_id, record_id, record_position);



CREATE INDEX i_toplay_po ON toplay USING btree (play_order);



CREATE TRIGGER artist_trigger AFTER INSERT OR DELETE OR UPDATE ON artist FOR EACH ROW EXECUTE PROCEDURE artist_trigger();



CREATE TRIGGER performance_trigger AFTER INSERT OR DELETE OR UPDATE ON performance FOR EACH ROW EXECUTE PROCEDURE performance_trigger();



CREATE TRIGGER record_trigger AFTER INSERT OR DELETE OR UPDATE ON record FOR EACH ROW EXECUTE PROCEDURE record_trigger();



CREATE TRIGGER song_trigger AFTER INSERT OR DELETE OR UPDATE ON song FOR EACH ROW EXECUTE PROCEDURE song_trigger();



ALTER TABLE ONLY artist_rel
    ADD CONSTRAINT artist1_rel_aid FOREIGN KEY (artist1_id) REFERENCES artist(artist_id);



ALTER TABLE ONLY artist_rel
    ADD CONSTRAINT artist2_rel_aid FOREIGN KEY (artist2_id) REFERENCES artist(artist_id);



ALTER TABLE ONLY category_index
    ADD CONSTRAINT category_index_ci_ci FOREIGN KEY (category_id) REFERENCES category(category_id);



ALTER TABLE ONLY category
    ADD CONSTRAINT category_parent_category_id FOREIGN KEY (parent_category_id) REFERENCES category(category_id);



ALTER TABLE ONLY category_record
    ADD CONSTRAINT category_record_aid FOREIGN KEY (record_id) REFERENCES record(record_id);



ALTER TABLE ONLY category_record
    ADD CONSTRAINT category_record_ci_ci FOREIGN KEY (category_id) REFERENCES category(category_id);



ALTER TABLE ONLY chart_performance
    ADD CONSTRAINT chart_perf_cid FOREIGN KEY (chart_id) REFERENCES chart(chart_id);



ALTER TABLE ONLY chart_performance
    ADD CONSTRAINT chart_perf_performance FOREIGN KEY (song_id, artist_id) REFERENCES performance(song_id, artist_id);



ALTER TABLE ONLY collection_performance
    ADD CONSTRAINT collection_perf_dc_id FOREIGN KEY (collection_id) REFERENCES collection(collection_id);



ALTER TABLE ONLY collection_performance
    ADD CONSTRAINT collection_perf_performance FOREIGN KEY (song_id, artist_id) REFERENCES performance(song_id, artist_id);



ALTER TABLE ONLY online_performance
    ADD CONSTRAINT online_perf_fid FOREIGN KEY (format_id) REFERENCES public.digital_format(format_id);



ALTER TABLE ONLY online_performance
    ADD CONSTRAINT online_perf_record_performance FOREIGN KEY (song_id, artist_id, record_id, record_position) REFERENCES record_performance(song_id, artist_id, record_id, record_position) DEFERRABLE INITIALLY DEFERRED;



ALTER TABLE ONLY online_performance
    ADD CONSTRAINT online_performance_soid FOREIGN KEY (source_id) REFERENCES public.source(source_id);



ALTER TABLE ONLY playlist_composite
    ADD CONSTRAINT playlist_comp_artist_pid FOREIGN KEY (playlist_artist_id) REFERENCES artist(artist_id);



ALTER TABLE ONLY playlist_composite
    ADD CONSTRAINT playlist_comp_chart_pid FOREIGN KEY (playlist_chart_id) REFERENCES chart(chart_id);



ALTER TABLE ONLY playlist_composite
    ADD CONSTRAINT playlist_comp_pid FOREIGN KEY (playlist_id) REFERENCES playlist(playlist_id);



ALTER TABLE ONLY playlist_composite
    ADD CONSTRAINT playlist_comp_playlist_pid FOREIGN KEY (playlist_playlist_id) REFERENCES playlist(playlist_id);



ALTER TABLE ONLY playlist_composite
    ADD CONSTRAINT playlist_comp_record_pid FOREIGN KEY (playlist_record_id) REFERENCES record(record_id);



ALTER TABLE ONLY playlist_performance
    ADD CONSTRAINT playlist_perf_col_performance FOREIGN KEY (song_id, artist_id, collection_id, c_position) REFERENCES collection_performance(song_id, artist_id, collection_id, c_position) DEFERRABLE INITIALLY DEFERRED;



ALTER TABLE ONLY playlist_performance
    ADD CONSTRAINT playlist_perf_performance FOREIGN KEY (song_id, artist_id) REFERENCES performance(song_id, artist_id) DEFERRABLE INITIALLY DEFERRED;



ALTER TABLE ONLY playlist_performance
    ADD CONSTRAINT playlist_perf_pid FOREIGN KEY (playlist_id) REFERENCES playlist(playlist_id);



ALTER TABLE ONLY playlist_performance
    ADD CONSTRAINT playlist_perf_rec_performance FOREIGN KEY (song_id, artist_id, record_id, record_position) REFERENCES record_performance(song_id, artist_id, record_id, record_position) DEFERRABLE INITIALLY DEFERRED;



ALTER TABLE ONLY record
    ADD CONSTRAINT record_aid FOREIGN KEY (artist_id) REFERENCES artist(artist_id);



ALTER TABLE ONLY collection_performance
    ADD CONSTRAINT record_perf_fi FOREIGN KEY (format_id) REFERENCES public.digital_format(format_id);



ALTER TABLE ONLY record_performance
    ADD CONSTRAINT record_perf_online_performance FOREIGN KEY (op_song_id, op_artist_id, op_record_id, op_record_position) REFERENCES online_performance(song_id, artist_id, record_id, record_position) DEFERRABLE INITIALLY DEFERRED;



ALTER TABLE ONLY record_performance
    ADD CONSTRAINT record_perf_performance FOREIGN KEY (song_id, artist_id) REFERENCES performance(song_id, artist_id);



ALTER TABLE ONLY record_performance
    ADD CONSTRAINT record_perf_record_performance FOREIGN KEY (op_song_id, op_artist_id, op_record_id, op_record_position) REFERENCES record_performance(song_id, artist_id, record_id, record_position) DEFERRABLE INITIALLY DEFERRED;



ALTER TABLE ONLY record_performance
    ADD CONSTRAINT record_perf_rid FOREIGN KEY (record_id) REFERENCES record(record_id);



ALTER TABLE ONLY performance
    ADD CONSTRAINT song_inst_aid FOREIGN KEY (artist_id) REFERENCES artist(artist_id);



ALTER TABLE ONLY performance
    ADD CONSTRAINT song_inst_sid FOREIGN KEY (song_id) REFERENCES song(song_id);



ALTER TABLE ONLY lyrics
    ADD CONSTRAINT song_inst_sid FOREIGN KEY (song_id) REFERENCES song(song_id);



ALTER TABLE ONLY toplay
    ADD CONSTRAINT toplay_online_performance FOREIGN KEY (song_id, artist_id, record_id, record_position) REFERENCES online_performance(song_id, artist_id, record_id, record_position) DEFERRABLE INITIALLY DEFERRED;



