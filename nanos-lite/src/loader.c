#include "common.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

uintptr_t loader(_Protect *as, const char *filename) {
  // TODO();
  size_t size = get_ramdisk_size();
  void *buf = NULL;
  ramdisk_read(buf, 0, size);
  memcpy(DEFAULT_ENTRY, buf, size);
  return (uintptr_t)DEFAULT_ENTRY;
}
