#_ osx.mak

#FLGS=-g -DDEBUG
FLGS=-O
LDFLG=
#LDFLG=-s
CC=gcc

CFLAGS = -w $(FLGS) -DLINUX -DOSX -DXTERM

SRC= ed.h menu.h ansi.c basic.c bsdunix.c buffer.c capture.h \
	capture.c file.c fileio.c ibmpc.c keypress.c line.c \
	main.c memenu.c menu.c more.c mouse.c patchexe.c disprev.c \
	display.c random.c region.c search.c spawn.c tcap.c termio.c \
	vt52.c win32.c window.c word.c xterm.c screen.c

OBJ= ansi.o basic.o buffer.o display.o file.o fileio.o line.o main.o \
	random.o region.o search.o spawn.o xterm.o termio.o vt52.o \
	window.o word.o more.o mouse.o

SOURCE= ed.h $(SRC) osx.mak me.html disprev.c

meosx : $(OBJ) osx.mak
	cc $(CFLAGS) -c disprev.c
	cc $(LDFLG) $(FLGS) -o meosx $(OBJ) disprev.o

tags: $(SRC)
	ctags $(SRC)

me.zip : $(SOURCE)
	zip me $(SOURCE) meosx

keypress : keypress.c
	$(CC) $(CFLAGS) keypress.c -o keypress





