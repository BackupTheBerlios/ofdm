#ifndef _CBLOCK_H
#define _CBLOCK_H

extern void enc_cblock(unsigned *, int, int, int, int);
extern int  dec_cblock(unsigned char data[DataCarriers], int *len, int *lvl, int *fecrate, int *reqfecrate);

#endif
