CC	 = clang
CFLAGS	+= -ansi -g -Wall -Wextra -pedantic
CFLAGS	+= -Wsign-compare -Wdeclaration-after-statement -Wshadow
CFLAGS	+= -I /usr/local/include
LDFLAGS	 = -L /usr/local/lib -L /usr/X11R6/lib -lgd -lpng -ljpeg -lz

.PHONY: all clean

all: thumbler.o
	${CC} ${CFLAGS} $^ -o thumbler thumbler.c ${LDFLAGS}
thumbler.o:
	${CC} ${CFLAGS} -c -o $@ thumbler.c
scanbuild:
	scan-build -analyze-headers -o result_html -v -enable-checker debug.DumpCallGraph make
clean:
	rm -rf *.o *.core thumbler _thmb.jpg pics/*_thmb.jpg

