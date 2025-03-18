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

WP* new_wp(char *e, bool *success) {
  if (free_->next == NULL) {
    printf("No free node in wp free list.\n");
    assert(0);
  }
  WP *wp = free_->next;
  wp->NO = wp_id++;
  free_->next = wp->next;
  wp->next = NULL;
  wp->old_val = wp->new_val = expr(e, success);
  if (*success == true) { strcpy(wp->expr, e); }
  else { printf("wp expression invalid.\n"); return NULL; }
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
  printf("No wp with NO %d found.\n", NO);
  return false;
}

/* TODO✔️: Implement the functionality of watchpoint */
int set_watchpoint(char *e) {
  bool success = true;
  WP* wp = new_wp(e, &success);
  if (success == true) { return wp->NO; }
  else { return -1; }
}

bool delete_breakpoint(int no){
  printf("wp with NO %d has been deleted.\n", no);
  return free_wp(no);
}

WP* scan_watchpoint() {
  WP *wp = head;
  while (wp) {
    bool success;
    wp->new_val = expr(wp->expr, &success);
    if (wp->old_val != wp->new_val) {
      return wp;
    }
    wp = wp->next;
  }
  return NULL;
}

static void watchpoint_display() {
  printf("=========WATCHPOINT INFO=========\n");
  printf("NO.\tExpr\tOld Val\tVal\n");
  WP* cur = head;
  while (cur){
    printf("\e[1;36m%d\e[0m\t\e[0;32m%s\e[0m\t\e[0;32m%d\e[0m\n", cur->NO, cur->expr, cur->old_val);
    cur = cur->next;
  }
}
