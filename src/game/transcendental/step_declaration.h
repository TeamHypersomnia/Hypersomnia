#pragma once
template <bool is_const>
struct basic_logic_step;

typedef basic_logic_step<false> logic_step;
typedef basic_logic_step<true> const_logic_step;