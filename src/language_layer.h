#pragma once

#ifdef DEVELOPER
#  define DEVELOPER 1
#else
#  define DEVELOPER 0
#endif

// C Standard Library
//-----------------------------------------------
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <limits>

// Naming convention
//-----------------------------------------------
#define MemoryCopy memcpy
#define MemoryMove memmove
#define MemorySet memset
#define CalculateCStringLength (u32) strlen
#define FMod fmodf
#define AbsoluteValue fabsf
#define SquareRoot sqrtf
#define Sin sinf
#define Cos cosf
#define Tan tanf
#define Min(a, b) (a < b ? a : b)
#define Max(a, b) (a >= b ? a : b)
#define Sign(a) (a > 0 ? 1 : -1)
#define CStringToI32(s) ((i32)atoi(s))
#define CStringToI16(s) ((i16)atoi(s))
#define CStringToF32(s) ((f32)atof(s))

// Helper Macros
//-----------------------------------------------
#define global static
#define internal static
#define local_persist static
#define ArrayCount(a) (sizeof(a) / sizeof((a)[0]))
#define Bytes(n) (n)
#define Kilobytes(n) (n << 10)
#define Megabytes(n) (n << 20)
#define Gigabytes(n) (((u64)n) << 30)
#ifndef PI
#  define PI (3.1415926535897f)
#endif

// Base Types
//-----------------------------------------------
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef i8 s8;
typedef i16 s16;
typedef i32 s32;
typedef i64 s64;
typedef i8 b8;
typedef i16 b16;
typedef i32 b32;
typedef i64 b64;
typedef float f32;
typedef double f64;
// NOTE(anton): hmm these are unsigned/signed long respectively, maybe a bit to excessive...
typedef size_t usize;
typedef std::ptrdiff_t isize;

#define F32_Max (std::numeric_limits<f32>::max())
template <typename T> inline void Swap(T &a, T &b) {
  T tmp = a;
  a = b;
  b = tmp;
}

