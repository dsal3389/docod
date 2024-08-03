#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

void fatal(const char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  vfprintf(stderr, fmt, va);
  va_end(va);
  exit(-1);
}
