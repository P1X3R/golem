#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "defs.h"

FORCE_INLINE uint64_t random_u64() {
  const uint64_t u1 = (uint64_t)(rand()) & 0xFFFF;
  const uint64_t u2 = (uint64_t)(rand()) & 0xFFFF;
  const uint64_t u3 = (uint64_t)(rand()) & 0xFFFF;
  const uint64_t u4 = (uint64_t)(rand()) & 0xFFFF;
  return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}
