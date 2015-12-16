CC	 = clang
CFLAGS	+= -ansi -g -Wall -Wextra -pedantic
CFLAGS	+= -I /usr/local/include
LDFLAGS	 = -L /usr/local/lib -L /usr/X11R6/lib -lgd -lpng -ljpeg -lz

.PHONY: all thumbler.o tiler clean

all: thumbler.o tiler
	${CC} ${CFLAGS} $^ -o thumbler thumbler.o utils.c ${LDFLAGS}
thumbler.o:
	${CC} ${CFLAGS} -c -o $@ thumbler.c
tiler:
	${CC} ${CFLAGS} -o $@ tiler.c
scanbuild:
	scan-build -analyze-headers -o result_html -v \
		-enable-checker debug.DumpCallGraph make
test:
	find . -type f \( -iname "*.jpg" -or -iname "*.jpeg" -or \
		-iname "*.png" -or -iname "*.gif" \) > piclist.txt
	./thumbler piclist.txt
rmthumbs:
	rm -rf pics/*_thmb.*
clean:
	rm -rf *.o *.core thumbler tiler _thmb.jpg pics/*_thmb.*

