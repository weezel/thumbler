CC	 = clang
CFLAGS	+= -std=c99 -g -march=native -O0 -Wall -Wextra -pedantic -Wformat \
	   -Wshadow -fstack-protector-strong
LDFLAGS	+= -lgd -ljpeg -lpng
OS	 = $(shell uname -s)

ifeq ($(OS), Linux)
	LDFLAGS		+= -lbsd
	CFLAGS		+= -fsanitize=address -fsanitize=undefined
endif

ifeq ($(OS), OpenBSD)
	INCLUDES	+= -I /usr/local/include
	LDPATHS		+= -L /usr/local/lib -L /usr/X11R6/lib
endif

.PHONY: all thumbler clean

all: thumbler

thumbler:
	${CC} ${CFLAGS} ${INCLUDES} -o $@ thumbler.c ${LDPATHS} ${LDFLAGS}
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
	rm -rf *.o core *.core thumbler _thmb.jpg pics/*_thmb.*

