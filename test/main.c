//Programa demostracion
#include "turbo.h"

int main(int argc, char **argv) 
{
	int rate = 30;
	int crc = 1;
	int maxite = 20;
	int size = 1024;
	char source[1024/8];
	char bcoded[1024*4];
	char channel[1024*4];
	char dest[1024/8];
	int ldest;
	int i;
	
	createturboencodetable(size, crc);
	ldest = turbogetoutput( size, rate, crc);
	ldest = turboencode(source, bcoded, size, rate, crc);
	for (i=0; i<ldest; i++) {
		if (bcoded[i])
			channel[i]=255;
		else
			channel[i]=0;
	}
	
	turbodecode(channel, dest, size, rate, maxite, crc);
	for (i=0; i<size/8; i++)
		if (dest[i] != source[i]) {
			printf("Error!");
			exit(1);
		}
					
	exit(0);
}
