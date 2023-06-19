#pragma once
#include "application/arena/arena_handle.h"
#include "game/modes/all_mode_includes.h"

using server_step_type = uint32_t;

template <bool C>
using online_arena_handle = basic_arena_handle<C, all_modes_variant, all_rulesets_variant>;
