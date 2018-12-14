#pragma once
#include "application/arena/arena_handle.h"

using server_step_type = uint32_t;

template <bool C>
using online_arena_handle = basic_arena_handle<C, online_mode_and_rules>;
