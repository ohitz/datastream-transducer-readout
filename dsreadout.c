/*
 * Copyright (C) 2007-2015 Oliver Hitz <oliver@net-track.ch>
 */

#include <getopt.h>
#include <termios.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include <sys/time.h>
#include <sys/select.h>
#include <sys/ioctl.h>

#include "serial.h"
#include "string.h"
#include "transducer.h"

char *version = "version 0.2";
char *progname;

static struct option long_options[] = {
  { "verbose",     0, NULL, 'V' },
  { "help",        0, NULL, 'h' },
  { "version",     0, NULL, 'v' },
  { "device",      1, NULL, 'd' },
  { "identify",    1, NULL, 'i' },
  { "read",        1, NULL, 'r' },
  { "clear",       1, NULL, 'c' },
  { "set-address", 1, NULL, 'a' },
  { "scan",        0, NULL, 'S' },
  { "reset",       0, NULL, 'R' },
  { "force",       0, NULL, 'f' },
};

/**
 * Shows extended help.
 */
void help()
{ 
  printf("Datastream energy transducer tool\n");
  printf("%s [-h|--help]\n", progname);
  printf("    Print help message.\n");
  printf("%s [-v|--version]\n", progname);
  printf("    Show version.\n");
  printf("%s [-d|--device device] [-i|--identify address]\n", progname);
  printf("    Identify transducer.\n");
  printf("%s [-d|--device device] [-r|--read address]\n", progname);
  printf("    Show current values.\n");
  printf("%s [-d|--device device] [-r|--clear address]\n", progname);
  printf("    Clear energy totalizer.\n");
  printf("%s [-d|--device device] [--scan]\n", progname);
  printf("    Scans all 256 addresses for transducers. Very slow!\n");
  printf("%s [-d|--device device] [--set-address address]\n", progname);
  printf("    Reset the transducer and set the transducer address. USE WITH CARE!\n");
  printf("%s [-d|--device device] [--reset]\n", progname);
  printf("    Reset the transducer to its factory defaults. USE WITH CARE!\n");
}

/**
 * Shows simple usage.
 */
void usage()
{
  fprintf(stderr, "%s [-h|--help] [-v|--version] [-d|--device device] options\n", progname);
  exit(EXIT_FAILURE);
}

/**
 * Warn the user before committing a really, really stupid operation.
 */
void warnforce()
{
  fprintf(stderr, "WARNING! THIS IS A DANGEROUS OPERATION!\n");
  fprintf(stderr, "This operation will cause bus conflicts if your transducers are networked.\n");
  fprintf(stderr, "If you are sure what you are doing, add the option '--force'.\n");
  exit(EXIT_FAILURE);
}

/**
 * Identify transducer model.
 */
int action_identify(transducer *t, char *device, int address)
{
  int success = EXIT_FAILURE;

  if (TR_OK == tr_identify(t, address)) {
    printf("max_voltage: %d\n", t->transducers[address].max_volts);
    printf("max_current: %d\n", t->transducers[address].max_amps);
    success = EXIT_SUCCESS;
  } else {
    fprintf(stderr, "Unknown transducer model.\n");
  }

  return success;
}

/**
 * Read transducer data.
 */
