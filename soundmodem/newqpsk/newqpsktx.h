#ifndef _NEWQPSKTX_H
#define _NEWQPSKTX_H

#include "complex.h"
#include "modemconfig.h"

/* --------------------------------------------------------------------- */

/* this results in a maximum of 1280*3*8/30 = 1024 symbols */
#define	DataBufSize	2*1280  // 2048 symbols

struct txstate {
	struct modemchannel *chan;
	struct filter filt;
	unsigned int bps;
	unsigned int bufsize;
	unsigned int tunelen;
	unsigned int synclen;
	void (*txroutine) (void *);
	int statecntr;
	int txdone;
	float *txwindowfunc;
	complex tunevect[TuneCarriers];
	complex datavect[DataCarriers];
	unsigned txword[SymbolBits];
	complex txwin[WindowLen];
	complex fftbuf[WindowLen];
	unsigned cblock[CBlockLen+1];
	unsigned char databuf[DataBufSize];
	unsigned char msgbuf[DataBufSize*3*8+100];
	int datalen;
	int msglen;
	int feclevel;
	int fecrate;
	int reqfecrate;
	int tcnoresponse;
	int inlv;
};

/* --------------------------------------------------------------------- */

extern void init_newqpsktx(void *);
extern int newqpsktx(void *, complex *);

/* --------------------------------------------------------------------- */

#endif
