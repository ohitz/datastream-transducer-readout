
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/ioctl.h>

#include "debug.h"
#include "transducer.h"
#include "serial.h"
#include "string.h"

int verbose = 0;

void tr_set_verbose(int level)
{
  verbose = level;
}

transducer *tr_alloc()
{
  int i;
  transducer *t;

  t = (void *) malloc(sizeof(transducer));

  for (i = 0; i < 256; i++) {
    t->transducers[i].max_volts = 0;
    t->transducers[i].max_amps = 0;
  }
  return t;
}

void tr_free(transducer *t)
{
  free(t);
}

int tr_read(transducer *t, int n)
{
  int result = TR_OK;

  string *line = str_alloc(NULL, 80);
  string *cmd = str_alloc(NULL, 20);
  string *value;

  str_sprintf(cmd, 10, "#%02XA\r", n);

  serial_write(t->fd, cmd);

  if (SER_OK == serial_readline(t->fd, line)) {

    if (verbose > 0) {
      printf("Read all data returned '%s'\n", str_getbuf(line));
    }

    if (t->transducers[n].type == TR_1PHASE) {
      value = str_substring(line, 1, 7);
      t->transducers[n].voltage_f1 = atof(str_getbuf(value));
      str_free(value);

      value = str_substring(line, 8, 7);
      t->transducers[n].current_f1 = atof(str_getbuf(value));
      str_free(value);
      
      value = str_substring(line, 15, 7);
      t->transducers[n].power_f = atof(str_getbuf(value));
      str_free(value);
      
      value = str_substring(line, 22, 7);
      t->transducers[n].vars_f = atof(str_getbuf(value));
      str_free(value);
      
      value = str_substring(line, 29, 7);
      t->transducers[n].pfactor_f = atof(str_getbuf(value));
      str_free(value);
      
      value = str_substring(line, 36, 6);
      t->transducers[n].frequency = atof(str_getbuf(value));
      str_free(value);
    } else if (t->transducers[n].type == TR_3PHASE4WIRE) {
      value = str_substring(line, 1, 7);
      t->transducers[n].voltage_f1 = atof(str_getbuf(value));
      str_free(value);

      value = str_substring(line, 8, 7);
      t->transducers[n].current_f1 = atof(str_getbuf(value));
      str_free(value);

      value = str_substring(line, 15, 7);
      t->transducers[n].voltage_f2 = atof(str_getbuf(value));
      str_free(value);

      value = str_substring(line, 22, 7);
      t->transducers[n].current_f2 = atof(str_getbuf(value));
      str_free(value);

      value = str_substring(line, 29, 7);
      t->transducers[n].voltage_f3 = atof(str_getbuf(value));
      str_free(value);

      value = str_substring(line, 36, 7);
      t->transducers[n].current_f3 = atof(str_getbuf(value));
      str_free(value);

      value = str_substring(line, 43, 7);
      t->transducers[n].power_f = atof(str_getbuf(value));
      str_free(value);
      
      value = str_substring(line, 50, 7);
      t->transducers[n].vars_f = atof(str_getbuf(value));
      str_free(value);
      
      value = str_substring(line, 57, 7);
      t->transducers[n].pfactor_f = atof(str_getbuf(value));
      str_free(value);
      
      value = str_substring(line, 64, 6);
      t->transducers[n].frequency = atof(str_getbuf(value));
      str_free(value);
    }

  } else {
    result = TR_ERROR;
  }

  str_free(line);
  str_free(cmd);
  return result;
}

int tr_read_energy(transducer *t, int n)
{
  int result = TR_OK;

  string *line = str_alloc(NULL, 80);
  string *cmd = str_alloc(NULL, 20);
  string *value;

  str_sprintf(cmd, 10, "#%02XW\r", n);

  serial_write(t->fd, cmd);

  if (SER_OK == serial_readline(t->fd, line)) {

    if (19 != str_len(line)) {
      result = TR_ERROR;
    } else {
      /* Calculate checksum */
      int i;
      int checksum_calc = 0;
      int checksum_read;
      for (i = 0; i < 17; i++) {
	checksum_calc += str_getc(line, i);
      }
      checksum_calc &= 0xff;

      /* Parse data */
      value = str_substring(line, 1, 2);
      t->transducers[n].time_period = atoi(str_getbuf(value));
      str_free(value);
      
      value = str_substring(line, 3, 7);
      sscanf(str_getbuf(value), "%x", &t->transducers[n].kwhr);
      str_free(value);
      
      value = str_substring(line, 10, 7);
      sscanf(str_getbuf(value), "%x", &t->transducers[n].kvarhr);
      str_free(value);

      value = str_substring(line, 17, 2);
      sscanf(str_getbuf(value), "%x", &checksum_read);
      str_free(value);

      if (checksum_calc != checksum_read) {
	result = TR_ERROR;
      }
    }
  } else {
    result = TR_ERROR;
  }

  str_free(line);
  str_free(cmd);
  return result;
}

int htoi(const char *h)
{
  int result = 0;
  while (*h) {
    int c = *h;
    if (c >= '0' && c <= '9') {
      c = c-'0';
    } else if (c >= 'a' && c <= 'f') {
      c = 10+c-'a';
    } else if (c >= 'A' && c <= 'F') {
      c = 10+c-'A';
    } else {
      c = 0;
    }
    result = (result << 8) + c;
    h++;
  }
  return result;
}

