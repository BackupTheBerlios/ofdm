#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "modem.h"

#include "modemconfig.h"
#include "complex.h"
#include "viterbi.h"
#include "filter.h"
#include "newqpsktx.h"
#include "tbl.h"
#include "misc.h"
#include "cblock.h"
#include "turbo.h"

/* --------------------------------------------------------------------- */

static void txtune(void *);
static void txsync(void *);
static void txpredata(void *);
static void txdata(void *);
static void txflush(void *);

/* --------------------------------------------------------------------- */

void init_newqpsktx(void *state)
{
	struct txstate *s = (struct txstate *)state;
	int i;

	/* switch to tune mode */
	s->txroutine = txtune;
	s->statecntr = 0;
	s->txwindowfunc = ToneWindowOut;
	s->msglen = 0;

	/* copy initial tune vectors */
	for (i = 0; i < TuneCarriers; i++) {
		s->tunevect[i].re = TuneIniVectI[i];
		s->tunevect[i].im = TuneIniVectQ[i];
	}
}

/* --------------------------------------------------------------------- */

static void encodenone(unsigned char *out, unsigned char *in, int len)
{
	int i;

	while (len-- > 0) {
		for (i = 7; i >= 0; i--)
			*out++ = (*in >> i) & 1;
		in++;
	}
}

static int puncturepattern[14] = {1, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1};

static int puncture(unsigned char *buf, int len)
{
	unsigned char *tmp = alloca(len);
	unsigned char *p = buf;
	int i = 0;
	int j = 0;

	while (len-- > 0) {
		if (puncturepattern[i++])
			tmp[j++] = *p;

		i %= 14;
		p++;
	}

	memcpy(buf, tmp, j);

	return j;
}

static void interleave(unsigned char *buf, int len)
{
	unsigned char *tmp = alloca(len);
	int i = 0;
	int j = 0;

	memcpy(tmp, buf, len);

	while (i < len) {
		while (rbits16(j) >= len)
			j++;

		buf[rbits16(j++)] = tmp[i++];
	}
}

static void txassemble(void *state)
{
	struct txstate *s = (struct txstate *)state;
	int i;
	int j;

	/* encoder tail */
	s->databuf[s->datalen++] = 0;

	/* error correction coding */
	switch (s->feclevel) {
	default:
	case 0:
		encodenone(s->msgbuf, s->databuf, s->datalen);
		s->msglen = s->datalen * 8;
		break;
	case 1:
		encode27(s->msgbuf, s->databuf, s->datalen);
		s->msglen = puncture(s->msgbuf, s->datalen * 2 * 8);
		break;
	case 2:
		encode27(s->msgbuf, s->databuf, s->datalen);
		s->msglen = s->datalen * 2 * 8;
		break;
	case 3:
		encode37(s->msgbuf, s->databuf, s->datalen);
		s->msglen = s->datalen * 3 * 8;
		break;
	case 4:
		/* pad up to multiple of TC packets */
		while (s->datalen % (1024/8))
			s->databuf[s->datalen++] = 0;
	 	for (i=0, j=0; i<s->datalen/(1024/8); i++)
		   j += turboencode (s->databuf+i*(1024/8), 
				   s->msgbuf+j, 1024, s->fecrate, 1);
		s->msglen = j;
	}

	/* pad up to full symbol length */
	while ((s->msglen % (DataCarriers * SymbolBits)) != 0)
		s->msgbuf[s->msglen++] = 0;

	/* interleave */
	interleave(s->msgbuf, s->msglen);

	/* make a control block */
	enc_cblock(s->cblock, s->msglen / DataCarriers / SymbolBits, s->feclevel, s->fecrate);
}

static unsigned getword(void *state)
{
	struct txstate *s = (struct txstate *)state;
	unsigned i, word = 0;

	for (i = 0; i < DataCarriers; i++)
		word |= s->msgbuf[s->statecntr++] << i;

	return word;
}

/* --------------------------------------------------------------------- */

static void fft(complex * in, complex * out)
{
	int i, j, k;
	int s, sep, width, top, bot;
	float tr, ti;

	/* order the samples in bit reverse order */
	for (i = 0; i < WindowLen; i++) {
		j = rbits8(i) >> (8 - WindowLenLog);
		out[j] = in[i];
	}

	/* in-place FFT */
	sep = 1;
	for (s = 1; s <= WindowLenLog; s++) {
		width = sep;	/* butterfly width =  2^(s-1) */
		sep <<= 1;	/* butterfly separation = 2^s */

		for (j = 0; j < width; j++) {

			k = WindowLen * j / sep;

			for (top = j; top < WindowLen; top += sep) {
				bot = top + width;

				tr = out[bot].re * CosTable[k] + out[bot].im * SinTable[k];
				ti = out[bot].im * CosTable[k] - out[bot].re * SinTable[k];

				out[bot].re = out[top].re - tr;
				out[bot].im = out[top].im - ti;

				out[top].re = out[top].re + tr;
				out[top].im = out[top].im + ti;
			}
		}
	}
}

/* --------------------------------------------------------------------- */

