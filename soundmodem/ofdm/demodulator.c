/*****************************************************************************/

/*
 *      demodulator.c  --  NEWQPSK demodulator.
 *
 *      Copyright (C) 2000  Tomi Manninen (tomi.manninen@hut.fi)
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/*****************************************************************************/

#define _GNU_SOURCE
#define _REENTRANT

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "modem.h"
#include "complex.h"
#include "modemconfig.h"
#include "filter.h"
#include "newqpskrx.h"
#include "tbl.h"

#include <stdlib.h>
#include <string.h>

/* --------------------------------------------------------------------- */

static const struct modemparams demodparams[] = {
	{ "bps", "Bits/s", "Bits per second", "2500", MODEMPAR_NUMERIC, { n: { 1000, 5000, 100, 500 } } },
	{ "mintune", "Tune length", "Minimum tune preamble length", "16", MODEMPAR_NUMERIC, { n: { 0, 32, 1, 1 } } },
	{ "minsync", "Sync length", "Minimum sync preamble length", "16", MODEMPAR_NUMERIC, { n: { 8, 32, 1, 1 } } },
	{ NULL }
};

#define SAMPLERATE(x)	((float)(x)*SymbolLen/DataCarriers/SymbolBits)

static void *demodconfig(struct modemchannel *chan, unsigned int *samplerate, const char *params[])
{
	struct rxstate *s;

	if ((s = calloc(1, sizeof(struct rxstate))) == NULL)
		logprintf(MLOG_FATAL, "out of memory");
	s->chan = chan;
	if (params[0]) {
		s->bps = strtoul(params[0], NULL, 0);
		if (s->bps < 1000)
			s->bps = 1000;
		if (s->bps > 5000)
			s->bps = 5000;
	} else
		s->bps = 2500;
	if (params[1]) {
		s->mintune = strtoul(params[1], NULL, 0);
		if (s->mintune < 0)
			s->mintune = 0;
		if (s->mintune > 32)
			s->mintune = 32;
	} else
		s->mintune = 16;
	if (params[2]) {
		s->minsync = strtoul(params[2], NULL, 0);
		if (s->minsync < 8)
			s->minsync = 8;
		if (s->minsync > 32)
			s->minsync = 32;
	} else
		s->mintune = 16;
	*samplerate = (int) (3.0 * SAMPLERATE(s->bps) + 0.5);
	return s;
}

static void demodinit(void *state, unsigned int samplerate, unsigned int *bitrate)
{
	struct rxstate *s = (struct rxstate *)state;
	float rate, f1, f2;

	s->srate = SAMPLERATE(s->bps);
	rate = SAMPLERATE(s->bps) / samplerate;
	f1 = 0.1 * rate;
	f2 = 0.9 * rate;
	init_tbl();
	init_filter(&s->filt, rate, f1, f2);
	init_newqpskrx(state);
	*bitrate = s->bps;
}

static complex getsample(struct rxstate *s)
{
	int16_t samples[SymbolLen];
	complex csamples[SymbolLen];
	int i;

	if (s->bufptr >= s->buflen) {
		audioread(s->chan, samples, SymbolLen, s->rxphase);
		s->rxphase = (s->rxphase + SymbolLen) & 0xffff;

		for (i = 0; i < SymbolLen; i++) {
			csamples[i].re = samples[i] * (1.0 / 32768.0);
			csamples[i].im = csamples[i].re;
		}

		s->buflen = filter(&s->filt, csamples, s->rxbuf);
		s->bufptr = 0;
	}
	return s->rxbuf[s->bufptr++];
}

static void demoddemodulate(void *state)
{
	struct rxstate *s = (struct rxstate *)state;
	complex buf[SymbolLen / 2];
	int i;

	s->rxphase = audiocurtime(s->chan);
	for (;;) {
		for (i = 0; i < s->skip; i++)
			getsample(s);
		s->skip = 0;

		for (i = 0; i < SymbolLen / 2; i++)
			buf[i] = getsample(s);

		newqpskrx(state, buf);
	}
}

/* --------------------------------------------------------------------- */

struct demodulator ofdmdemodulator = {
        NULL,
        "ofdm",
        demodparams,
        demodconfig,
        demodinit,
        demoddemodulate,
        free
};

/* --------------------------------------------------------------------- */
