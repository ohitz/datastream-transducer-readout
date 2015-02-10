/*
 * Copyright (C) 2007-2008 Oliver Hitz <oliver@net-track.ch>
 *
 * $Id: serial.h,v 1.3 2008-08-14 08:53:34 oli Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __SERIAL_H
#define __SERIAL_H

#include "string.h"

#define SER_OK 0
#define SER_ERROR -1
#define SER_TIMEOUT -2

int serial_write(int fd, string *s);
int serial_writeb(int fd, char *b);
int serial_readline(int fd, string *s);
int serial_readchars(int fd, string *s, int n);

#endif /* __SERIAL_H */
