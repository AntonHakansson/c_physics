#include "language_layer.h"

void _DebugLog(i32 flags, const char *file, int line, const char *format, ...) {
  // Log to stdout
  {
    const char *name = "Info";
    if (flags & Log_Error) {
      name = "Error";
    } else if (flags & Log_Warning) {
      name = "Warning";
    }
    fprintf(stdout, "%s (%s:%i) ", name, file, line);
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    fprintf(stdout, "%s", "\n");
  }
}

void _AssertFailure(const char *expression, int line, const char *file, int crash) {
  LogError("[Assertion Failure] Assertion of %s at %s:%i failed.", expression, file, line);
  if (crash) {
    fprintf(stdout, "[Assertion Failure] Assertion of %s at %s:%i failed, Trying to crash...",
            expression, file, line);
    *(volatile int *)0 = 0;
  }
}

u64 murmur64(void const *data, isize len) { return murmur64_seed(data, len, 0x9747b28c); }

u64 murmur64_seed(void const *data_, isize len, u64 seed) {
#if defined(GB_ARCH_64_BIT)
  u64 const m = 0xc6a4a7935bd1e995ULL;
  i32 const r = 47;

  u64 h = seed ^ (len * m);

  u64 const *data = (u64 const *)data_;
  u8 const *data2 = (u8 const *)data_;
  u64 const *end = data + (len / 8);

  while (data != end) {
    u64 k = *data++;

    k *= m;
    k ^= k >> r;
    k *= m;

    h ^= k;
    h *= m;
  }

  switch (len & 7) {
    case 7:
      h ^= (u64)(data2[6]) << 48;
    case 6:
      h ^= (u64)(data2[5]) << 40;
    case 5:
      h ^= (u64)(data2[4]) << 32;
    case 4:
      h ^= (u64)(data2[3]) << 24;
    case 3:
      h ^= (u64)(data2[2]) << 16;
    case 2:
      h ^= (u64)(data2[1]) << 8;
    case 1:
      h ^= (u64)(data2[0]);
      h *= m;
  };

  h ^= h >> r;
  h *= m;
  h ^= h >> r;

  return h;
#else
  u64 h;
  u32 const m = 0x5bd1e995;
  i32 const r = 24;

  u32 h1 = (u32)(seed) ^ (u32)(len);
  u32 h2 = (u32)(seed >> 32);

  u32 const *data = (u32 const *)data_;

  while (len >= 8) {
    u32 k1, k2;
    k1 = *data++;
    k1 *= m;
    k1 ^= k1 >> r;
    k1 *= m;
    h1 *= m;
    h1 ^= k1;
    len -= 4;

    k2 = *data++;
    k2 *= m;
    k2 ^= k2 >> r;
    k2 *= m;
    h2 *= m;
    h2 ^= k2;
    len -= 4;
  }

  if (len >= 4) {
    u32 k1 = *data++;
    k1 *= m;
    k1 ^= k1 >> r;
    k1 *= m;
    h1 *= m;
    h1 ^= k1;
    len -= 4;
  }

  switch (len) {
    case 3:
      h2 ^= ((u8 const *)data)[2] << 16;
    case 2:
      h2 ^= ((u8 const *)data)[1] << 8;
    case 1:
      h2 ^= ((u8 const *)data)[0] << 0;
      h2 *= m;
  };

  h1 ^= h2 >> 18;
  h1 *= m;
  h2 ^= h1 >> 22;
  h2 *= m;
  h1 ^= h2 >> 17;
  h1 *= m;
  h2 ^= h1 >> 19;
  h2 *= m;

  h = h1;
  h = (h << 32) | h2;

  return h;
#endif
}
