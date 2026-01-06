#pragma once

#include <stdint.h>
#include <time.h>

#include "defs.h"

extern uint64_t prng_state;

FORCE_INLINE uint64_t random_u64(void) {
  uint64_t z = (prng_state += 0x9e3779b97f4a7c15);
  z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
  z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
  return z ^ (z >> 31);
}

FORCE_INLINE uint64_t now_ms(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}