int tr_clear_energy(transducer *t, int n)
{
  int result = TR_OK;

  string *line = str_alloc(NULL, 80);
  string *cmd = str_alloc(NULL, 20);
  string *value;

  str_sprintf(cmd, 10, "#%02XW\r", n);

  serial_write(t->fd, cmd);

  if (SER_OK == serial_readline(t->fd, line)) {
    /* Get time period */
    value = str_substring(line, 1, 2);
    t->transducers[n].time_period = htoi(str_getbuf(value));
    str_free(value);

    /* Construct and send clear command */
    str_sprintf(cmd, 10, "&%02X%02X\r", n, t->transducers[n].time_period);
    serial_write(t->fd, cmd);

    /* Expected result */
    str_sprintf(cmd, 10, "!%02X", n);

    str_clear(line);
    if (SER_OK == serial_readline(t->fd, line)) {
      if (0 != str_cmp(line, cmd)) {
	result = TR_ERROR;
      }
    } else {
      result = TR_ERROR;
    }
  } else {
    result = TR_ERROR;
  }

  str_free(line);
  str_free(cmd);
  return result;
}

int tr_identify(transducer *t, int n)
{
  string *model;
  
  string *line = str_alloc(NULL, 80);
  string *cmd = str_alloc(NULL, 20);

  str_sprintf(cmd, 10, "$%02XM\r", n);
    
  serial_write(t->fd, cmd);
  
  if (SER_OK == serial_readline(t->fd, line)) {

    if (verbose > 0) {
      printf("Read transducer name returned '%s'\n", str_getbuf(line));
    }
    
    /* First character has to be ! */
    if ('!' != str_getc(line, 0)) {
      return TR_UNKNOWN_MODEL;
    }
    
    /* Get model string */
    model = str_substring(line, 3, 0);
    
    if (0 == str_cmpb(model, "CRD5110-300-25")) {
      t->transducers[n].type = TR_1PHASE;
      t->transducers[n].max_volts = 300;
      t->transducers[n].max_amps = 25;
      return TR_OK;
    } else if (0 == str_cmpb(model, "CRD5170-300-5")) {
      t->transducers[n].type = TR_3PHASE4WIRE;
      t->transducers[n].max_volts = 300;
      t->transducers[n].max_amps = 5;
      return TR_OK;
    } else {
      return TR_UNKNOWN_MODEL;
    }
  } else {

    if (verbose > 0) {
      printf("Read transducer name returned nothing.\n");
    }

  }

  return TR_UNKNOWN_MODEL;
}

int tr_open(transducer *t, char *device)
{
  struct termios options;

  /* Try to open the file */
  t->fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (t->fd < 0) {
    return TR_DEVICE_OPEN;
  }

  /* Ensure we are the only process accessing this file */
  if (flock(t->fd, LOCK_EX | LOCK_NB) != 0) {
    close(t->fd);
    return TR_LOCK;
  }
 
  /* Get current comm parameters */
  tcgetattr(t->fd, &options);

  /* Set 9600 baud */
  cfsetispeed(&options, B9600);
  cfsetospeed(&options, B9600);

  options.c_cflag |= (CLOCAL | CREAD);
  options.c_cflag &= ~PARENB;
  options.c_cflag &= ~CSTOPB;
  options.c_cflag &= ~CSIZE;
  options.c_cflag |= CS8;
  options.c_cflag &= ~CRTSCTS;

  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

  options.c_iflag &= ~(IXON | IXOFF | IXANY);

  options.c_oflag &= ~OPOST;

  /* Probably not necessary, but well... */
  options.c_cc[VTIME] = 0;
  options.c_cc[VMIN] = 0;

  /* Flush buffers and set new options */
  tcflush(t->fd, TCIFLUSH);
  tcsetattr(t->fd, TCSANOW, &options);

  return TR_OK;
}

void tr_close(transducer *t)
{
  flock(t->fd, LOCK_UN);
  close(t->fd);
}

int tr_scan(transducer *t)
{
  int i;
  int num = 0;

  for (i = 0; i < 256; i++) {
    if (TR_OK == tr_identify(t, i)) {
      printf("%d: %d V, %d A\n", i, t->transducers[i].max_volts, t->transducers[i].max_amps);
      num++;
    }
  }

  return num;
}

int tr_reset(transducer *t)
{
  int success = 0;

  string *line = str_alloc(NULL, 40);

  serial_writeb(t->fd, "@CEAFW\r");
  if (SER_OK == serial_readchars(t->fd, line, 10)) {
    if (0x01 == str_getc(line, 0) &&
	0x06 == str_getc(line, 1)) {
      success = 1;
    }
  }

  str_free(line);
  return success;
}

int tr_set_address(transducer *t, int address)
{
  int success = 0;

  string *cmd = str_alloc(NULL, 40);
  string *line = str_alloc(NULL, 40);
  string *result = str_alloc(NULL, 40);

  /* Command */
  str_sprintf(cmd, 12, "%%01%02X000601\r", address);

  /* Expected result */
  str_sprintf(result, 10, "!%02X\r", address);

  serial_write(t->fd, cmd);
  if (SER_OK == serial_readline(t->fd, line)) {
    success = (0 == str_cmp(line, result));
  }
  str_free(cmd);
  str_free(line);
  str_free(result);

  return success;
}
