#pragma once

#include "language_layer.h"

#define M_ARENA_MAX Gigabytes(4)
#define M_ARENA_COMMIT_SIZE Kilobytes(4)

struct MemoryArena {
  void *base;
  u64 max;
  u64 alloc_position;
  u64 commit_position;
};
