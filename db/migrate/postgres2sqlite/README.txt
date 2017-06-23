This script, given a source database and schema, will:
-	create a copy of this database (to deal with sequences) in a tmp database
-	backups this tmp database while adjusting the script for sqlite
-	populates a brand new sqlite database.

