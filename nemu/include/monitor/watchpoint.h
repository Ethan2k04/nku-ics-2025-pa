#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO✔️: Add more members if necessary */
  char expr[32];
} WP;

int set_watchpoint(char *e);

bool delete_breakpoint(int);

#endif
