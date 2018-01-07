PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE config_host 
( 
config_host_id  INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, 
hostname        VARCHAR NOT NULL, 
local_data_path INTEGER NOT NULL 
);
INSERT INTO "config_host" VALUES(0,'phobos','E:/songbase/files/xmas');
CREATE TABLE configuration 
( 
keyword varchar NOT NULL, 
value varchar NOT NULL, 
CONSTRAINT pk_config_host PRIMARY KEY(keyword,value) 
);
INSERT INTO "configuration" VALUES('default_schema','rock');
INSERT INTO "configuration" VALUES('performer_album_directory_structure_flag','1');
INSERT INTO "configuration" VALUES('run_import_on_startup_flag','0');
INSERT INTO "configuration" VALUES('unknown_album_id','0');
INSERT INTO "configuration" VALUES('various_performer_id','0');
INSERT INTO "configuration" VALUES('version','20170101');
CREATE TABLE genre 
( 
genrename varchar NOT NULL 
);
CREATE TABLE article 
( 
word varchar NOT NULL, 
CONSTRAINT pk_article PRIMARY KEY(word) 
);
INSERT INTO "article" VALUES('the');
INSERT INTO "article" VALUES('de');
INSERT INTO "article" VALUES('het');
INSERT INTO "article" VALUES('een');
INSERT INTO "article" VALUES('el');
INSERT INTO "article" VALUES('la');
INSERT INTO "article" VALUES('der');
INSERT INTO "article" VALUES('die');
INSERT INTO "article" VALUES('das');
CREATE TABLE greatest_hits_record 
( 
greatest_hits_record_id INTEGER PRIMARY KEY NOT NULL, 
title                   VARCHAR NOT NULL 
);
INSERT INTO "greatest_hits_record" VALUES(0,'Greatest Hits');
INSERT INTO "greatest_hits_record" VALUES(1,'The Best Of');
INSERT INTO "greatest_hits_record" VALUES(2,'Best Of');
INSERT INTO "greatest_hits_record" VALUES(3,'The Very Best Of');
INSERT INTO "greatest_hits_record" VALUES(4,'Het Beste Van');
INSERT INTO "greatest_hits_record" VALUES(5,'The Ultimate Collection');
INSERT INTO "greatest_hits_record" VALUES(6,'Ultimate Collection');
INSERT INTO "greatest_hits_record" VALUES(7,'Gold');
INSERT INTO "greatest_hits_record" VALUES(8,'Unplugged');
INSERT INTO "greatest_hits_record" VALUES(9,'Collection');
INSERT INTO "greatest_hits_record" VALUES(10,'The Singles');
DELETE FROM sqlite_sequence;
INSERT INTO "sqlite_sequence" VALUES('config_host',0);
INSERT INTO "sqlite_sequence" VALUES('artist',0);
INSERT INTO "sqlite_sequence" VALUES('record',104);
COMMIT;
PRAGMA foreign_keys=ON;
