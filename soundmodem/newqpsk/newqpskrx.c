#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "modem.h"

#include "complex.h"
#include "modemconfig.h"
#include "filter.h"
#include "newqpskrx.h"
#include "newqpsktx.h"
#include "tbl.h"
#include "misc.h"
#include "cblock.h"
#include "viterbi.h"
#include "turbo.h"
#include "soundio.h"
#include "fec.h"
#include "crc.h"

/* --------------------------------------------------------------------- */

static void rxtune(void *);
static void rxsync(void *);
static void rxdata(void *);

/* --------------------------------------------------------------------- */

void init_newqpskrx(void *state)
{
	struct rxstate *s = (struct rxstate *)state;
	int i;

	/* clear dcd */
	pktsetdcd(s->chan, 0);
	if (s->mintune != 0) {
		/* switch to waiting tune mode */
		s->rxroutine = rxtune;
		s->rxwindowfunc = ToneWindowInp;
		s->carrfreq = 0.0;
		s->acceptance = 0;
		for (i = 0; i < TuneCarriers; i++) {
			s->tunepower[i] = 0.0;
			s->tunephase[i] = 0.0;
			s->tunecorr[i].re = 0.0;
			s->tunecorr[i].im = 0.0;
		}
	} else {
		/* switch to waiting sync mode */
		s->rxroutine = rxsync;
		s->rxwindowfunc = DataWindowInp;
		s->atsymbol = 1;
		s->acceptance = 0;
		for (i = 0; i < TuneCarriers; i++) {
			s->power_at[i] = s->tunepower[i];
			s->corr1_at[i].re = s->tunepower[i];
			s->corr1_at[i].im = 0.0;
			s->corr2_at[i].re = s->tunepower[i];
			s->corr2_at[i].im = 0.0;

			s->power_inter[i] = s->tunepower[i];
			s->corr1_inter[i].re = s->tunepower[i];
			s->corr1_inter[i].im = 0.0;
			s->corr2_inter[i].re = s->tunepower[i];
			s->corr2_inter[i].im = 0.0;
		}
	}
}

/* --------------------------------------------------------------------- */

static void decodenone(unsigned char *out, unsigned char *in, int len)
{
	int i;

	len /= 8;

	while (len-- > 0) {
		*out = 0;
		for (i = 0; i < 8; i++)
			*out = (*out << 1) | (*in++ > 127);
		out++;
	}
}

static int puncturepattern[14] = {1, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1};

static int unpuncture(unsigned char *buf, int len)
{
	unsigned char tmp[DataBufSize * 3 * 8], *p;
	int i, j;

	p = buf;
	i = 0;
	j = 0;
	while (len > 0) {
		if (puncturepattern[i++]) {
			tmp[j++] = *p++;
			len--;
		} else {
			tmp[j++] = 128;
		}
		if (i == 14)
			i = 0;
	}
	memcpy(buf, tmp, j);

	return j;
}

static void deinterleave(unsigned char *buf, int len)
{
	unsigned char tmp[2*30720];
	int i, j;

	memcpy(tmp, buf, len);

	i = 0;
	j = 0;
	while (i < len) {
		while (rbits16(j) >= len)
			j++;

		buf[i++] = tmp[rbits16(j++)];
	}
}

void recvsymbol(struct rxstate *s, unsigned char symbol)
{
	if (s->msglen < sizeof(s->msgbuf))
		s->msgbuf[s->msglen++] = symbol;
	else
		logprintf(MLOG_INFO, "Message truncated\n");
}

