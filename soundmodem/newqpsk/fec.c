#include "fec.h"


/** Fec (64,x) for Small Packets  */
/*  Possible x = 7, 10, 16, 18, 24, 30, 36 
   ******************************************* */


#define POL7  05231045543503271737
#define POL10 0472622305527250155
#define POL16 06331141367235453


static int bchHeaderTable[65536][2];
static int l_value, l_value_log;

int bchencode(unsigned char *in, unsigned char *out, int length)
{
	int i, j, k ,n;
	in[length] = 0;
	in[length+1] = 0;
	k = (8*length)/l_value_log;
	if (((8*length)/l_value_log)%8)
		k++;
	for (j=0, i=0; i < k; j+=8, i++) {
		n = *(int *)&in[(l_value_log*i)/8];
		bchEncode64(( n >> ((l_value_log*i)%8)) & (l_value - 1), (int *)&out[j]);
	}
	return (j);
}

int bchdecode(unsigned char *in, unsigned char *out, int length)
{
	int i, j, k;
	for (i=0; i < ((l_value_log*length)/64)+1; i++)
		out[i] = 0;
	for (i=0, j=0;i<length; i+=8, j+=l_value_log) {
		k = *(int *)&out[j/8];
		k = *(int *)&in[i];
		k = j % 8;
		k = j / 8;
		*(int *)&out[j/8] |= bchDecode64((int *)&in[i]) << (j%8);
		k = *(int *)&out[j/8];
	}
	i = (l_value_log*(length/8))/8;
	if ((l_value_log*(length/8) % 8))
		i++;
	return (i);
}
	

void bchGenerate64(int x)
{
	int i, j, k, n;
	int nval[3]={7,10,16};
	int bchMatrix[16][63];
	int len_code;
	
	int bchHeaderPoly[3][2]={{POL7 & 0xffffffff,(POL7  >> 32) & 0xffffffff},				{POL10 & 0xffffffff,(POL10 >> 32) & 0xffffffff},				{POL16 & 0xffffffff,(POL16 >> 32) & 0xffffffff}};
	n = 0;			
			
	while(n < 3 && nval[n] != x)
		n++;

	if (n == 3)
		return;
	
	len_code = 0;
	for (i=0; i<64;i++) {
		if (i<32) {
			if ((bchHeaderPoly[n][0] >> i) & 1)
				len_code = i;
		}
		else {
		        if ((bchHeaderPoly[n][1] >> (i-32)) & 1)
				len_code = i;
		}
	}
	len_code++;
	
	for(i=0;i<x;i++)
		for(j=0;j<63;j++) {
			k=j-i;
			while(k<0)
				k+=len_code;
			bchMatrix[i][j]=(bchHeaderPoly[n][k/32]>>(k%32))&1;
		}
	l_value_log = x;
	l_value = 1;
	i = x;
	while (i--)
		l_value <<= 1;
	for(i=0;i<l_value;i++) {
		bchHeaderTable[i][0] = 0;
		bchHeaderTable[i][1] = 0;
		for(j=0;j<63;j++)
			for(k=0;k<x;k++) 
				bchHeaderTable[i][j/32] ^= (((i>>k)&1)* bchMatrix[k][j])<<(j%32);
	}
}

int bchDecode64(int in[2])
{
	int i, j, k, n;
	int min, out=0;

	min = 63;
	
	for (i=0;i<l_value;i++) {
		for(j=0, k=0;j<63 && k<min;j++)
			k += (((bchHeaderTable[i][j/32] ^ in[j/32]) >> (j%32)) &1);
		if (k < min) {
			min = k;
			out = i;
		}
	}
	return (out);
}

void bchEncode64(int in,int out[2])
{
	out[0] = bchHeaderTable[in][0];
	out[1] = bchHeaderTable[in][1];
}


