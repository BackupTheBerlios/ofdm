INCLUDES = -I$(top_srcdir)/soundcard

noinst_LIBRARIES = libnewqpsk.a

libnewqpsk_a_SOURCES = \
	modulator.c newqpsktx.c \
	demodulator.c newqpskrx.c \
	tbl.c filter.c \
	viterbi27.c viterbi37.c cblock.c crc.c vittab.c

noinst_HEADERS = \
	complex.h filter-i386.h filter.h \
	misc.h modemconfig.h newqpskrx.h newqpsktx.h tbl.h \
	viterbi.h viterbi27.h viterbi37.h cblock.h crc.h vittab.h

if CROSSCOMP
else

noinst_PROGRAMS = genfilt
genfilt_SOURCES = genfilt.c
# BUILT_SOURCES = filter-i386.h

filter-i386.h:	genfilt$(EXEEXT)
		./genfilt > filter-i386.h

endif
