#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern void ramdisk_write(const void *buf, off_t offset, size_t len);
extern void dispinfo_read(void *buf, off_t offset, size_t len);
extern void fb_write(const void *buf, off_t offset, size_t len);

void init_fs() {
  // TODO: initialize the size of /dev/fb
  // a pixel has (R, G, B, A) 4 channels, so we should x4
  file_table[FD_FB].size = _screen.height * _screen.width * 4;
}

size_t fs_sizez(int fd) {
  return file_table[fd].size;
}

int fs_open(const char *pathname, int flags, mode_t mode) {
  int i;
  for (i = 0; i < NR_FILES; i++) {
    // return the first file that match the name.
    if (strcmp(file_table[i].name, pathname) == 0) {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  assert(0);
  return -1;
}

ssize_t fs_read(int fd, void *buf, size_t count) {
  ssize_t fs_size = fs_sizez(fd);
  if (file_table[fd].open_offset + count > fs_size) {
    count = fs_size - file_table[fd].open_offset;
  }
  switch (fd) {
    case FD_STDIN:
    case FD_STDOUT:
    case FD_STDERR:
    case FD_EVENTS:
      return 0;
    case FD_DISPINFO:
      dispinfo_read(buf, file_table[fd].open_offset, count);
      file_table[fd].open_offset += count;
      break;
    default:
      ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, count);
      file_table[fd].open_offset += count;
      break;
  }

  return count;
}

ssize_t fs_write(int fd, const void *buf, size_t count) {
  ssize_t fs_size = fs_sizez(fd);
  switch (fd) {
    case FD_STDOUT:
    case FD_STDERR:
      for (int i = 0; i < count; i++) {
        _putc(((char *)buf)[i]);
      }
      break;
    case FD_FB:
      fb_write(buf, file_table[fd].open_offset, count);
      file_table[fd].open_offset += count;
      break;
    default:
      if (file_table[fd].open_offset + count > fs_size) {
        count = fs_size - file_table[fd].open_offset;
      }
      ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, count);
      file_table[fd].open_offset += count;
      break;
  }

  return count;
}

off_t fs_lseek(int fd, off_t offset, int whence) {
  off_t res = -1;
  switch (whence) {
    case SEEK_SET:
      if (offset >= 0 && offset <= file_table[fd].size) {
        file_table[fd].open_offset = offset;
        res = file_table[fd].open_offset;
      }
      break;
    case SEEK_CUR:
      if ((file_table[fd].open_offset + offset >= 0) &&
          (file_table[fd].open_offset + offset <= file_table[fd].size)) {
          file_table[fd].open_offset += offset;
          res = file_table[fd].open_offset;
      }
      break;
    case SEEK_END:
      file_table[fd].open_offset = file_table[fd].size + offset;
      res = file_table[fd].open_offset;
      break;
  }

  return res;
}

int fs_close(int fd) {
  return 0;
}
