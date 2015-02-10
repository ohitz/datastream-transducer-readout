/*
 * Copyright (C) 2007-2015 Oliver Hitz <oliver@net-track.ch>
 */

#include <sys/select.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

#include "debug.h"
#include "serial.h"

int serial_write(int fd, string *s)
{
  return serial_writeb(fd, (char *) str_getbuf(s));
}

int serial_writeb(int fd, char *buf)
{
  int w;

#ifdef DEBUG
  int i;
  printf("serial_writeb: ");
  for (i = 0; i < strlen(buf); i++) {
    printf("%02x ", buf[i]);
  }
  printf("\n");
#endif
  
  w = write(fd, buf, strlen(buf));
  return w;
}

int readline(int fd, string *s, char *breakchars, int chars_max)
{
  static char ibuf[80];
  int chars_read = 0;

  while (1) {
    fd_set readfds;
    struct timeval time;
    int rc;

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    /* Wait for input */   
    time.tv_sec = 1;
    time.tv_usec = 0;
    rc = select(fd+1, &readfds, NULL, NULL, &time);

#ifdef DEBUG
    printf("select returned %d\n", rc);
#endif

    if (rc < 0) {

      return SER_ERROR;

    } else if (rc == 0) {

      return SER_TIMEOUT;

    } else {

      if (FD_ISSET(fd, &readfds)) {

	int rrc = read(fd, ibuf, sizeof(ibuf));

#ifdef DEBUG
	printf("serial_readline: read %d characters\n", rrc);
#endif
	
	if (rrc > 0) {

	  int i;
	  for (i = 0; i < rrc; i++) {
	    int c = ibuf[i];

	    if (breakchars != NULL) {
	      /* Check if a break character occurred */
	      int j;
	      for (j = 0; j < strlen(breakchars); j++) {
		if (c == breakchars[j]) {
		  return SER_OK;
		}
	      }
	    }

	    if (c != 0) {
	      str_appendc(s, c);
	      chars_read++;

#ifdef DEBUG
	      printf("serial_readline: current line='%s'\n", str_getbuf(s));
#endif

	      if (chars_max > 0 && chars_read == chars_max) {
		return SER_OK;
	      } 
/* 	    } else { */
/* 	      printf("serial_readline: read character 0\n"); */
	    }
	  }
	  
	} else {

	  return SER_ERROR;
	  
	}
      }
    }
  }
}

int serial_readchars(int fd, string *s, int chars_max)
{
  return readline(fd, s, NULL, chars_max);
}

int serial_readline(int fd, string *s)
{
  return readline(fd, s, "\r\n", 0);
}
