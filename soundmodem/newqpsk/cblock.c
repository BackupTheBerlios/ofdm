#include <stdio.h>
#include <string.h>

#include "modemconfig.h"
#include "cblock.h"
#include "viterbi37.h"
#include "crc.h"

#define	CBlockBits	(CBlockLen*DataCarriers)
#define	CBlockDLen	(CBlockBits/8/3)

#if (CBlockBits == 120)
/*
 * Hard coded interleaver table for CBlockBits = 120.
 */
static int interleavetable[CBlockBits] = {
	   0,   64,   32,   96,   16,   80,   48,  112,
	   8,   72,   40,  104,   24,   88,   56,    4,
	  68,   36,  100,   20,   84,   52,  116,   12,
	  76,   44,  108,   28,   92,   60,    2,   66,
	  34,   98,   18,   82,   50,  114,   10,   74,
	  42,  106,   26,   90,   58,    6,   70,   38,
	 102,   22,   86,   54,  118,   14,   78,   46,
	 110,   30,   94,   62,    1,   65,   33,   97,
	  17,   81,   49,  113,    9,   73,   41,  105,
	  25,   89,   57,    5,   69,   37,  101,   21,
	  85,   53,  117,   13,   77,   45,  109,   29,
	  93,   61,    3,   67,   35,   99,   19,   83,
	  51,  115,   11,   75,   43,  107,   27,   91,
	  59,    7,   71,   39,  103,   23,   87,   55,
	 119,   15,   79,   47,  111,   31,   95,   63
};
#endif

#if (CBlockBits == 240)
/*
 * Hard coded interleaver table for CBlockBits = 240.
 */
static int interleavetable[CBlockBits] = {
	   0,  128,   64,  192,   32,  160,   96,  224, 
	  16,  144,   80,  208,   48,  176,  112,    8, 
	 136,   72,  200,   40,  168,  104,  232,   24, 
	 152,   88,  216,   56,  184,  120,    4,  132, 
	  68,  196,   36,  164,  100,  228,   20,  148, 
	  84,  212,   52,  180,  116,   12,  140,   76, 
	 204,   44,  172,  108,  236,   28,  156,   92, 
	 220,   60,  188,  124,    2,  130,   66,  194, 
	  34,  162,   98,  226,   18,  146,   82,  210, 
	  50,  178,  114,   10,  138,   74,  202,   42, 
	 170,  106,  234,   26,  154,   90,  218,   58, 
	 186,  122,    6,  134,   70,  198,   38,  166, 
	 102,  230,   22,  150,   86,  214,   54,  182, 
	 118,   14,  142,   78,  206,   46,  174,  110, 
	 238,   30,  158,   94,  222,   62,  190,  126, 
	   1,  129,   65,  193,   33,  161,   97,  225, 
	  17,  145,   81,  209,   49,  177,  113,    9, 
	 137,   73,  201,   41,  169,  105,  233,   25, 
	 153,   89,  217,   57,  185,  121,    5,  133, 
	  69,  197,   37,  165,  101,  229,   21,  149, 
	  85,  213,   53,  181,  117,   13,  141,   77, 
	 205,   45,  173,  109,  237,   29,  157,   93, 
	 221,   61,  189,  125,    3,  131,   67,  195, 
	  35,  163,   99,  227,   19,  147,   83,  211, 
	  51,  179,  115,   11,  139,   75,  203,   43, 
	 171,  107,  235,   27,  155,   91,  219,   59, 
	 187,  123,    7,  135,   71,  199,   39,  167, 
	 103,  231,   23,  151,   87,  215,   55,  183, 
	 119,   15,  143,   79,  207,   47,  175,  111, 
	 239,   31,  159,   95,  223,   63,  191,  127, 
};
#endif

static void interleave(unsigned char *out, unsigned char *in)
{
	int i;

	for (i = 0; i < CBlockBits; i++)
		out[interleavetable[i]] = in[i];
}

static void deinterleave(unsigned char *out, unsigned char *in)
{
	int i;

	for (i = 0; i < CBlockBits; i++)
		out[i] = in[interleavetable[i]];
}

void enc_cblock(unsigned *cblock, int len, int lvl, int fecrate, int reqfecrate)
{
	unsigned char data[CBlockDLen];
	unsigned char symbols[CBlockBits];
	unsigned char tmp[CBlockBits],  *ptr;
	int i, j;

	memset(data, 0, sizeof(data));

	data[0] = ((len >> 8) & 0x03) | ((lvl << 2) & 0x3c) | (((reqfecrate-10) & 0x18) << 3);
	data[1] = len & 0xff;
	data[2] = ((fecrate - 10) & 0x1f) | (((reqfecrate - 10) & 0x7) << 5) ;

	append_crc_ccitt(data, 3);

//	fprintf(stderr, "enc_cblock: len=%d lvl=%d (%d %d %d %d)\n", len, lvl, data[0], data[1], data[2], data[3]);

	encode37(symbols, data, CBlockDLen);

	interleave(tmp, symbols);

	memset(cblock, 0, (CBlockLen + 1) * sizeof(unsigned));

	ptr = tmp;
	for (i = 0; i < CBlockLen; i++)
		for (j = 0; j < DataCarriers; j++)
			cblock[i + 1] |= *ptr++ << j;
}

int dec_cblock(unsigned char data[DataCarriers], int *len, int *lvl, int *fecrate, int *reqfecrate)
{
	static unsigned char symbols[CBlockBits];
	unsigned char tmp[CBlockBits];
	unsigned char mesg[CBlockDLen];
	unsigned long metric;
	static int counter = 0;
	int i;

	memmove(symbols, symbols + DataCarriers, sizeof(symbols) - DataCarriers);

	for (i = 0; i < DataCarriers; i++)
		symbols[i + CBlockBits - DataCarriers] = data[i];

	if (++counter < CBlockLen)
		return -1;

	deinterleave(tmp, symbols);

	viterbi37(&metric, mesg, tmp, CBlockBits / 3);

	if (!check_crc_ccitt(mesg, 5))
		return -1;

	*len = ((mesg[0] & 0x03) << 8) | mesg[1];
	*lvl = ((mesg[0] & 0x3c) >> 2);
	*fecrate = ((mesg[2] & 0x1f) + 10);
	*reqfecrate = (((mesg[2] & 0xe0) >> 5) + 10) + ((mesg[0] & 0xc0) >> 3);

//	fprintf(stderr, "dec_cblock: len=%d lvl=%d (%d %d %d %d)\n", *len, *lvl, mesg[0], mesg[1], mesg[2], mesg[3]);

	return 0;
}
