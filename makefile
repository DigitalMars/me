#_ makefile
# Needs Digital Mars C compiler to build, available free from:
# www.digitalmars.com


target: men

all:
	make clean
	make -f win32.mak
	make clean
	make -f dos32.mak
	make clean
	make -f dos.mak
	make clean

men:
	make -f win32.mak

mex:
	make -f dos32.mak

medos:
	make -f dos.mak

clean:
	del *.obj
	del *.lnk
	del *.sym
	del *.def
#	del *.bak
#	del test.*
#	del *.exe
	del *.map
	del *.tmp
	del *.lst
	del *.exp
	del *.dbg

########################################

# and the dependencies:

SRC= capture.h ed.h menu.h \
	ansi.c basic.c bsdunix.c buffer.c \
	capture.c file.c fileio.c ibmpc.c keypress.c line.c \
	main.c memenu.c menu.c more.c mouse.c patchexe.c disprev.c \
	display.c random.c region.c search.c spawn.c tcap.c termio.c \
	vt52.c win32.c window.c word.c xterm.c screen.c


MAK= linux.mak win32.mak dos.mak dos32.mak makefile \
	osx.mak linuxtcap.mak freebsd.mak

DOC= me.html
EXE= melinux men.exe mex.exe medos.exe

######################################

tolf:
	tolf $(SRC) $(MAK) $(DOC)

zip : tolf makefile
	del me.zip
	zip32 me $(SRC) $(MAK) $(DOC)

git: tolf makefile
	\putty\pscp -i c:\.ssh\colossus.ppk $(SRC) $(MAK) $(DOC) walter@mercury:dm/me