int action_read(transducer *t, char *device, int address)
{
  int success = EXIT_FAILURE;

  if (TR_OK == tr_identify(t, address)) {
    if (TR_OK == tr_read(t, address) && TR_OK == tr_read_energy(t, address)) {
      printf("max_voltage: %d\n", t->transducers[address].max_volts);
      printf("max_current: %d\n", t->transducers[address].max_amps);
      if (t->transducers[address].type == TR_1PHASE) {
	printf("voltage: %f\n", t->transducers[address].voltage_f1 * t->transducers[address].max_volts);
	printf("current: %f\n", t->transducers[address].current_f1 * t->transducers[address].max_amps);
      } else if (t->transducers[address].type == TR_3PHASE4WIRE) {
	printf("voltage1: %f\n", t->transducers[address].voltage_f1 * t->transducers[address].max_volts);
	printf("current1: %f\n", t->transducers[address].current_f1 * t->transducers[address].max_amps);
	printf("voltage2: %f\n", t->transducers[address].voltage_f2 * t->transducers[address].max_volts);
	printf("current2: %f\n", t->transducers[address].current_f2 * t->transducers[address].max_amps);
	printf("voltage3: %f\n", t->transducers[address].voltage_f3 * t->transducers[address].max_volts);
	printf("current3: %f\n", t->transducers[address].current_f3 * t->transducers[address].max_amps);
      }
      printf("real_power: %f\n", t->transducers[address].power_f * t->transducers[address].max_volts * t->transducers[address].max_amps);
      printf("reactive_power: %f\n", t->transducers[address].vars_f * t->transducers[address].max_volts * t->transducers[address].max_amps);
      printf("frequency: %f\n", t->transducers[address].frequency);
      printf("kwhr: %f\n", (double) t->transducers[address].kwhr * t->transducers[address].max_volts * t->transducers[address].max_amps / 3600000.0);
      printf("kvarhr: %f\n", (double) t->transducers[address].kvarhr * t->transducers[address].max_volts * t->transducers[address].max_amps / 3600000.0);
      success = EXIT_SUCCESS;
    } else {
      fprintf(stderr, "Unable to read transducer.\n");
    }
  } else {
    fprintf(stderr, "Unknown transducer model.\n");
  }

  return success;
}

/**
 * Clear energy data.
 */
int action_clear_energy(transducer *t, char *device, int address)
{
  int success = EXIT_FAILURE;

  if (TR_OK == tr_identify(t, address)) {
    if (TR_OK == tr_clear_energy(t, address)) {
      success = EXIT_SUCCESS;
    } else {
      fprintf(stderr, "Unable to clear transducer energy values.\n");
    }
  } else {
    fprintf(stderr, "Unknown transducer model.\n");
  }

  return success;
}

/**
 * Main program.
 */
int main(int argc, char *argv[])
{
  transducer *t = NULL;
  char *device = NULL;
  int optc;

  int scan = 0;
  int identify = 0;
  int readvalues = 0;
  int clear = 0;
  int set_address = 0;
  int reset = 0;
  int force = 0;

  int address = -1;

  int success = EXIT_SUCCESS;

  progname = argv[0];

  while ((optc = getopt_long(argc, argv, "hvVd:i:r:c:", long_options, (int *) 0)) != EOF) {
    switch (optc) {
    case 'h':
      help();
      exit(EXIT_SUCCESS);
    case 'V':
      tr_set_verbose(1);
      break;
    case 'd':
      device = optarg;
      break;
    case 'S':
      scan = 1;
      break;
    case 'i':
      identify = 1;
      address = atoi(optarg);
      break;
    case 'r':
      readvalues = 1;
      address = atoi(optarg);
      break;
    case 'c':
      clear = 1;
      address = atoi(optarg);
      break;
    case 'a':
      set_address = 1;
      address = atoi(optarg);
      break;
    case 'R':
      reset = 1;
      break;
    case 'f':
      force = 1;
      break;
    case 'v':
      printf("%s\n", version);
      exit(EXIT_SUCCESS);      
    default:
      usage();
    }
  }

  if (device == NULL) {
    usage();
  }

  t = tr_alloc();

  /* Try to open device */
  if (TR_OK != tr_open(t, device)) {
    fprintf(stderr, "Unable to open device `%s'.\n", device);
    success = EXIT_FAILURE;
  } else {
    if (identify) {
      /* Identify a transducer. */
      success = action_identify(t, device, address);
    } else if (readvalues) {
      /* Get live values. */
      success = action_read(t, device, address);
    } else if (clear) {
      /* Clear energy totalizer. */
      success = action_clear_energy(t, device, address);
    } else if (scan) {
      printf("%d transducers found.\n", tr_scan(t));
    } else if (set_address) {
      /* Set transducer address. */
      if (!force) {
	warnforce();
      }
      tr_reset(t);
      sleep(10);
      tr_set_address(t, address);
      sleep(5);
    } else if (reset) {
      /* Reset a transducer. */
      if (!force) {
	warnforce();
      }
      tr_reset(t);
    } else {
      usage();
    }
    tr_close(t);
  }

  exit(success);
}

