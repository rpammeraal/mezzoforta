SELECT
	a1.artist_id,
	a2.artist_id,
	a1.name,
	a2.name,
	r.record_id,
	r.title
FROM
	rock.artist a1
		JOIN rock.artist a2 USING(name)
		LEFT JOIN rock.record r ON
			a2.artist_id=r.artist_id
WHERE
	a1.artist_id!=a2.artist_id AND
	a1.artist_id<a2.artist_id
ORDER BY
	a1.name
;
