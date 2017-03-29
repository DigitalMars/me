#_ freebsd.mak

#FLGS=-g -DDEBUG
FLGS=-O
#LDFLG=
LDFLG=-s
CC=cc

CFLAGS = -w $(FLGS) -DLINUX -DXTERM

SRC= ansi.c basic.c buffer.c display.c file.c fileio.c line.c main.c \
	random.c region.c search.c spawn.c xterm.c termio.c vt52.c \
	window.c word.c more.c mouse.c keypress.c screen.c url.c browse.c

OBJ= ansi.o basic.o buffer.o display.o file.o fileio.o line.o main.o \
	random.o region.o search.o spawn.o xterm.o termio.o vt52.o \
	window.o word.o more.o mouse.o url.o browse.o

SOURCE= ed.h $(SRC) freebsd.mak me.html disprev.c

mefreebsd : $(OBJ) freebsd.mak
	cc $(CFLAGS) -c disprev.c
	cc $(LDFLG) $(FLGS) -o mefreebsd $(OBJ) disprev.o

tags: $(SRC)
	ctags $(SRC)

me.zip : $(SOURCE)
	zip me $(SOURCE)

keypress : keypress.c
	$(CC) $(CFLAGS) keypress.c -o keypress





