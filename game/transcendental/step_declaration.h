#pragma once
template<bool is_const>
class basic_cosmic_step;

typedef basic_cosmic_step<false> cosmic_step;
typedef basic_cosmic_step<true> const_cosmic_step;

template <bool is_const>
class basic_logic_step;

typedef basic_logic_step<false> logic_step;
typedef basic_logic_step<true> const_logic_step;

class viewing_step;