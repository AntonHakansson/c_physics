#include "memory.h"

internal MemoryArena MemoryArenaInitialize(void) {
  MemoryArena arena = {0};
  arena.max = M_ARENA_MAX;
  arena.base = malloc(arena.max);  // TODO(anton): anyway to reserve memory without committing?
  arena.alloc_position = 0;
  arena.commit_position = 0;
  return arena;
}

internal void *MemoryArenaPush(MemoryArena *arena, u64 size) {
  void *memory = 0;
  if (arena->alloc_position + size > arena->commit_position) {
    u64 commit_size = size;
    commit_size += M_ARENA_COMMIT_SIZE - 1;
    commit_size -= commit_size % M_ARENA_COMMIT_SIZE;
    // os->Commit((u8 *)arena->base + arena->commit_position, commit_size);
    arena->commit_position += commit_size;
  }
  memory = (u8 *)arena->base + arena->alloc_position;
  arena->alloc_position += size;
  return memory;
}

internal void *MemoryArenaPushZero(MemoryArena *arena, u64 size) {
  void *memory = MemoryArenaPush(arena, size);
  MemorySet(memory, 0, size);
  return memory;
}

internal void MemoryArenaPop(MemoryArena *arena, u64 size) {
  if (size > arena->alloc_position) {
    size = arena->alloc_position;
  }
  arena->alloc_position -= size;
}

internal void MemoryArenaClear(MemoryArena *arena) { MemoryArenaPop(arena, arena->alloc_position); }

internal void MemoryArenaRelease(MemoryArena *arena) { free(arena->base); }