int newqpsktx(void *state, complex *samples)
{
	struct txstate *s = (struct txstate *)state;
	complex tmp[WindowLen];
	int i;

	/* assemble the packet if not done so yet */
	if (s->msglen == 0)
		txassemble(state);

	/* clear all FFT bins */
	for (i = 0; i < WindowLen; i++) {
		s->fftbuf[i].re = 0.0;
		s->fftbuf[i].im = 0.0;
	}

	/* process the data */
	s->txroutine(state);

	/* fft */
	fft(s->fftbuf, tmp);

	/* overlap and apply the window function */
	i = 0;
	while (i < WindowLen - SymbolLen) {
		s->txwin[i] = s->txwin[i + SymbolLen];
		s->txwin[i].re += tmp[i].re * s->txwindowfunc[i];
		s->txwin[i].im += tmp[i].im * s->txwindowfunc[i];
		i++;
	}
	while (i < WindowLen) {
		s->txwin[i].re = tmp[i].re * s->txwindowfunc[i];
		s->txwin[i].im = tmp[i].im * s->txwindowfunc[i];
		i++;
	}

	/* output filter and interpolation */
	i = filter(&s->filt, s->txwin, samples);

	return i;
}

/* --------------------------------------------------------------------- */

/*
 * Send tune carriers (four continuous carriers spaced at 600Hz).
 */
static void txtune(void *state)
{
	struct txstate *s = (struct txstate *)state;
	int i, j;

	j = FirstTuneCarr;
	for (i = 0; i < TuneCarriers; i++) {
		/* flip odd carriers -> continuous phase */
		if (j % 2) {
			s->fftbuf[j].re = -s->tunevect[i].re;
			s->fftbuf[j].im = -s->tunevect[i].im;
		} else {
			s->fftbuf[j].re = s->tunevect[i].re;
			s->fftbuf[j].im = s->tunevect[i].im;
		}
		s->tunevect[i] = s->fftbuf[j];
		j += TuneCarrSepar;
	}

	if (s->statecntr++ < s->tunelen)
		return;

	/* switch to sync mode */
	s->txroutine = txsync;
	s->statecntr = 0;
	s->txwindowfunc = DataWindowOut;
}

/* --------------------------------------------------------------------- */

/*
 * Send sync sequence (tune carriers turned by 180 degrees every symbol).
 */
static void txsync(void *state)
{
	struct txstate *s = (struct txstate *)state;
	int i, j;

	j = FirstTuneCarr;
	for (i = 0; i < TuneCarriers; i++) {
		/* flip even carriers -> inverted phase */
		if (j % 2) {
			s->fftbuf[j].re = s->tunevect[i].re;
			s->fftbuf[j].im = s->tunevect[i].im;
		} else {
			s->fftbuf[j].re = -s->tunevect[i].re;
			s->fftbuf[j].im = -s->tunevect[i].im;
		}
		s->tunevect[i] = s->fftbuf[j];
		j += TuneCarrSepar;
	}

	if (s->statecntr++ < s->synclen)
		return;

	/* switch to pre data mode */
	s->txroutine = txpredata;
	s->statecntr = 0;

	/* copy initial data vectors */
	for (i = 0; i < DataCarriers; i++) {
		s->datavect[i].re = DataIniVectI[i];
		s->datavect[i].im = DataIniVectQ[i];
	}
}

/* --------------------------------------------------------------------- */

static void encodeword(void *state, int symbolbits)
{
	struct txstate *s = (struct txstate *)state;
	unsigned i, j, k, data;
	complex z;
	float phi;

	j = FirstDataCarr;
	for (i = 0; i < DataCarriers; i++) {
		/* collect databits for this symbol */
		for (data = 0, k = 0; k < symbolbits; k++)
			if (s->txword[k] & (1 << i))
				data |= (1 << k);

		/* gray encode */
		data ^= data >> 1;

		/* flip the top bit */
		data ^= 1 << (symbolbits - 1);

		/* modulate */
		phi = data * 2.0 * M_PI / (1 << symbolbits);
		z.re = cos(phi);
		z.im = sin(phi);
		s->fftbuf[j] = cmul(s->datavect[i], z);

		/* turn odd carriers by 180 degrees */
		if (j % 2) {
			s->fftbuf[j].re = -s->fftbuf[j].re;
			s->fftbuf[j].im = -s->fftbuf[j].im;
		}

		s->datavect[i] = s->fftbuf[j];
		j += DataCarrSepar;
	}
}

static void txpredata(void *state)
{
	struct txstate *s = (struct txstate *)state;

	s->txword[0] = s->cblock[s->statecntr];

	/* BPSK */
	encodeword(state, 1);

	if (s->statecntr++ < CBlockLen)
		return;

	/* switch to data mode */
	s->txroutine = txdata;
	s->statecntr = 0;
}

static void txdata(void *state)
{
	struct txstate *s = (struct txstate *)state;
	int i;

	for (i = 0; i < SymbolBits; i++)
		s->txword[i] = getword(state);

	encodeword(state, SymbolBits);

	if (s->statecntr < s->msglen)
		return;

	/* switch to post data mode */
	s->txroutine = txflush;
	s->statecntr = 0;
}

static void txflush(void *state)
{
	struct txstate *s = (struct txstate *)state;

	if (s->statecntr++ < 3)
		return;

	/* get ready for the next transmission */
	init_newqpsktx(state);

	/* signal the main routine we're done */
	s->txdone = 1;
}

/* --------------------------------------------------------------------- */
