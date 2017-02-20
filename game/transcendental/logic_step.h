#pragma once
#include "augs/misc/delta.h"
#include "game/transcendental/cosmic_entropy.h"
#include "game/transcendental/entity_handle_declaration.h"

class cosmos;
struct data_living_one_step;

class config_lua_table;

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

template <bool is_const>
class basic_logic_step : public basic_cosmic_step<is_const> {
	friend class cosmos;
	typedef maybe_const_ref_t<is_const, data_living_one_step> data_living_one_step_ref;
public:
	data_living_one_step_ref transient;
	const cosmic_entropy& entropy;

	basic_logic_step(
		cosmos_ref cosm,
		const cosmic_entropy& entropy,
		data_living_one_step_ref transient
	);

	augs::delta get_delta() const;

	operator basic_logic_step<true>() const;
};

typedef basic_logic_step<false> logic_step;
typedef basic_logic_step<true> const_logic_step;