// Assertions
//-----------------------------------------------
#undef Assert
#define AssertStatement HardAssert
#define Assert HardAssert
#define HardAssert(b)                            \
  do {                                           \
    if (!(b)) {                                  \
      _AssertFailure(#b, __LINE__, __FILE__, 1); \
    }                                            \
  } while (0)

#define SoftAssert(b)                            \
  do {                                           \
    if (!(b)) {                                  \
      _AssertFailure(#b, __LINE__, __FILE__, 0); \
    }                                            \
  } while (0)

void _AssertFailure(const char *expression, int line, const char *file, int crash);
void _DebugLog(i32 flags, const char *file, int line, const char *format, ...);

// Logging
//-----------------------------------------------
#define Log(...) printf(__VA_ARGS__)
#define LogWarning(...) _DebugLog(Log_Warning, __FILE__, __LINE__, __VA_ARGS__)
#define LogError(...) _DebugLog(Log_Error, __FILE__, __LINE__, __VA_ARGS__)

#define Log_Info (1 << 0)
#define Log_Warning (1 << 1)
#define Log_Error (1 << 2)

// Defer statements
//-----------------------------------------------
namespace {
  template <typename T> struct gbRemoveReference { typedef T Type; };
  template <typename T> struct gbRemoveReference<T &> { typedef T Type; };
  template <typename T> struct gbRemoveReference<T &&> { typedef T Type; };

  template <typename T> inline T &&gb_forward(typename gbRemoveReference<T>::Type &t) {
    return static_cast<T &&>(t);
  }
  template <typename T> inline T &&gb_forward(typename gbRemoveReference<T>::Type &&t) {
    return static_cast<T &&>(t);
  }
  template <typename T> inline T &&gb_move(T &&t) {
    return static_cast<typename gbRemoveReference<T>::Type &&>(t);
  }
  template <typename F> struct gbprivDefer {
    F f;
    gbprivDefer(F &&f) : f(gb_forward<F>(f)) {}
    ~gbprivDefer() { f(); }
  };
  template <typename F> gbprivDefer<F> gb__defer_func(F &&f) {
    return gbprivDefer<F>(gb_forward<F>(f));
  }

#define GB_DEFER_1(x, y) x##y
#define GB_DEFER_2(x, y) GB_DEFER_1(x, y)
#define GB_DEFER_3(x) GB_DEFER_2(x, __COUNTER__)
#define Defer(code) auto GB_DEFER_3(_defer_) = gb__defer_func([&]() -> void { code; })
}  // namespace

// Raylib cpp
//-----------------------------------------------
#ifdef RAYLIB_H
#  define v2 Vector2
#  define v3 Vector3

inline v2 operator+(v2 a, v2 b) { return Vector2Add(a, b); }
inline v2 operator-(v2 a, v2 b) { return Vector2Subtract(a, b); }
inline v2 operator*(v2 a, float b) { return Vector2Scale(a, b); }
inline v2 operator*(v2 a, v2 b) { return Vector2MultiplyV(a, b); }
inline void operator+=(v2 &a, v2 b) { a = a + b; }
inline void operator-=(v2 &a, v2 b) { a = a - b; }

inline v2 Vector2Abs(v2 a) { return (v2){AbsoluteValue(a.x), AbsoluteValue(a.y)}; }
inline f32 Vector2Cross(v2 a, v2 b) { return a.x * b.y - a.y * b.x; }
inline v2 Vector2Cross(v2 a, f32 s) { return (v2){s * a.y, -s * a.x}; }
inline v2 Vector2Cross(f32 s, v2 a) { return (v2){-s * a.y, s * a.x}; }
#endif

// Math
//-----------------------------------------------
struct Matrix2x2 {
  union {
    struct {
      f32 m0;
      f32 m1;
      f32 m2;
      f32 m3;
    };
    struct {
      v2 col1;
      v2 col2;
    };
  };
};

Matrix2x2 Matrix2x2FromAngle(f32 angle) {
  Matrix2x2 m;

  f32 c = Cos(angle);
  f32 s = Sin(angle);
  m.m0 = c;
  m.m1 = s;
  m.m2 = -s;
  m.m3 = c;

  return m;
}

Matrix2x2 Matrix2x2Transpose(Matrix2x2 m) {
  Swap(m.m1, m.m2);
  return m;
}

Matrix2x2 Matrix2x2Invert(Matrix2x2 m) {
  Matrix2x2 result;

  f32 det = m.m0 * m.m3 - m.m2 * m.m1;
  Assert(det != 0.0f);
  det = 1.0f / det;

  result.m0 = det * m.m3;
  result.m1 = -det * m.m1;
  result.m2 = -det * m.m2;
  result.m3 = det * m.m0;

  return result;
}

Matrix2x2 Matrix2x2Abs(Matrix2x2 m) {
  m.m0 = AbsoluteValue(m.m0);
  m.m1 = AbsoluteValue(m.m1);
  m.m2 = AbsoluteValue(m.m2);
  m.m3 = AbsoluteValue(m.m3);

  return m;
}

inline Vector2 operator*(const Matrix2x2 &a, const Vector2 &v) {
  return {a.m0 * v.x + a.m2 * v.y, a.m1 * v.x + a.m3 * v.y};
}

Matrix2x2 operator+(const Matrix2x2 &a, const Matrix2x2 &b) {
  Matrix2x2 m;

  m.m0 = a.m0 + b.m0;
  m.m1 = a.m1 + b.m1;
  m.m2 = a.m2 + b.m2;
  m.m3 = a.m3 + b.m3;

  return m;
}

Matrix2x2 operator*(const Matrix2x2 &a, const Matrix2x2 &b) {
  Matrix2x2 m;

  // m.m0 = a.m0 * b.m0 + a.m2 * b.m1;
  // m.m1 = a.m1 * b.m0 + a.m3 * b.m1;
  // m.m2 = a.m0 * b.m2 + a.m2 * b.m3;
  // m.m3 = a.m1 * b.m2 + a.m3 * b.m3;
  m.col1 = a * b.col1;
  m.col2 = a * b.col2;

  return m;
}

// Remap input value within input range to output range
f32 Remap(f32 value, f32 input_start, f32 input_end, f32 output_start, f32 output_end) {
  return (value - input_start) / (input_end - input_start) * (output_end - output_start)
         + output_start;
}

Vector2 Vector2Remap(v2 value, v2 input_start, v2 input_end, v2 output_start, v2 output_end) {
  return {Remap(value.x, input_start.x, input_end.x, output_start.x, output_end.x),
          Remap(value.y, input_start.y, input_end.y, output_start.y, output_end.y)};
};

// Hashing functions
//-----------------------------------------------
u64 murmur64(void const *data, isize len);
u64 murmur64_seed(void const *data, isize len, u64 seed);

// Simple Hash table
//-----------------------------------------------
struct HashTableFindResult {
  isize hash_index;
  isize entry_prev;
  isize entry_index;
};

template <typename T> struct HashTableEntry {
  u64 key;
  isize next;
  T value;
};

// NOTE(anton): size must be a power of two -> binary modulo
template <typename T, usize size> struct HashTable {
  isize hashes[size];

  HashTableEntry<T> entries[size];
  usize entries_count;

  usize mask;  // used for binary modulo
};

template <typename T, usize size> void HashTableInit(HashTable<T, size> *table) {
  table->mask = size - 1;
  Assert((size & table->mask) == 0);  // size must be a power of two!

  HashTableClear(table);
}

template <typename T, usize size> void HashTableClear(HashTable<T, size> *table) {
  MemorySet(table->hashes, -1, sizeof(isize) * size);
  MemorySet(table->entries, 0, sizeof(HashTableEntry<T>) * table->entries_count);
  table->entries_count = 0;
}

template <typename T, usize size> isize HashTableAddEntry(HashTable<T, size> *table, u64 key) {
  HashTableEntry<T> e = {0};
  e.key = key;
  e.next = -1;

  isize index = table->entries_count;

  table->entries[index] = e;
  table->entries_count++;

  Assert(table->entries_count <= size);

  return index;
}

template <typename T, usize size>
HashTableFindResult HashTableFind(HashTable<T, size> *table, u64 key) {
  HashTableFindResult r = {-1, -1, -1};
  if (table->entries_count > 0) {
    r.hash_index = key & table->mask;
    r.entry_index = table->hashes[r.hash_index];
    while (r.entry_index >= 0) {
      if (table->entries[r.entry_index].key == key) {
        return r;
      }

      r.entry_prev = r.entry_index;
      r.entry_index = table->entries[r.entry_index].next;
    }
  }

  return r;
}

template <typename T, usize size> T *HashTableGet(HashTable<T, size> *table, u64 key) {
  isize index = HashTableFind(table, key).entry_index;
  if (index >= 0) {
    return &table->entries[index].value;
  }

  return nullptr;
}

template <typename T, usize size> void HashTableSet(HashTable<T, size> *table, u64 key, T value) {
  isize index;
  HashTableFindResult r = HashTableFind(table, key);

  if (r.entry_index >= 0) {
    index = r.entry_index;
  } else {
    index = HashTableAddEntry(table, key);
    if (r.entry_prev >= 0) {
      table->entries[r.entry_prev].next = index;
    } else {
      table->hashes[r.hash_index] = index;
    }
  }

  table->entries[index].value = value;
}

// Tagged Handle Resource pool
//-----------------------------------------------
// NOTE(anton): size must be a power of two -> binary modulo
// template <typename T> struct ResourceHandle {
//   u32 index;
//   u32 generation;
// };

// template <typename T, usize size> struct ResourcePool {
//   T slots[size];
//   usize slots_count;

//   u32 free_slots[size];
//   usize free_slots_count;

//   usize mask;  // used for binary modulo
// };

// template <typename T, usize size> void ResourcesPoolInit(ResourcePool<T, size> *r) {
//   MemorySet((void *)r, 0, sizeof(ResourcePool<T, size>));

//   r->mask = size - 1;
//   Assert((size & r->mask) == 0);  // size must be a power of two!

//   for (u32 i = 0; i < size; i++) {
//     r->free_slots[i] = i;
//   }
// }

// template <typename T, usize size>
// T *ResourcePoolGet(ResourcePool<T, size> *r, ResourceHandle<T> handle) {
//   Assert(handle.index < size);
//   T *slot = r->slots[handle.index];
// }
