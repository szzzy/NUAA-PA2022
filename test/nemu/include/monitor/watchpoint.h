#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char expr[32];
  uint32_t new_val;
  uint32_t old_val;

} WP;

WP* new_wp(char *);
void free_wp(WP *);
void list_watchpoint();
bool delete_watchpoint(int);
int set_watchpoint(char *);
WP* scan_watchpoint();
#endif