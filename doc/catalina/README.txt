PREREQUISITE:
************
brew install cmake xz

LIBS:
****
-	libogg
URL: https://xiph.org/downloads/
FILE: libogg-1.3.4.zip
./configure --disable-shared --enable-static
make LDFLAGS=-lm SHARED=0 CC='gcc -static'

-	libvorbis
URL: https://xiph.org/downloads/
FILE: libvorbis-1.3.7.zip
Add to: /usr/local/include/ogg/os_types.h
#include <stdint.h>
as the 1st definition after comments
./configure --disable-shared --enable-static
make LDFLAGS=-lm SHARED=0 CC='gcc -static'

-	lame
URL: https://sourceforge.net/projects/lame/
FILE: lame-3.100.tar.gz
./configure --disable-shared --enable-static
make LDFLAGS=-lm SHARED=0 CC='gcc -static'

-	libflac
URL: https://xiph.org/downloads/
FILE: flac-1.3.2.tar.xz
./configure --disable-shared --enable-static
make LDFLAGS=-lm SHARED=0 CC='gcc -static'

-	vorbistools
URL: https://xiph.org/downloads/
FILE: vorbis-tools-1.4.0.zip
./configure --disable-shared --enable-static
make LDFLAGS=-lm SHARED=0 CC='gcc -static'

-	libsndfile
URL: http://www.mega-nerd.com/libsndfile/#Download
FILE: libsndfile-1.0.28.tar.gz
./configure --disable-shared --enable-static
make LDFLAGS=-lm SHARED=0 CC='gcc -static'

-	portaudio
URL: http://www.portaudio.com/download.html
FILE: pa_stable_v190600_20161030.tgz
--- for now installed by brew
---	renamed the dylib files in /usr/local/lib
brew install portaudio

-	libmad
URL: git@github.com:markjeee/libmad.git
FILE: n/a
./configure --disable-shared --enable-static
Change Makefile:
globe:libmad root# diff Makefile.org  Makefile
129c129
< CFLAGS = -Wall -march=i486 -g -O -fforce-mem -fforce-addr -fthread-jumps -fcse-follow-jumps -fcse-skip-blocks -fexpensive-optimizations -fregmove -fschedule-insns2 -fstrength-reduce
---
> CFLAGS = -Wall -g -O -fforce-addr -fexpensive-optimizations -fschedule-insns2 -fstrength-reduce

-	libid3tag
brew install libid3tag

-	taglib
brew install taglib

DEPLOYMENT NOTES:
****************
-	libtag:
	installs framework at: /Library/Frameworks/tag.framework
	may need to copy fo Frameworks directory of app.
