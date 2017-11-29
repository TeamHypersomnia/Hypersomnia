#pragma once
#if PLATFORM_WINDOWS
#define FORCE_INLINE __forceinline
#elif PLATFORM_UNIX
#define FORCE_INLINE inline
#else
#error "Unsupported platform!"
#endif
