#ifndef _FEC_H
#define _FEC_H
void bchGenerate64(int);
int bchDecode64(int in[2]);
void bchEncode64(int in,int out[2]);
int bchencode(unsigned char *in, unsigned char *out, int length);
int bchdecode(unsigned char *in, unsigned char *out, int length);

/* --------------------------------------------------------------------- */

#endif
