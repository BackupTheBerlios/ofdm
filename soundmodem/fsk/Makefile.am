INCLUDES = -I$(top_srcdir)/soundcard -I$(top_srcdir)/matlib -I.


noinst_LIBRARIES = libfsk.a
libfsk_a_SOURCES = modem.c modempsp.c psp.h

genpsptbl_SOURCES = genpsptbl.c
genpsptbl_LDADD = ../matlib/libmat.a

if CROSSCOMP

else

noinst_PROGRAMS = genpsptbl

$(srcdir)/psp.h:	genpsptbl$(EXEEXT)
			./genpsptbl > $@

endif
