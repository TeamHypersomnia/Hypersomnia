#pragma once

#ifdef __clang__
template <class H>
struct hash;
#else
namespace std {
	template <class H>
	struct hash;
}
#endif
