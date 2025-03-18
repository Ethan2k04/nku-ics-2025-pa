#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;
  head = NULL;
  free_ = wp_pool;
}

static int wp_id = 1;

static void insert_node(WP *wp, WP* ll) {
  wp->next = ll->next;
  ll->next = wp;
}

WP* new_wp(const char *e, bool *success) {
  if (free_->next == NULL) {
    printf("No free node in wp free list.");
    assert(0);
  }
  WP *wp = free_->next;
  wp->NO = wp_id++;
  free_->next = wp->next;
  wp->next = NULL;
  expr(e, success);
  if (*success == true) { strcpy(wp->expr, e); }
  else { printf("wp expression invalid."); return ; }
  if (head == NULL) { head = wp; }
  else { insert_node(wp, head); }
  return wp;
}

bool free_wp(int NO) {
  WP *tmp, *prev;
  if (head->NO == NO) {
    tmp = head->next;
    insert_node(head, free_);
    head = tmp;
    return true;
  }
  prev = head;
  while (prev->next) {
    if (prev->next->NO == NO) {
      tmp = prev->next->next;
      insert_node(prev->next, free_);
      prev->next = tmp;
      return true;
    }
    prev = prev->next;
  }
  printf("No watchpoint with NO %d found.", NO);
  return false;
}

/* TODO✔️: Implement the functionality of watchpoint */
int set_watchpoint(char *e) {
  bool *success = true;
  WP* wp = new_wp(e, success);
  if (*success == true) { return wp->NO; }
  else { return -1; }
}

bool delete_breakpoint(int no){
  return free_wp(no);
}
