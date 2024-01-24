SELECT
    t1.song_title,
    t2.song_title,
    t1.performer
FROM
    top2000.top2000all t1
        JOIN top2000.top2000all t2 ON
            t1.performer=t2.performer AND
            t1.song_title ILIKE '%' || t2.song_title || '%'
WHERE
    t1.song_title!=t2.song_title
;
