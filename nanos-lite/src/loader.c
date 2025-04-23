#include "common.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();

uintptr_t loader(_Protect *as, const char *filename) {
  // TODO();
  size_t size = get_ramdisk_size();
  void *buf = NULL;
  ramdisk_read(buf, 0, size);
  memcpy(DEFAULT_ENTRY, buf, size);
  return (uintptr_t)DEFAULT_ENTRY;
}
