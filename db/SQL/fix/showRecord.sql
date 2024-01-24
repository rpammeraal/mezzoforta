SELECT
    r.record_id,
    r.title                             AS record_title,
    rp.record_performance_id,
    rp.record_position,
    s.title,
    op.online_performance_id,
    op.path

FROM
    rock.record r
        JOIN rock.record_performance rp USING(record_id)
        JOIN rock.performance p USING(performance_id)
        JOIN rock.song s USING(song_id)
        LEFT JOIN rock.online_performance op USING(record_performance_id)
WHERE
    r.record_id IN (691)
ORDER BY
    r.record_id,
    rp.record_position
;
