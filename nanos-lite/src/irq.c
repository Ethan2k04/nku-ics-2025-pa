#include "common.h"

extern _RegSet* do_syscall(_RegSet *r);

static _RegSet* do_event(_Event e, _RegSet* r) {
  switch (e.event) {
    case _EVENT_SYSCALL: return do_syscall(r);
    case _EVENT_TRAP: { printf("Nanos-lite received _EVENT_TRAP but _umake() unimplemented."); return NULL;}
    default: panic("Unhandled event ID = %d", e.event);
  }

  return NULL;
}

void init_irq(void) {
  _asye_init(do_event);
}
