#pragma once
#include "augs/misc/timing/delta.h"
#include "game/transcendental/cosmic_entropy.h"
#include "game/transcendental/step_declaration.h"

struct data_living_one_step;
struct all_logical_assets;

struct logic_step_input {
	const cosmic_entropy& entropy;
	const all_logical_assets& logical_assets;
};

template <bool is_const>
struct basic_logic_step {
	using data_living_one_step_ref = maybe_const_ref_t<is_const, data_living_one_step>;
	using cosmos_ref = maybe_const_ref_t<is_const, cosmos>;
	
	cosmos_ref cosm;
	data_living_one_step_ref transient;
	const logic_step_input input;

	basic_logic_step(
		cosmos_ref cosm,
		const logic_step_input logic_step_input,
		data_living_one_step_ref transient
	);

	cosmos_ref get_cosmos() const;

	augs::delta get_delta() const;

	operator const_logic_step() const;
};