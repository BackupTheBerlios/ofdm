#ifndef _VITERBI_H
#define _VITERBI_H

/* in viterbi27.c */
extern int encode27(unsigned char *, unsigned char *, unsigned int);
extern int viterbi27(unsigned long *, unsigned char *, unsigned char *, unsigned int nbits);

/* in viterbi37.c */
extern int encode37(unsigned char *, unsigned char *, unsigned int);
extern int viterbi37(unsigned long *, unsigned char *, unsigned char *, unsigned int nbits);

#endif
