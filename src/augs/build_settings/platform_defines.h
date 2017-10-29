#pragma once
#if PLATFORM_WINDOWS
#define FORCE_INLINE __forceinline
#define EMPTY_BASES
#elif PLATFORM_UNIX
#define FORCE_INLINE inline
#define EMPTY_BASES
#else
#error "Unsupported platform!"
#endif