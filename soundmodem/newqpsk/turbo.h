#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

unsigned turboencode(char *cmesg,char *dest,unsigned size,unsigned rate, unsigned crc);
void createturboencodetable(unsigned size, unsigned crc);
unsigned turbodecode (unsigned char *channelmesg, char *, unsigned size, unsigned rate, unsigned maxite, unsigned crc);
unsigned turbogetoutput(unsigned size, unsigned rate, unsigned crc);
