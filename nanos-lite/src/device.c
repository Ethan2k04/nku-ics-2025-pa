#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  size_t size;
  int key = _read_key();
  char buffer[128];
  bool keydown = false;
  if (key & 0x8000) {
    key ^= 0x8000;
    keydown = true;
  }
  if (key != _KEY_NONE) {
    sprintf(buffer, "k%s %s\n", keydown ? "d" : "u", keyname[key]);
    size = strlen(buffer) > len ? len : strlen(buffer);
    memcpy(buf, buffer, size);
    return size;
  }
  else {
    sprintf(buffer, "t %d\n", _uptime());
    size = strlen(buffer) > len ? len : strlen(buffer);
    memcpy(buf, buffer, size);
    return size;
  }
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
  memcpy(buf, dispinfo + offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
  int row = (offset / 4) / _screen.width;
  int col = (offset / 4) % _screen.width;
  _draw_rect((uint32_t*)buf, col, row, len / 4, 1);
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", _screen.width, _screen.height);
}
