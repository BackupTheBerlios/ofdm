## Process this file with automake to produce Makefile.in

INCLUDES = @GTK_CFLAGS@ @XMLCFLAGS@ -I$(top_srcdir)/intl -I$(top_srcdir) -I$(top_srcdir)/matlib -I$(top_srcdir)/libmisc \
	-I$(top_srcdir)/soundcard -I$(top_srcdir)/directx/include -I$(top_srcdir)/directx/include/directx6 \
	-I$(top_srcdir)/configapp

man_MANS = soundmodemconfig.8

if WIN32
#MWINDOWS = -mwindows
MWINDOWS =
RCOBJ = configapp.res
else
MWINDOWS =
RCOBJ =
endif

MODEMLIBS = ../../afsk/libafsk.a ../../fsk/libfsk.a ../../pammodem/libpam.a ../../pskmodem/libpsk.a ../../newqpsk/libnewqpsk.a ../../p3dmodem/libp3d.a

bin_PROGRAMS = soundmodemconfig

if WIN32
configapp.res:	configapp.rc
		$(WINDRES) $< -O coff -o $@
endif

soundmodemconfig_SOURCES = \
	main.c app.c diag.c configapp.h \
	support.c support.h \
	interface.c interface.h \
	callbacks.h \
	spectrum.c spectrum.h \
	scope.c scope.h

soundmodemconfig_LDADD = ../../soundcard/liblog.a ../../soundcard/libaudioio.a ../../soundcard/libpttio.a \
			../../matlib/libmat.a ../../libmisc/libmisc.a $(MODEMLIBS) ../../soundcard/libcommon.a \
			@LIBTHREAD@ @GTK_LIBS@ @XMLLIBS@ @AUDIOFILE_LIBS@ $(RCOBJ)
soundmodemconfig_LDFLAGS = $(MWINDOWS)

EXTRA_DIST = configapp.rc soundmodemconfig.8
