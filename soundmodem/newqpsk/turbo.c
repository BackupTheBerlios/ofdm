/*****************************************************************************/

/*
 *      turbo.c  --  BCJR turbo code encoder/decoder
 *
 *      Copyright (C) 1998-2002
 *        Mathys Walma (walma@hotmail.com)
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
 *
 */

/*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define true 1
#define false 0
#define DEFAULT_NOISE 0.1

#include "turbo.h"

// Turbo Code encoder & decoder by Mathys Walma, 1998

// Some improvements by Arnau Sanchez, 2002
// Conversion to C, CRC-check, puncturing mechanism and noise estimation
// Also speed optimization (3 times faster now on my AMD)
// arnau@softhome.net arnau@gbt.tfo.upm.es
//
// encodes mesg into parity1, and if force it true it modifies mesg to
// force the encoder to the zero state by the last bit.
typedef int bool;

static inline u_int16_t calc_crc(const u_int8_t *buffer, int len);
static inline void append_crc(u_int8_t *buffer, int len);
static inline int check_crc(const u_int8_t *buffer, int len);

void encode_rsc(bool *mesg,bool *parity1,unsigned size, bool force);
// binary addition with no carry
bool add(bool a,bool b);
void interleave(bool *mesg, unsigned size);
void deinterleave(bool *mesg, unsigned size);
void deinterleavedouble(double *mesg, unsigned size);
void interleavedouble(double *mesg, unsigned size);
void createturbointerleave(unsigned size);
void createturboencodetable(unsigned size, unsigned crc);
void inttobool(unsigned state, bool *array, unsigned size);
void booltoint(bool *array, unsigned size, unsigned *state);

unsigned *interleavearray;
unsigned *deinterleavearray;

// global information about the encoder used by the encoder and the decoder
// routine
// how many states are in the encoder (a power of 2)
unsigned numstates;
// log2(numstates)
unsigned memory;
// [2] = input, [16] = current state, tostate[2][16] = next state
unsigned *tostate[2];
// [2] = last input, [16] = current state, fromstate[2][16] = previous state
unsigned *fromstate[2];
// [2] = input, [16] = current state, output[2][16] = output of encoder
bool *output[2];

void createturboencodetable(unsigned size, unsigned crc)
{

   bool *boolstate;
   bool *newstate;
   unsigned input, intstate;
   memory = 4;
   size+=memory;
   if (crc)
	   size+=16;
   interleavearray = malloc(sizeof(bool)*size);
   deinterleavearray = malloc(sizeof(bool)*size);
   numstates = 16;

   createturbointerleave(size-memory);

   // create arrays used by encode and decode
   output[0] = malloc(sizeof(bool)*numstates);
   output[1] = malloc(sizeof(bool)*numstates);
   fromstate[0] = malloc(sizeof(int)*numstates);
   fromstate[1] = malloc(sizeof(int)*numstates);
   tostate[0] = malloc(sizeof(int)*numstates);
   tostate[1] = malloc(sizeof(int)*numstates);

   boolstate = malloc(sizeof(bool)*memory);
   newstate = malloc(sizeof(bool)*memory);
   for (input=0;input<2;input++)
     for (intstate=0;intstate<numstates;intstate++)
       {
	  bool boolinput = (input == 0) ? false : true;

	  inttobool(intstate,boolstate,memory);

	  // calculate output due to the input
	  output[input][intstate] = add(boolinput,boolstate[0]);
	  output[input][intstate] = add(output[input][intstate],boolstate[3]);
	  output[input][intstate] = add(output[input][intstate],boolstate[1]);
	  output[input][intstate] = add(output[input][intstate],boolstate[2]);
	  output[input][intstate] = add(output[input][intstate],boolstate[3]);

	  // calculate new states
	  newstate[3] = boolstate[2];
	  newstate[2] = boolstate[1];
	  newstate[1] = boolstate[0];
	  newstate[0] = add(add(boolinput,boolstate[0]),boolstate[3]);
	  // from s' to s
	  booltoint (newstate,memory,&tostate[input][intstate]);
	  // from s to s'
	  fromstate[input][tostate[input][intstate]] = intstate;
       }

   free(boolstate);
   free(newstate);
}

void inttobool(unsigned state, bool *array, unsigned size)
{
   unsigned x, next;
   for (x=0;x<size;x++)
     {
	next = state >> 1;

	if ((next << 1) == state)
	  array[x] = false;
	else
	  array[x] = true;

	state = next;
     }
}

void booltoint(bool *array, unsigned size, unsigned *state)
{
   unsigned x;
   *state = 0;
   for (x=0;x<size;x++)
     if (array[x] == true)
       (*state) |= (1 << x);
}
unsigned turbogetoutput(unsigned size,unsigned rate, unsigned crc)
{
   int x;
   int i, k, l;
   if(rate<10)
     rate=10;
   if(rate>30)
     rate=30;
   rate = rate - 10;

   if (crc)
	   size+=16;
   
   x = (rate*(size - size%20))/20;
   for (i=0, k=0;i<size%20;i++) {
	   if (!(i & 1))
		   l = i >> 1;
	   else
		   l = ((i & ~1) >> 1)+10;
	   if (l%20 < rate)
		   k++;
   }

   x+=k;

   x = size + memory + 2*(x+memory);
   if (x % 8)
	   x = (x & ~7) + 8;
  
   return (x);
}


unsigned turbodecode (unsigned char *cmesg, char *dest, unsigned size, unsigned rate, unsigned maxite, unsigned crc)
{
   double *parity1,*parity2;
   static double *mesg;
   static double *mesgi;
   static double **a[2];
   static double **b[2];
   static double *L[2];
   static double *Lo[2];
   static double *Lc;
   static double *Lci;
   static double *noise;
   static double **gamma[2][2];
   static double **gammat[2][2];
   static double **gammaE[2][2];
   char *bcoded;
   unsigned char *tdest;
   static bool initialized=false;
   static unsigned lastsize=0;
   double topbottom[2];
   unsigned returnvalue=0;
   int i,j,k,l;
   int c, x, y, z, s, u;
   double d, d2;
   int window, nonoise;
   int input, state;
   bool crc_state = false;
   int total_size;

   // rate = inverse of traditional rate * 10
   // Only rates between 1/3 and 1
   if(rate<10)
     rate=10;
   if(rate>30)
     rate=30;
   total_size = turbogetoutput(size, rate, crc);

   if (crc)
	   size += 16;

   parity1=malloc(sizeof(double)*(size+memory));
   parity2=malloc(sizeof(double)*(size+memory));
   tdest=malloc(sizeof(char)*(size+16+7)/8);
   mesg = malloc(sizeof(double)*total_size);
   mesgi = malloc(sizeof(double)*total_size);
   for (i=0, nonoise = 1;i<total_size;i++) {
	   if (cmesg[i] != 255 || cmesg[i] != 0)
		   nonoise = 0;
		   
	   mesgi[i] = mesg[i] = ((double)cmesg[i]-127)/128;
   }
  
   k = (rate-10);

   // Puncturing parity1
   for(i=0,j=0;i<size+memory;i++) {
     // Avoid puncturing tail
     l = i % 20;
     if (!(l & 1)) 
	     l = l >> 1;
     else
	     l = ((l & ~1) >> 1) + 10;
     if( l < k || i>=size)
       {
	  parity1[i]=mesg[size+memory+j];
	  j++;
       }
   else
     parity1[i]=0;
   }
   // Pucturing parity2
   for(i=0;i<size+memory;i++) {
     l = i % 20;
     if (!(l & 1))
	     l = l >> 1;
     else 
	     l = ((l & ~1)>>1) + 10;
     	   
     if( l < k || i>=size)
       {
	  parity2[i]=mesg[size+memory+j];
	  j++;
       }
   else
     parity2[i]=0;
   }
   // encode considering tail
   size+=memory;
 
   Lc = malloc(sizeof(double)*size);
   Lci =malloc(sizeof(double)*size);
   noise = malloc(sizeof(double)*size);
   // Fixed interleaving of message
   interleavedouble(mesgi, size);

   if (crc) {
	   
	bzero(tdest,(size-memory)/8);
	for (k=0;k<size-memory;k++) {
		if (mesg[k] > 0)
		   tdest[k/8] |= 1 << (k%8);
	}
	if(check_crc(tdest, (size-memory)/8)) {
		i = turbogetoutput(size-memory-16, rate, 1);
		bcoded = malloc(sizeof(char)*i);
		i = turboencode(tdest, bcoded, size-memory-16, rate, 1);
		for (k=0; k<i;k++)
			if ((bcoded[k] == 0 && mesg[k] >= 0) || 
					(bcoded[k] == 1 && mesg[k]<0))
				break;
		if (k!=i)
			returnvalue++;
		free(bcoded);
		crc_state = true;
       }
   }
if (!crc_state) { 
   // minimize new's and delete's to only when needed
   if (size != lastsize && initialized == true)
     {
	// delete all the arrays and rebuild
	for (y=0;y<2;y++)
	  for (x=0;x<2;x++)
	    {
	       for (z=0;z<lastsize;z++)
		 {
		    free(gamma[y][x][z]);
		    free(gammaE[y][x][z]);
		 }

	       free(gamma[y][x]);
	       free(gammaE[y][x]);
	    }

	// create L[encoder #]
	for (y=0;y<2;y++) {
	  free(L[y]);
	  free(Lo[y]);
	}

	// create alpha[encoder #][k][state]
	for (x=0;x<2;x++)
	  {
	     for (y=0;y<lastsize;y++)
	       {
		  free(a[x][y]);
		  free(b[x][y]);
	       }

	     free(a[x]);
	     free(b[x]);
	  }
     }

   if (initialized == false || size != lastsize)
     {
	initialized = true;
	lastsize = size;

	// create the arrays dynamically at runtime, delete at end of routine
	// create gamma[encoder #][uk][k][state]
	for (y=0;y<2;y++)
	  for (x=0;x<2;x++)
	    {
	       gamma[y][x] = malloc(sizeof(double)*size);
	       gammat[y][x] = malloc(sizeof(double)*size);
	       gammaE[y][x] = malloc(sizeof(double)*size);
	       for (z=0;z<size;z++)
		 {
		    gamma[y][x][z] = malloc(sizeof(double)*numstates);
		    gammat[y][x][z] = malloc(sizeof(double)*numstates);
		    gammaE[y][x][z] = malloc(sizeof(double)*numstates);
		 }
	    }

	// create L[encoder #]
	for (y=0;y<2;y++) {
	  L[y] = malloc(sizeof(double)*size);
	  Lo[y] = malloc(sizeof(double)*size);
	}

	// create alpha[encoder #][k][state]
	for (x=0;x<2;x++)
	  {
	     a[x] = malloc(sizeof(double)*size);
	     b[x] = malloc(sizeof(double)*size);
	     // each Yk has 'numstates' values of gamma
	     for (y=0;y<size;y++)
	       {
		  a[x][y] = malloc(sizeof(double)*numstates);
		  b[x][y] = malloc(sizeof(double)*numstates);
	       }
	  }
     }

   // initialization of iteration arrays
   for (x=0;x<numstates;x++)
     {
	a[0][0][x] = b[0][size-1][x] = a[1][0][x] = (x==0) ? 1.0 : 0.0;
	// extrinsic information from 2-1
     }

   // initialization of extrinsic information array from decoder 2, used in decoder 1
   for (x=0;x<size;x++) {
     L[1][x] = 0.0;
     Lo[1][x] = 1.0;
   }
    
   // 4*Eb/No
   window=256;
   for(k=0;k<size;k++)
     {
	int sum=0;
	double meannoise=0;
	if (nonoise) {
		noise[k] = DEFAULT_NOISE;
	}
	else {
	
		noise[k]=0;
		for(l=0;l<window;l++)
		  {
	             z = window/2+k-l;
		     if((!(z<0 || z>=size)))
		       {
			  meannoise =fabs(mesg[z]);
			  meannoise+=fabs(parity1[z]);
			  meannoise+=fabs(parity2[z]);
		       }
		     else
		       meannoise+=3.0;
		  }
		meannoise/=window*3;
	
		for(l=0;l<window;l++)
		  {
	           z = window/2 + k - l;
		     if(!(z<0 || z>=size))
		       {
			  noise[k]+=(fabs(mesg[z])-meannoise)*(fabs(mesg[z])-meannoise);
			  noise[k]+=(fabs(parity1[z])-meannoise)*(fabs(parity1[z])-meannoise);
			  noise[k]+=(fabs(parity2[z])-meannoise)*(fabs(parity2[z])-meannoise);
			  sum+=3;
		       }
		  }
		noise[k]/=sum;
		noise[k]=exp(1.12*noise[k])-1;
	      }
	     }
	for(k=0;k<size;k++)
          {
	Lc[k] = (4.0*1.0*1)/(2*noise[k]);
	if (Lc[k] > 10)
		Lc[k] = 10;
	Lci[k]=Lc[k];
         }
   interleavedouble(Lci, size);
   // k from 0 to N-1 instead of 1 to N
	for (k=0;k<size;k++)
	  {
	     // calculate the gamma(s',s);
	     for (input=0;input<2;input++)
	       {
		  double uk = (input == 0) ? -1.0 : 1.0;

		  for (s=0;s<numstates;s++)
		    {
		       double xk = (output[input][s] == 0) ? -1.0 : 1.0;

		       gammaE[0][input][k][s]=exp(0.5*Lc[k]*parity1[k]*xk);
		       gammat[0][input][k][s]=exp(0.5*uk*(Lc[k]*mesg[k]))*gammaE[0][input][k][s];

		       gammaE[1][input][k][s]=exp(0.5*Lc[k]*parity2[k]*xk);
		       gammat[1][input][k][s]=exp(0.5*uk*(Lci[k]*mesgi[k]))*gammaE[1][input][k][s];
		    }
	       }
	  }
   
   for (c=0;c<maxite;c++)
     {
	// k from 0 to N-1 instead of 1 to N
	for (k=0;k<size;k++)
	  {
 	     d = Lo[1][k];
	     d2 = 1/Lo[1][k];
	     // calculate the gamma(s',s);
	     for (input=0;input<2;input++)
	       {
		  for (s=0;s<numstates;s++)
		    {

		       if (input)
       			       gamma[0][input][k][s]=gammat[0][input][k][s]*d;
		       else
       			       gamma[0][input][k][s]=gammat[0][input][k][s]*d2;
		    }
	       }
	  }

	// calculate the alpha terms
	// from 1 to N-1, 0 is precalculated, N is never used
	for (k=1;k<size;k++)
	  {
	     double temp=0;

	     // calculate denominator
	     for (state=0;state<numstates;state++)
	       temp += a[0][k-1][fromstate[0][state]]*gamma[0][0][k-1][fromstate[0][state]] + a[0][k-1][fromstate[1][state]]*gamma[0][1][k-1][fromstate[1][state]];

	     for (state=0;state<numstates;state++)
	       a[0][k][state] = (a[0][k-1][fromstate[0][state]]*gamma[0][0][k-1][fromstate[0][state]] + a[0][k-1][fromstate[1][state]]*gamma[0][1][k-1][fromstate[1][state]])/temp;
	  }

	// from N-1 to
	for (k=size-1;k>=1;k--)
	  {
	     double temp=0;

	     // calculate denominator
	     for (state=0;state<numstates;state++)
	       temp += a[0][k][fromstate[0][state]]*gamma[0][0][k][fromstate[0][state]] + a[0][k][fromstate[1][state]]*gamma[0][1][k][fromstate[1][state]];

	     for (state=0;state<numstates;state++)
	       b[0][k-1][state] = (b[0][k][tostate[0][state]]*gamma[0][0][k][state] + b[0][k][tostate[1][state]]*gamma[0][1][k][state])/temp;
	  }

	for (k=0;k<size;k++)
	  {
	     double min=0;
	     double temp;

	     // find the minimum product of alpha, gamma, beta
	     for (u=0;u<2;u++)
	       for (state=0;state<numstates;state++)
		 {
		    temp=a[0][k][state]*gammaE[0][u][k][state]*b[0][k][tostate[u][state]];
		    	if ((temp < min ) || min == 0) 
		           min = temp;
		 }

	     // if all else fails, make min real small
	     if (min == 0 || min > 1)
	       min = 1E-100;


	     for (u=0;u<2;u++)
	       {
		  topbottom[u]=0.0;

		  for(state=0;state<numstates;state++)
		    topbottom[u] += (a[0][k][state]*gammaE[0][u][k][state]*b[0][k][tostate[u][state]]);
	       }

	     if (topbottom[0] == 0)
	       topbottom[0] = min;
	     else if (topbottom[1] == 0)
	       topbottom[1] = min;

	     Lo[0][k] = sqrt(topbottom[1]/topbottom[0]);
	     L[0][k]  = log(topbottom[1]/topbottom[0]);
	  }

	interleavedouble(L[0],size);
	interleavedouble(Lo[0],size);

	// start decoder 2
	// code almost same as decoder 1, could combine code into one but too lazy
	for (k=0;k<size;k++)
	  {
             d = Lo[0][k];
	     d2 = 1/Lo[0][k];
	     // calculate the gamma(s',s);
	     for (input=0;input<2;input++)
	       {
		  for (s=0;s<numstates;s++)
		    {

		       if (input)
			       gamma[1][input][k][s]=gammat[1][input][k][s]*d;
		       else
			       gamma[1][input][k][s]=gammat[1][input][k][s]*d2;

		    }
	       }
	  }

	// calculate the alpha terms
	for (k=1;k<size;k++)
	  {
	     double temp=0;

	     // calculate denominator
	     for (state=0;state<numstates;state++)
	       temp += a[1][k-1][fromstate[0][state]]*gamma[1][0][k-1][fromstate[0][state]] + a[1][k-1][fromstate[1][state]]*gamma[1][1][k-1][fromstate[1][state]];

	     for (state=0;state<numstates;state++)
	       a[1][k][state] = (a[1][k-1][fromstate[0][state]]*gamma[1][0][k-1][fromstate[0][state]] + a[1][k-1][fromstate[1][state]]*gamma[1][1][k-1][fromstate[1][state]])/temp;
	  }

	// in the first iteration, set b[1][N-1] = a[1][N-1] for decoder 2
	// this decoder can't be terminated to state 0 because of the interleaver
	// the performance loss is supposedly neglible.
	if (c==0)
	  {
	     double temp=0;

	     // calculate denominator
	     for (state=0;state<numstates;state++)
	       temp += a[1][size-1][fromstate[0][state]]*gamma[1][0][size-1][fromstate[0][state]] + a[1][size-1][fromstate[1][state]]*gamma[1][1][size-1][fromstate[1][state]];

	     for (state=0;state<numstates;state++)
	       b[1][size-1][state] = (a[1][size-1][fromstate[0][state]]*gamma[1][0][size-1][fromstate[0][state]] + a[1][size-1][fromstate[1][state]]*gamma[1][1][size-1][fromstate[1][state]])/temp;
	  }

	for (k=size-1;k>=1;k--)
	  {
	     double temp=0;

	     // calculate denominator
	     for (state=0;state<numstates;state++)
	       temp += a[1][k][fromstate[0][state]]*gamma[1][0][k][fromstate[0][state]] + a[1][k][fromstate[1][state]]*gamma[1][1][k][fromstate[1][state]];

	     for (state=0;state<numstates;state++)
	       b[1][k-1][state] = (b[1][k][tostate[0][state]]*gamma[1][0][k][state] + b[1][k][tostate[1][state]]*gamma[1][1][k][state])/temp;
	  }

	for (k=0;k<size;k++)
	  {
	     double min = 0;

	     // find the minimum product of alpha, gamma, beta
	     for (u=0;u<2;u++)
	       for (state=0;state<numstates;state++)
		 {
		    double temp=a[1][k][state]*gammaE[1][u][k][state]*b[1][k][tostate[u][state]];
	    	    if ((temp < min)|| min == 0)
		         min = temp;
		 }
	     // if all else fails, make min real small
	     if (min == 0 || min > 1)
	       min = 1E-100;

	     for (u=0;u<2;u++)
	       {
		  topbottom[u]=0.0;

		  for(state=0;state<numstates;state++)
		    topbottom[u] += (a[1][k][state]*gammaE[1][u][k][state]*b[1][k][tostate[u][state]]);
	       }

	     if (topbottom[0] == 0)
	       topbottom[0] = min;
	     else if (topbottom[1] == 0)
	       topbottom[1] = min;

	     Lo[1][k] = sqrt(topbottom[1]/topbottom[0]);
	     L[1][k] = log(topbottom[1]/topbottom[0]);

	  }

	
	deinterleavedouble(L[1],size);
	deinterleavedouble(Lo[1],size);
	// get L[0] back to normal after decoder 2
	deinterleavedouble(L[0],size);
	deinterleavedouble(Lo[0],size);
	// end decoder 2

        // make decisions
	bzero(tdest,(size-memory)/8);
	for (k=0;k<size-memory;k++) {
		if ((Lc[k]*mesg[k] + L[0][k] + L[1][k]) > 0)
		   tdest[k/8] |= 1 << (k%8);
	}
	returnvalue++;
	if (crc) {
		if(check_crc(tdest, (size-memory)/8)) { 
			crc_state = true;
			break;
		}
 	}
     }
   }
   if (crc)
	   size -= 16;
   memcpy(dest, tdest, size/8);
   if (crc && !crc_state)
	   returnvalue = -1;
   free(Lc);
   free(Lci);
   free(noise);
   free(parity1);
   free(parity2);
   free(tdest);
   return returnvalue;
}
void deinterleavedouble(double *mesg, unsigned size)
{
   double *temp;
   int x;

   temp = malloc(sizeof(double)*size);

   for (x=0;x<size;x++)
     temp[x] = mesg[x];

   for (x=0;x<size;x++)
     mesg[deinterleavearray[x]] = temp[x];

   free(temp);
}

void interleavedouble(double *mesg, unsigned size)
{
   double *temp;
   int x;

   temp = malloc(sizeof(double)*size);

   for (x=0;x<size;x++)
     temp[x] = mesg[x];

   for (x=0;x<size;x++)
     mesg[interleavearray[x]] = temp[x];

   free(temp);
}

void interleave(bool *mesg,unsigned size)
{
   bool *temp;
   int x;

   temp = malloc(sizeof(int)*size);

   for (x=0;x<size;x++)
     temp[x] = mesg[x];

   for (x=0;x<size;x++)
     mesg[interleavearray[x]] = temp[x];

   free(temp);
}

void deinterleave(bool *mesg,unsigned size)
{
   bool *temp;
   int x;

   temp = malloc(sizeof(int)*size);

   for (x=0;x<size;x++)
     temp[x] = mesg[x];

   for (x=0;x<size;x++)
     mesg[deinterleavearray[x]] = temp[x];

   free(temp);
}

void createturbointerleave(unsigned size)
{
   int x, i=0;
   bool *yesno;
   size+=memory;

   yesno = malloc(sizeof(bool)*size);

   for (x=0;x<size;x++)
     yesno[x]=false;
   srand(0x01ff02fe);

   // create an interleave array

   for (x=0;x<size;x++,i+=64)
     {
	unsigned  val;
	if (i >=size)
	  i = (i %size)+1;

	do
	  {
	     val=i%size;
	     val = rand()%size;
	  }
	while(yesno[val] == true);

	yesno[val] = true;
	interleavearray[x] = val;
	deinterleavearray[val] = x;
     }

   free(yesno);
}

unsigned turboencode(char *source,char *dest,unsigned size,unsigned rate, unsigned crc)
{
   bool *mesg,*parity1,*parity2;
   int i,j,k,l;
   unsigned char *cdest = source;
   
   if(rate < 10)
     rate = 10;
   if(rate > 30)
     rate = 30;
  // add crc if needed
   if (crc) {
	   cdest = malloc(sizeof(char) * ((size+16)/8)+1);
	   memcpy(cdest, source, size/8);
	   append_crc(cdest, size/8);
	   size+=16;
	   source = cdest;
   }
   mesg = malloc(sizeof(bool)*(size+memory+7));
   parity1 = malloc(sizeof(bool)*(size+memory+7));
   parity2 = malloc(sizeof(bool)*(size+memory+7));
 
   // Generate systematic bits from original message
   for(i=0,k=0;k<size;i++)
     for(j=0;j<8;j++,k++)
       mesg[k]=(source[i]&(1<<j))!=0?true:false;
   // Generate parity1
   encode_rsc(mesg,parity1,size+memory,true);

   // We must copy mesg before interleaving
   for(i=0;i<size+memory;i++)
     dest[i]=(mesg[i]==true)?1:0;
   interleave(mesg,size+memory);

   // Generate parity2
   encode_rsc(mesg,parity2,size+memory,false);
   // Pucturing
   k = (rate-10);
   for(i=0,j=0;i<size+memory;i++) {
     l = i % 20;
     if (!(l & 1))
	     l = l >> 1;
     else
	     l = ((l & ~1)>>1) + 10;
     if( l < k || i>=size)
       {
	  dest[size+memory+j]=(parity1[i]==true)?1:0;
	  j++;
       }
   }
   for(i=0;i<size+memory;i++) {
     l = i % 20;
     if (!(l & 1))
	     l = l >> 1;
     else
	     l = ((l & ~1)>>1) + 10;
 	   
     if( l < k || i>=size)
       {
	  dest[size+memory+j]=(parity2[i]==true)?1:0;
	  j++;
       }
   }
   j += size+memory;
   // Output multiple of 8 bits
   
   while(j%8) {
	   dest[j]=0;
	   j++;
   }
   free(mesg);
   free(parity1);
   free(parity2);
   if (crc)
	free(cdest);
   
   // number of bits in output
   return(j);
}

void encode_rsc(bool *mesg,bool *parity,unsigned size, bool force)
{

   unsigned state=0;
   int x, uk;

   for (x=0;x<size;x++)
     {
	// force the encoder to zero state at the end
	if (x>=size-memory && force)
	  {
	     if (tostate[0][state]&1)
	       mesg[x] = true;
	     else
	       mesg[x] = false;
	  }

	// can't assume the bool type has an intrinsic value of 0 or 1
	// may differ from platform to platform
	uk = mesg[x] ? 1 : 0;

	// calculate output due to new mesg bit
	parity[x] = output[uk][state];
	// calculate the new state
	state = tostate[uk][state];
     }
}

bool add(bool a, bool b)
{
   return a==b ? false : true;
}

/* ---------------------------------------------------------------------- */
/*
 * the CRC routines are from WAMPES
 * by Dieter Deyke (Thanks!)
 */

const u_int16_t crc_table[0x100] = {
        0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
        0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
        0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
        0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
        0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
        0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
        0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
        0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
        0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
        0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
        0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
        0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
        0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
        0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
        0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
        0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
        0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
        0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
        0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
        0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
        0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
        0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
        0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
        0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
        0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
        0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
        0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
        0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
        0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
        0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
        0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
        0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

/* ---------------------------------------------------------------------- */
static inline u_int16_t calc_crc(const u_int8_t *buffer, int len)
{
        u_int16_t crc = 0xffff;

        for (;len>0;len--)
                crc = (crc >> 8) ^ crc_table[(crc ^ *buffer++) & 0xff];
        crc ^= 0xffff;
	return crc;
}

static inline void append_crc(u_int8_t *buffer, int len)
{
        u_int16_t crc = calc_crc(buffer, len);
        buffer[len] = crc;
        buffer[len+1] = crc >> 8;
}

static inline int check_crc(const u_int8_t *buffer, int len)
{
        u_int16_t crc = calc_crc(buffer, len);
        return (crc & 0xffff) == 0x0f47;
}


