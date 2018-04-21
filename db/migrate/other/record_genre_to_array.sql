BEGIN;
ALTER TABLE rock.record RENAME COLUMN genre TO genre_old;
ALTER TABLE rock.record ADD COLUMN genre VARCHAR[];

UPDATE rock.record
SET
    genre=string_to_array(genre_old,'|')
;
SELECT DISTINCT genre_old,genre FROM rock.record ORDER BY 1;

ALTER TABLE rock.record DROP COLUMN genre_old;
--  \d rock.record

COMMIT;

--  SELECT genre FROM rock.record WHERE 'Nederlands' = ANY(genre) ;
--  UPDATE rock.record SET genre=ARRAY_REPLACE(genre,'Nederlands','uitdemoddergetrokken');
--  SELECT DISTINCT INITCAP(UNNEST(genre)) FROM rock.record ORDER BY 1;
