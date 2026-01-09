#pragma once

#include <stdint.h>

#include "defs.h"

extern uint64_t prng_state;

FORCE_INLINE uint64_t random_u64(void) {
  uint64_t z = (prng_state += 0x9e3779b97f4a7c15);
  z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
  z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
  return z ^ (z >> 31);
}

#if defined(_WIN32)
#include <malloc.h>
#include <windows.h>

static FORCE_INLINE uint64_t now_ms(void) {
  static LARGE_INTEGER freq;
  static int init = 0;
  if (!init) {
    QueryPerformanceFrequency(&freq);
    init = 1;
  }
  LARGE_INTEGER t;
  QueryPerformanceCounter(&t);
  return (uint64_t)(t.QuadPart * 1000 / freq.QuadPart);
}

FORCE_INLINE int aligned_alloc_64(void** ptr, const size_t size) {
  *ptr = _aligned_malloc(size, 64);
  return *ptr ? 0 : -1;
}

FORCE_INLINE void aligned_free(void* ptr) { _aligned_free(ptr); }

#else

#include <stdlib.h>
#include <time.h>

FORCE_INLINE uint64_t now_ms(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

FORCE_INLINE int aligned_alloc_64(void** ptr, const size_t size) {
  return posix_memalign(ptr, 64, size);
}
FORCE_INLINE void aligned_free(void* ptr) { free(ptr); }
#endif
