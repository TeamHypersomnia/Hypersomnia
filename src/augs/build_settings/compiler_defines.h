#pragma once
#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline
#define FORCE_NOINLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
#define FORCE_INLINE inline
#define FORCE_NOINLINE __attribute__ ((noinline))
#else
#error "Unsupported compiler!"
#endif
