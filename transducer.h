/*
 * Copyright (C) 2007-2015 Oliver Hitz <oliver@net-track.ch>
 */

#ifndef __TRANSDUCER_H
#define __TRANSDUCER_H

#include "string.h"

#define TR_OK 0
#define TR_DEVICE_OPEN -1
#define TR_UNKNOWN_MODEL -2
#define TR_ERROR -3
#define TR_LOCK -4

#define TR_1PHASE 0
#define TR_3PHASE3WIRE 1
#define TR_3PHASE4WIRE 2

struct transducer
{
  int fd;

  struct {
    int type;
    int max_volts;
    int max_amps;

    double voltage_f1;
    double voltage_f2;
    double voltage_f3;
    double current_f1;
    double current_f2;
    double current_f3;
    double power_f;
    double vars_f;
    double pfactor_f;
    double frequency;

    int time_period;
    int kwhr;
    int kvarhr;

  } transducers[256];
};

typedef struct transducer transducer;

transducer *tr_alloc();
int tr_open(transducer *t, char *device);
void tr_close(transducer *t);
int tr_identify(transducer *t, int n);
int tr_read(transducer *t, int n);
int tr_read_energy(transducer *t, int n);
int tr_clear_energy(transducer *t, int n);
void tr_free(transducer *t);
int tr_reset(transducer *t);
int tr_scan(transducer *t);
int tr_set_address(transducer *t, int address);
void tr_set_verbose(int level);

#endif /* __TRANSDUCER_H */
