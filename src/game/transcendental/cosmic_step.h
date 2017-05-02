#pragma once
#include "augs/templates/maybe_const.h"

class cosmos;

template <bool is_const>
struct basic_cosmic_step {
	typedef maybe_const_ref_t<is_const, cosmos> cosmos_ref;
	cosmos_ref cosm;

	basic_cosmic_step(cosmos_ref cosm);
	cosmos_ref get_cosmos() const;
	operator basic_cosmic_step<true>() const;
};

typedef basic_cosmic_step<false> cosmic_step;
typedef basic_cosmic_step<true> const_cosmic_step;