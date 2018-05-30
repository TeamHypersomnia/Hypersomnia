#pragma once
#include <cstddef>

#define augs_offsetof(s,m) ((std::size_t)&reinterpret_cast<char const volatile&>((((s*)0)->m)))

