#pragma once
#ifdef PLATFORM_WINDOWS
#define FORCE_INLINE __forceinline
#define EMPTY_BASES
#else
#define FORCE_INLINE
#define EMPTY_BASES
#endif