/*****************************************************************************/

/*
 *      ptt.c  --  PTT signalling.
 *
 *      Copyright (C) 1999-2000
 *        Thomas Sailer (sailer@ife.ee.ethz.ch)
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
 */

/*****************************************************************************/

#define _GNU_SOURCE
#define _REENTRANT

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "modem.h"
#include "pttio.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#ifdef HAVE_SYS_IOCCOM_H
#include <sys/ioccom.h>
#endif

#ifdef HAVE_LINUX_PPDEV_H
#include <linux/ppdev.h>
#else
#include "ppdev.h"
#endif

#if defined HAVE_STRING_H
# include <string.h>
#else
# include <strings.h>
#endif

/* ---------------------------------------------------------------------- */

struct modemparams pttparams[] = {
	{ "file", "PTT Driver", "Path name of the serial or parallel port driver for outputting PTT", "none", MODEMPAR_COMBO, 
	  { c: { { "none", "/dev/ttyS0", "/dev/ttyS1", "/dev/parport0", "/dev/parport1" } } } },
	{ NULL }
};

/* ---------------------------------------------------------------------- */

int pttinit(struct pttio *state, const char *params[])
{
	const char *path = params[0];
	int fd;
	unsigned char x;
	unsigned int y = 0;

	if (!path || !path[0] || !strcasecmp(path, "none")) {
		state->fd = -1;
		return 0;
	}
	if ((fd = open(path, O_RDWR, 0)) < 0) {
		logprintf(MLOG_ERROR, "Cannot open PTT device \"%s\"\n", path);
		state->fd = -1;
		return -1;
	}
	if (!ioctl(fd, TIOCMBIC, &y)) {
		state->fd = fd;
		state->mode = serport;
	} else if (!ioctl(fd, PPRDATA, &x)) {
		state->fd = fd;
		state->mode = parport;
	} else {
		logprintf(MLOG_ERROR, "Device \"%s\" neither parport nor serport\n", path);
		close(fd);
		return -1;
	}
	pttsetptt(state, 0);
	pttsetdcd(state, 0);
        return 0;
}

void pttsetptt(struct pttio *state, int pttx)
{
	unsigned char reg;
	unsigned char y = TIOCM_RTS;

	if (!state || state->fd == -1)
		return;
	state->ptt = !!pttx;
	if (state->mode == serport)
		ioctl(state->fd, state->ptt ? TIOCMBIS : TIOCMBIC, &y);
	else if (state->mode == parport) {
		reg = state->ptt | (state->dcd << 1);
		ioctl(state->fd, PPWDATA, &reg);
	}
}

void pttsetdcd(struct pttio *state, int dcd)
{
	unsigned char reg;
	unsigned char y = TIOCM_DTR;

	if (!state || state->fd == -1)
		return;
	state->dcd = !!dcd;
	if (state->mode == serport)
		ioctl(state->fd, state->dcd ? TIOCMBIS : TIOCMBIC, &y);
	else if (state->mode == parport) {
		reg = state->ptt | (state->dcd << 1);
		ioctl(state->fd, PPWDATA, &reg);
	}
}

void pttrelease(struct pttio *state)
{
	if (!state || state->fd == -1)
		return;
	pttsetptt(state, 0);
	pttsetdcd(state, 0);
	close(state->fd);
	state->fd = -1;
}
