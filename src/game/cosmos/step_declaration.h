#pragma once

template <bool is_const>
struct basic_logic_step_input;

template <bool is_const>
class basic_logic_step;

using logic_step = basic_logic_step<false>;
using const_logic_step = basic_logic_step<true>;

using logic_step_input = basic_logic_step_input<false>;
using const_logic_step_input = basic_logic_step_input<true>;