#pragma once
template <bool is_const>
struct basic_logic_step;

using logic_step = basic_logic_step<false>;
using const_logic_step = basic_logic_step<true>;