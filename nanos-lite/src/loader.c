#include "common.h"

#define DEFAULT_ENTRY ((void *)0x8048000)

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();
extern int fs_open(const char *pathname, int flags, mode_t mode);
extern ssize_t fs_read(int fd, void *buf, size_t count);
extern int fs_close(int fd);
extern size_t fs_sizez(int fd);
extern void* new_page(void);

uintptr_t loader(_Protect *as, const char *filename) {
  // TODO();
  // size_t size = get_ramdisk_size();
  // void *buf = NULL;
  // ramdisk_read(buf, 0, size);
  // memcpy(DEFAULT_ENTRY, buf, size);
  // return (uintptr_t)DEFAULT_ENTRY;
  int fd = fs_open(filename, 0, 0);
  int fs_size = fs_sizez(fd);

  Log("Loading file %s [fd: %d] with size %d bytes", filename, fd, fs_size);

  /* PA3 legacy */
  // if (fs_size > 0) {
  //   fs_read(fd, DEFAULT_ENTRY, fs_size);
  //   fs_close(fd);
  // }
  /* PA3 legacy */

  uint32_t i = 0;
  void *pageptr;
  for(i = 0; i < fs_sizez(fd); i += 0x1000) {
    pageptr = new_page();
    _map(as, (void *)(0x8048000 + i), pageptr);
    fs_read(fd, pageptr, ((fs_sizez(fd) - i >= 0x1000) ? 0x1000 : fs_sizez(fd) - i));
  }
  fs_close(fd);
  new_page();
  return (uintptr_t)DEFAULT_ENTRY;
}
