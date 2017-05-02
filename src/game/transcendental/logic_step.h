#pragma once
#include "augs/misc/delta.h"

#include "game/transcendental/cosmic_step.h"
#include "game/transcendental/cosmic_entropy.h"

struct data_living_one_step;
struct all_logical_metas_of_assets;

struct logic_step_input {
	const cosmic_entropy& entropy;
	const all_logical_metas_of_assets& metas_of_assets;
};

template <bool is_const>
struct basic_logic_step : basic_cosmic_step<is_const> {
	typedef maybe_const_ref_t<is_const, data_living_one_step> data_living_one_step_ref;
	
	data_living_one_step_ref transient;
	const logic_step_input input;

	basic_logic_step(
		cosmos_ref cosm,
		const logic_step_input logic_step_input,
		data_living_one_step_ref transient
	);

	augs::delta get_delta() const;

	operator basic_logic_step<true>() const;
};

typedef basic_logic_step<false> logic_step;
typedef basic_logic_step<true> const_logic_step;