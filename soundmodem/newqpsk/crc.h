#ifndef	_CRC_H
#define	_CRC_H

extern unsigned short calc_crc_ccitt(const unsigned char *buffer, int len);
extern void append_crc_ccitt(unsigned char *buffer, int len);
extern int check_crc_ccitt(const unsigned char *buffer, int len);

#endif
