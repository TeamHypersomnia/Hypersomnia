#pragma once
#ifdef PLATFORM_WINDOWS
#define EMPTY_BASES __declspec(empty_bases)
#else
#define EMPTY_BASES
#endif