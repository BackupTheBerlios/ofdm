/*
 * 8-bit parity lookup table, generated by partab.c
 */
unsigned char Partab[] = {
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
};

/* 
 * Metrics table for soft decision Viterbi decoder
 * 
 * Eb/N0     = 5.00 dB
 * Amplitude = 90
 * Rate      = 1/3
 * 
 */
int mettab[2][256] = {
	{
	   4,    4,    4,    4,    4,    4,    4,    4, 
	   4,    4,    4,    4,    4,    4,    4,    4, 
	   4,    4,    4,    4,    4,    4,    4,    4, 
	   4,    4,    4,    4,    4,    4,    4,    4, 
	   4,    4,    4,    4,    4,    4,    4,    4, 
	   4,    4,    4,    4,    4,    4,    4,    4, 
	   4,    4,    4,    4,    4,    4,    4,    4, 
	   4,    4,    4,    4,    4,    4,    4,    4, 
	   4,    4,    4,    4,    4,    4,    4,    4, 
	   4,    4,    4,    4,    4,    3,    3,    3, 
	   3,    3,    3,    3,    3,    3,    3,    3, 
	   3,    3,    3,    3,    3,    3,    3,    3, 
	   3,    3,    3,    3,    3,    3,    3,    2, 
	   2,    2,    2,    2,    2,    2,    2,    2, 
	   2,    2,    2,    1,    1,    1,    1,    1, 
	   1,    1,    1,    1,    1,    0,    0,    0, 
	   0,    0,    0,    0,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -2,   -2,   -2,   -2,   -2,   -2, 
	  -3,   -3,   -3,   -3,   -3,   -4,   -4,   -4, 
	  -4,   -4,   -5,   -5,   -5,   -5,   -5,   -6, 
	  -6,   -6,   -6,   -6,   -7,   -7,   -7,   -7, 
	  -8,   -8,   -8,   -8,   -9,   -9,   -9,   -9, 
	 -10,  -10,  -10,  -10,  -11,  -11,  -11,  -11, 
	 -12,  -12,  -12,  -12,  -13,  -13,  -13,  -13, 
	 -14,  -14,  -14,  -14,  -15,  -15,  -15,  -15, 
	 -16,  -16,  -16,  -16,  -17,  -17,  -17,  -17, 
	 -18,  -18,  -18,  -19,  -19,  -19,  -19,  -20, 
	 -20,  -20,  -20,  -21,  -21,  -21,  -21,  -22, 
	 -22,  -22,  -23,  -23,  -23,  -23,  -24,  -24, 
	 -24,  -24,  -25,  -25,  -25,  -26,  -26,  -26, 
	 -26,  -27,  -27,  -27,  -27,  -28,  -28,  -28, 
	 -28,  -29,  -29,  -29,  -30,  -30,  -30,  -37
	}, {
	 -37,  -30,  -30,  -30,  -30,  -29,  -29,  -29, 
	 -28,  -28,  -28,  -28,  -27,  -27,  -27,  -27, 
	 -26,  -26,  -26,  -26,  -25,  -25,  -25,  -24, 
	 -24,  -24,  -24,  -23,  -23,  -23,  -23,  -22, 
	 -22,  -22,  -21,  -21,  -21,  -21,  -20,  -20, 
	 -20,  -20,  -19,  -19,  -19,  -19,  -18,  -18, 
	 -18,  -17,  -17,  -17,  -17,  -16,  -16,  -16, 
	 -16,  -15,  -15,  -15,  -15,  -14,  -14,  -14, 
	 -14,  -13,  -13,  -13,  -13,  -12,  -12,  -12, 
	 -12,  -11,  -11,  -11,  -11,  -10,  -10,  -10, 
	 -10,   -9,   -9,   -9,   -9,   -8,   -8,   -8, 
	  -8,   -7,   -7,   -7,   -7,   -6,   -6,   -6, 
	  -6,   -6,   -5,   -5,   -5,   -5,   -5,   -4, 
	  -4,   -4,   -4,   -4,   -3,   -3,   -3,   -3, 
	  -3,   -2,   -2,   -2,   -2,   -2,   -2,   -1, 
	  -1,   -1,   -1,   -1,   -1,    0,    0,    0, 
	   0,    0,    0,    0,    1,    1,    1,    1, 
	   1,    1,    1,    1,    1,    1,    2,    2, 
	   2,    2,    2,    2,    2,    2,    2,    2, 
	   2,    2,    3,    3,    3,    3,    3,    3, 
	   3,    3,    3,    3,    3,    3,    3,    3, 
	   3,    3,    3,    3,    3,    3,    3,    3, 
	   3,    3,    3,    3,    4,    4,    4,    4, 
	   4,    4,    4,    4,    4,    4,    4,    4, 
	   4,    4,    4,    4,    4,    4,    4,    4, 
	   4,    4,    4,    4,    4,    4,    4,    4, 
	   4,    4,    4,    4,    4,    4,    4,    4, 
	   4,    4,    4,    4,    4,    4,    4,    4, 
	   4,    4,    4,    4,    4,    4,    4,    4, 
	   4,    4,    4,    4,    4,    4,    4,    4, 
	   4,    4,    4,    4,    4,    4,    4,    4, 
	   4,    4,    4,    4,    4,    4,    4,    4
	}
};