void recvdone(struct rxstate *s, long *metric)
{
	unsigned char buf[2*1280];
	unsigned char msgtemp[2*1280*8];
	unsigned char msgtemp2[2*1280*8];
	unsigned char msgtemp3[2*1280*8];
	int len, ite, errors;
	int i, j, k;
	struct txstate *txstate = s->chan->modstate;
	int iterations;
	float fiterations;
	int rate;
	int terrors, ttotal;
	
	/* deinterleave it */
	if (s->inlv == 2)
		deinterleave(s->msgbuf, s->msglen);

	/* fec decoding */
	switch (s->feclevel) {
	case 0:
		decodenone(buf, s->msgbuf, s->msglen);
		len = s->msglen / 8;
		*metric = 0;
		break;
	case 1:
		s->msglen = unpuncture(s->msgbuf, s->msglen);
		viterbi27(metric, buf, s->msgbuf, s->msglen / 2);
		len = s->msglen / 2 / 8;
		break;
	case 2:
		viterbi27(metric, buf, s->msgbuf, s->msglen / 2);
		len = s->msglen / 2 / 8;
		break;
	case 3:
		viterbi37(metric, buf, s->msgbuf, s->msglen / 3);
		len = s->msglen / 3 / 8;
		break;
	case 4:
		if (s->fecrate > 30) {
			if (s->inlv == 1)
				deinterleave(s->msgbuf, s->msglen);

			for (i=0;i<s->msglen;i++)  
				if (s->msgbuf[i] > 127)
					msgtemp[i/8] |= 1 << (i%8);
				else
					msgtemp[i/8] &= ~(1 << (i%8));
			s->msglen = ((s->msglen / 8) & ~7);
			len = bchdecode(msgtemp, msgtemp3, s->msglen);
			bzero(buf, msgtemp3[0]-3);
			memcpy(buf, msgtemp3+1, msgtemp3[0]-3);
			k = bchencode(msgtemp3, msgtemp2, len);
			len = msgtemp3[0]-3;
			errors = -1;
			if (check_crc_ccitt(msgtemp3, msgtemp3[0])) {
				errors = 0;
				for(i=0;i<k;i++)
					for (j=0;j<8;j++)
						if (((msgtemp[i] ^ msgtemp2[i]) >> j) & 1) 
							errors++;
				sprintf(msgtemp, "\t\t%1.6f\t", (double)errors / (double)(8*s->msglen));
			}
			else {
				sprintf(msgtemp, "\t\t%1.6f\t", 0.5);
			}
			write(2, msgtemp, strlen(msgtemp));
			printf("RX: BCH (Small Packets). Errors (bits) = %d / %d \n", errors, 8*s->msglen);
					
			break;

		}
		iterations = 0;
		len = turbogetoutput(1024, s->fecrate, 1);
		sprintf(msgtemp, "\t%d\t", s->fecrate);
		write(2, msgtemp, strlen(msgtemp));
	
		ttotal = terrors = 0;
		for (i=0; i<s->msglen/len; i++) {
			if (s->inlv == 1)
				deinterleave(s->msgbuf+len*i, i==(s->msglen/len - 1)?s->msglen-(i*len):len);
			ite = turbodecode(s->msgbuf+len*i, buf+(1024/8)*i, 1024, s->fecrate, 10, 1);
			errors = -1;
			if (ite >= 0) {
				iterations += ite*ite;
				turboencode(buf+(1024/8)*i, msgtemp, 1024, s->fecrate, 1);
				errors = 0;
				for (j=0; j<len; j++) {
					if (msgtemp[j] == 1 && s->msgbuf[j+len*i] != 255)
						errors++;
					if (msgtemp[j] == 0 && s->msgbuf[j+len*i] != 0)
						errors++;
				}
			}
			else 
				iterations += 20*20;
			printf("RX: TurboCode-Packet #%d (rate = %d): %d iterations. Errors = %d/%d\n", i, s->fecrate, ite, errors, len);
			if (errors > -1) {
				terrors += errors;
				ttotal += len;	
			}
			else {
				terrors += len/2;
				ttotal += len;
			}
		}
		if (i > 0) { 
			sprintf(msgtemp, "%1.6f\t", (double)terrors / (double)ttotal);
			write(2, msgtemp, strlen(msgtemp));
			fiterations = sqrt(iterations/(float)i);
			printf("RX: Iteraciones cuadratica media = %f\n",fiterations);
			rate = s->fecrate;
			if (fiterations < 2 && rate != 10) {
				if (rate > 11)
					s->channelstate++;
				else if (fiterations == 0) 
					s->channelstate++;
				else  
					s->channelstate = 0;
					
				if (rate > 10 && s->channelstate > 10.0 / (rate - 10.0)) {
					rate--;
					s->channelstate = 0;
				}
			}
			else 
				s->channelstate = 0;

			if (fiterations > 2.5) 
				rate += 2;
			if (fiterations > 5)
				rate += 2;
				
			if (rate > 30)
				rate = 30;
			if (rate < 10)
				rate = 10;
	
			txstate->reqfecrate = rate;
		}
		len = i*1024/8;
		break;
	default:
		len = 0;
		break;
	}

	pktput(s->chan, buf, len);
}

