#pragma once
class cosmos;

#include "augs/templates/maybe_const.h"

template<bool is_const>
class basic_cosmic_step {
protected:
	typedef maybe_const_ref_t<is_const, cosmos> cosmos_ref;
public:
	cosmos_ref cosm;

	basic_cosmic_step(cosmos_ref cosm);
	cosmos_ref get_cosmos() const;
	operator basic_cosmic_step<true>() const;
};

typedef basic_cosmic_step<false> cosmic_step;
typedef basic_cosmic_step<true> const_cosmic_step;