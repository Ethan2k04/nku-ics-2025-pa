#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO✔️: Add more members if necessary */
  char expr[32];
  uint32_t old_val, new_val;
} WP;

int set_watchpoint(char *e);

bool delete_breakpoint(int);

WP* scan_watchpoint();

static void watchpoint_display();

#endif
