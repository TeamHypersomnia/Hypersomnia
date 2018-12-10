#pragma once
#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline
#define FORCE_NOINLINE __declspec(noinline)
#define Likely(x)      (x)
#define Unlikely(x)    (x)
#elif defined(__GNUC__) || defined(__clang__)
#define FORCE_INLINE inline
#define FORCE_NOINLINE __attribute__ ((noinline))
#define Likely(x)      __builtin_expect(!!(x), 1)
#define Unlikely(x)    __builtin_expect(!!(x), 0)
#else
#error "Unsupported compiler!"
#endif