/* --------------------------------------------------------------------- */

static void fft(complex *in, complex *out, float *window)
{
	int i, j, k;
	int s, sep, width, top, bot;
	float tr, ti;

	/* order the samples in bit reverse order and apply window */
	for (i = 0; i < WindowLen; i++) {
		j = rbits8(i) >> (8 - WindowLenLog);
		out[j].re = in[i].re * window[i];
		out[j].im = in[i].im * window[i];
	}

	/* in-place FFT */
	sep = 1;
	for (s = 1; s <= WindowLenLog; s++) {
		width = sep;    /* butterfly width =  2^(s-1) */
		sep <<= 1;      /* butterfly separation = 2^s */
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

static void mixer(struct rxstate *s, complex *buf)
{
	complex z;
	int i;

	for (i = 0; i < HalfSymbol; i++) {
		s->carrphase += s->carrfreq;

		if (s->carrphase > M_PI)
			s->carrphase -= 2 * M_PI;
		if (s->carrphase < -M_PI)
			s->carrphase += 2 * M_PI;

		z.re = cos(s->carrphase);
		z.im = sin(s->carrphase);

		buf[i] = cmul(buf[i], z);
	}
}

/* --------------------------------------------------------------------- */

void newqpskrx(void *state, complex *in)
{
	struct rxstate *s = (struct rxstate *)state;
	complex z;
	int i, j;

	/* make room for new samples at the end of RX window */
	memmove(s->rxwin, s->rxwin + HalfSymbol, (WindowLen - HalfSymbol) * sizeof(complex));

	/* copy the new samples */
	memcpy(s->rxwin + WindowLen - HalfSymbol, in, HalfSymbol * sizeof(complex));

	/* mix the new samples with the internal NCO */
	mixer(s, s->rxwin + WindowLen - HalfSymbol);

	/* apply window function and fft */
	fft(s->rxwin, s->fftbuf, s->rxwindowfunc);

	/* select the wanted FFT bins and adjust the phases */
	s->rxphasecorr = (s->rxphasecorr + HalfSymbol) % WindowLen;
	j = FirstDataCarr;
	for (i = 0; i < DataCarriers; i++) {
		z.re =  CosTable[(j * s->rxphasecorr) % WindowLen];
		z.im = -SinTable[(j * s->rxphasecorr) % WindowLen];
		s->rxpipe[s->rxptr][i] = cmul(s->fftbuf[j], z);
		j += DataCarrSepar;
	}

	/* process the data */
	s->rxroutine(state);

	s->rxptr = (s->rxptr + 1) % RxPipeLen;
}

/* --------------------------------------------------------------------- */

static void rxtune(void *state)
{
	struct rxstate *s = (struct rxstate *)state;
	float x;
	complex z;
	int i, j, cntr;
	unsigned int prev, curr;
	char buf[256];

	curr = s->rxptr;
	prev = (curr - 1) % RxPipeLen;

	j = (FirstTuneCarr - FirstDataCarr) / DataCarrSepar;

	cntr = 0;
	for (i = 0; i < TuneCarriers; i++) {
		x = cpwr(s->rxpipe[curr][j]);
		s->tunepower[i] = avg(s->tunepower[i], x, RxAverFollow);

		z = ccor(s->rxpipe[curr][j], s->rxpipe[prev][j]);
		s->tunecorr[i].re = avg(s->tunecorr[i].re, z.re, RxAverFollow);
		s->tunecorr[i].im = avg(s->tunecorr[i].im, z.im, RxAverFollow);

		if (2 * cmod(s->tunecorr[i]) > s->tunepower[i])
			cntr++;

		j += TuneCarrSepar / DataCarrSepar;
	}

	if (cntr >= TuneCarriers - 1)
		s->acceptance++;
	else if (s->acceptance > 0)
		s->acceptance--;

	if (s->acceptance < 2 * s->mintune)
		return;

#ifdef RxAvoidPTT
	//	if (sm_getptt(state))
	//		return;
#endif

	/* ok, we have a carrier */
	pktsetdcd(s->chan, 1);

	for (i = 0; i < TuneCarriers; i++)
		s->tunephase[i] = carg(s->tunecorr[i]);

	x = phaseavg(s->tunephase, TuneCarriers);
	s->carrfreq += x * 4.0 / WindowLen;

	buf[0] = 0;
	for (i = 0; i < TuneCarriers; i++) {
		x = s->tunepower[i] / (s->tunepower[i] - cmod(s->tunecorr[i]));
		sprintf(buf + strlen(buf), "%+.1fHz/%.1fdB  ",
			s->tunephase[i] * 4.0 / WindowLen /
			(2.0 * M_PI / s->srate),
			10 * log10(x));
	}
	logprintf(MLOG_INFO, "Tune tones: %s\n", buf);

	/* switch to sync mode */
	s->rxroutine = rxsync;
	s->rxwindowfunc = DataWindowInp;
	s->atsymbol = 1;
	s->acceptance = 0;
	s->statecntr = 2 * RxSyncTimeout;
	for (i = 0; i < TuneCarriers; i++) {
		s->power_at[i] = s->tunepower[i];
		s->corr1_at[i].re = s->tunepower[i];
		s->corr1_at[i].im = 0.0;
		s->corr2_at[i].re = s->tunepower[i];
		s->corr2_at[i].im = 0.0;

		s->power_inter[i] = s->tunepower[i];
		s->corr1_inter[i].re = s->tunepower[i];
		s->corr1_inter[i].im = 0.0;
		s->corr2_inter[i].re = s->tunepower[i];
		s->corr2_inter[i].im = 0.0;
	}
}

/* --------------------------------------------------------------------- */

static void rxsync(void *state)
{
	struct rxstate *s = (struct rxstate *)state;
	int i, j, cntr;
	unsigned int prev2, prev1, curr;
	complex *cor1, *cor2, z;
	float *pwr, x;

	/* timeout only if mintune not zero */
	if (s->mintune && s->statecntr-- <= 0) {
		/* timeout waiting sync */
		init_newqpskrx(state);
		return;
	}
	curr = s->rxptr;
	prev1 = (curr - 1) % RxPipeLen;
	prev2 = (curr - 2) % RxPipeLen;

	s->atsymbol ^= 1;

	if (s->atsymbol) {
		pwr = s->power_at;
		cor1 = s->corr1_at;
		cor2 = s->corr2_at;
	} else {
		pwr = s->power_inter;
		cor1 = s->corr1_inter;
		cor2 = s->corr2_inter;
	}

	j = (FirstTuneCarr - FirstDataCarr) / DataCarrSepar;
	for (i = 0; i < TuneCarriers; i++) {
		x = cpwr(s->rxpipe[curr][j]);
		pwr[i] = avg(pwr[i], x, RxAverFollow - 1);

		z = ccor(s->rxpipe[curr][j], s->rxpipe[prev1][j]);
		cor1[i].re = avg(cor1[i].re, z.re, RxAverFollow - 1);
		cor1[i].im = avg(cor1[i].im, z.im, RxAverFollow - 1);

		z = ccor(s->rxpipe[curr][j], s->rxpipe[prev2][j]);
		cor2[i].re = avg(cor2[i].re, z.re, RxAverFollow - 1);
		cor2[i].im = avg(cor2[i].im, z.im, RxAverFollow - 1);

		j += TuneCarrSepar / DataCarrSepar;
	}

	if (!s->atsymbol)
		return;

	cntr = 0;
	for (i = 0; i < TuneCarriers; i++) {
		if (s->power_at[i] > s->power_inter[i]) {
			x = s->power_at[i];
			z = s->corr2_at[i];
		} else {
			x = s->power_inter[i];
			z = s->corr2_inter[i];
		}

		if (-z.re > x / 2)
			cntr++;
	}

	if (cntr >= TuneCarriers - 1)
		s->acceptance++;
	else if (s->acceptance > 0)
		s->acceptance--;

	if (s->acceptance < s->minsync)
		return;

	/* again, we have a carrier */
	pktsetdcd(s->chan, 1);

	cntr = 0;
	for (i = 0; i < TuneCarriers; i++) {
		if (s->corr2_at[i].re < s->corr2_inter[i].re)
			cntr++;
		else
			cntr--;
	}

	if (cntr < 0) {
		s->atsymbol = 0;
		pwr = s->power_inter;
		cor1 = s->corr1_inter;
		cor2 = s->corr2_inter;
	}

	for (i = 0; i < TuneCarriers; i++) {
		z.re = -cor2[i].re;
		z.im = cor1[i].re - (pwr[i] + cor2[i].re) / 2;

		s->syncdelay[i] = carg(z);

		z.re = -cor2[i].re;
		z.im = cor2[i].im;
		s->syncphase[i] = carg(z);
	}

	x = phaseavg(s->syncdelay, TuneCarriers) / M_PI;
	s->skip = (0.5 - x) * SymbolLen;

	logprintf(MLOG_INFO, "Sync: %d (%s-symbol)\n", s->skip, s->atsymbol ? "at" : "inter");

	x = phaseavg(s->syncphase, TuneCarriers);
	s->carrfreq -= x * 2.0 / WindowLen;
	logprintf(MLOG_INFO, "Preamble at: %+.2fHz\n",
		  s->carrfreq / (2.0 * M_PI / s->srate));

	/* switch to data mode */
	s->rxroutine = rxdata;
	s->atsymbol ^= 1;
	s->statecntr = 0;
	s->foundcblock = 0;

	for (i = 0; i < DataCarriers; i++) {
		s->phesum[i] = 0.0;
		s->pheavg[i] = 0.0;
		s->dcdavg[i] = 0.0;
		s->power[i]  = 0.0;
		s->correl[i] = 0.0;
		s->fecerrors[i] = 0;
	}

	s->phemax = 0.0;
}

/* --------------------------------------------------------------------- */

static void rxdata(void *state)
{
	struct rxstate *s = (struct rxstate *)state;
	unsigned char rxsoftword[SymbolBits][DataCarriers];
	unsigned char bpskword[DataCarriers];
	int i, j, bits, data, dcd, cor1, cor2;
	unsigned int prev2, prev1, curr;
	float pherr[DataCarriers];
	complex z;
	float x;
	char buf[256];
	char temp[256];
	long metric;
	struct modemchannel *mc = s->chan;
	struct txstate *txstate = mc->modstate;
		

	/* nothing to do if inter-symbol */
	if ((s->atsymbol ^= 1) == 0)
		return;

	if (!s->foundcblock && s->statecntr++ >= RxDataTimeout) {
		/* timeout waiting control block */
		logprintf(MLOG_INFO, "Timeout waiting control block\n");
		init_newqpskrx(state);
		return;
	}

	curr = s->rxptr;
	prev1 = (curr - 1) % RxPipeLen;
	prev2 = (curr - 2) % RxPipeLen;

	for (i = 0; i < DataCarriers; i++) {
		z = ccor(s->rxpipe[curr][i], s->rxpipe[prev2][i]);

		/* bpsk demodulation */
		bpskword[i] = (z.re > 0) ? 255 : 0;

		/* get the angle and add bias */
		x = carg(z) + M_PI / PhaseLevels;

		/* shift it to range 0 ... 2*PI */
		if (x < 0)
			x += 2 * M_PI;

		/* work out the bits */
		bits = (int) (x * PhaseLevels / (2 * M_PI));
		bits &= (1 << SymbolBits) - 1;

		/* calculate phase error (`0.5' compensates the bias) */
		pherr[i] = x - (bits + 0.5) * 2 * M_PI / PhaseLevels;

		/* flip the top bit back */
		bits ^= 1 << (SymbolBits - 1);

		/* gray decode */
		data = 0;
		for (j = 0; j < SymbolBits; j++)
			data ^= bits >> j;

		/* put the bits to rxword */
		for (j = 0; j < SymbolBits; j++) {
			if (data & (1 << j))
				rxsoftword[j][i] = 255;
			else
				rxsoftword[j][i] = 0;
		}

		/* skip the rest if control block not found yet */
		if (!s->foundcblock)
			continue;

		/* update phase error power average */
		s->dcdavg[i] = avg2(s->dcdavg[i], pherr[i] * pherr[i], DCDTuneAverWeight);

		/* update phase error average */
		s->pheavg[i] = avg2(s->pheavg[i], pherr[i], DCDTuneAverWeight);

		/* update carrier power average */
		x = cpwr(s->rxpipe[curr][i]);
		s->power[i] = avg(s->power[i], x, RxDataSyncFollow);

		/* update sync correlation average */
		x = ccorI(s->rxpipe[prev1][i], s->rxpipe[curr][i]) -
		    ccorI(s->rxpipe[prev1][i], s->rxpipe[prev2][i]);
		s->correl[i] = avg(s->correl[i], x, RxDataSyncFollow);
	}

	if (!s->foundcblock) {
		if (dec_cblock(bpskword, &s->pktlen, &s->feclevel, &s->fecrate, &txstate->fecrate) != -1) {
			s->pktlen = 2*s->pktlen;
			s->msglen = 0;
			s->statecntr = 0;
			s->foundcblock = 1;
			printf("RX: Control block: packet length: %d, FEC level: %d, FEC = %d, requested FEC = %d\n",s->pktlen, s->feclevel, s->fecrate, txstate->fecrate);
			txstate->tcnoresponse = 0;
		}
		return;
	}

	/* feed the data to the decoder */
	for (i = 0; i < SymbolBits; i++)
		for (j = 0; j < DataCarriers; j++)
			recvsymbol(s, rxsoftword[i][j]);

	/* count carriers that have small enough phase error */
	for (dcd = 0, i = 0; i < DataCarriers; i++)
		if (s->dcdavg[i] < DCDThreshold)
			dcd++;

	/* decide if this was a "good" symbol */
	if (dcd >= DataCarriers / 2) {
		/* sync tracking */
		cor1 = 0;
		cor2 = 0;
		for (i = 0; i < DataCarriers; i++) {
			if (s->power[i] < fabs(s->correl[i]) * RxSyncCorrThres) {
				if (s->correl[i] >= 0)
					cor1++;
				else
					cor2++;
			}
		}

		if (cor1 > DataCarriers * 2 / 3)
			s->skip--;
		else if (cor2 > DataCarriers * 2 / 3)
			s->skip++;

		if (s->skip != 0) {
			for (i = 0; i < DataCarriers; i++)
				s->correl[i] /= 2.0;
			logprintf(MLOG_INFO, "Correcting sync: %+d\n", s->skip);
		}

		/* sum up phase errors */
		for (i = 0; i < DataCarriers; i++)
			s->phesum[i] += pherr[i] * pherr[i];

		/* sum up maximum possible error */
		s->phemax += M_PI * M_PI / PhaseLevels / PhaseLevels;

		/* correct frequency error */
		x = phaseavg(pherr, DataCarriers);
		s->carrfreq += RxFreqFollowWeight * x * 2.0 / WindowLen;
	}

	s->statecntr++;

	if (s->statecntr < s->pktlen)
		return;

	recvdone(s, &metric);

	logprintf(MLOG_INFO, "Packet done at: %+.2fHz, final path metric: %ld\n",
		  s->carrfreq / (2.0 * M_PI / s->srate),
		  metric);

	buf[0] = 0;
	for (i = 0; i < DataCarriers; i++) {
		sprintf(buf + strlen(buf), "%2.0f ", 10 * log10(s->phemax * 2 / s->phesum[i]));
	}
	logprintf(MLOG_INFO, "S/N ratio:  %s dB\n", buf);
	for (i=0,x=0.0;i<DataCarriers;i++)
		x += s->phesum[i];

	sprintf(temp,"%2.1f\n", 10 * log10((DataCarriers*s->phemax *2) / x));
	write(2,temp, strlen(temp));
	printf("S/N ratio = %2.1f\n", 10 * log10((DataCarriers*s->phemax *2) / x));

	/* go back to waiting tune */
	init_newqpskrx(state);
	return;
}

/* --------------------------------------------------------------------- */

