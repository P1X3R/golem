#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "defs.h"

FORCE_INLINE uint64_t random_u64(void) {
  const uint64_t u1 = (uint64_t)(rand()) & 0xFFFF;
  const uint64_t u2 = (uint64_t)(rand()) & 0xFFFF;
  const uint64_t u3 = (uint64_t)(rand()) & 0xFFFF;
  const uint64_t u4 = (uint64_t)(rand()) & 0xFFFF;
  return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

FORCE_INLINE uint64_t now_ms(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}
