#include "augs/readwrite/memory_stream.h"

#include "augs/misc/randomization.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/create_entity.hpp"
#include "game/cosmos/change_common_significant.hpp"
#include "game/cosmos/change_solvable_significant.h"

#include "augs/readwrite/lua_readwrite.h"
#include "augs/readwrite/byte_readwrite.h"

cosmos::cosmos(const cosmic_pool_size_type reserved_entities) 
	: solvable(reserved_entities) 
{}

const cosmos cosmos::zero = {};

std::string cosmos::summary() const {
	return typesafe_sprintf("Entities: %x\n", get_entities_count());
}

rng_seed_type cosmos::get_rng_seed_for(const entity_id id) const {
	const auto passed = get_total_steps_passed();

	if (const auto handle = operator[](id)) {
		return augs::simple_two_hash(handle.get_guid(), passed);
	}

	return std::hash<std::remove_const_t<decltype(passed)>>()(passed);
}

randomization cosmos::get_rng_for(const entity_id id) const {
	return{ get_rng_seed_for(id) };
}

fast_randomization cosmos::get_fast_rng_for(const entity_id id) const {
	return{ get_rng_seed_for(id) };
}

bool cosmos::empty() const {
	return get_solvable().empty();
}

void cosmos::set(const cosmos_solvable_significant& new_signi) {
	cosmic::change_solvable_significant(*this, [&](cosmos_solvable_significant& current_signi){ 
		{
			auto scope = measure_scope(profiler.duplication);
			current_signi = new_signi; 
		}

		return changer_callback_result::REFRESH; 
	});
}

void cosmos::reinfer_everything() {
	common.reinfer();
	cosmic::reinfer_solvable(*this);
}

void cosmos::set_fixed_delta(const augs::delta& dt) {
	cosmic::change_solvable_significant(*this, [&](cosmos_solvable_significant& current_signi){ 
		ensure_eq(0, current_signi.clk.now.step);
		current_signi.clk.dt = dt;
		return changer_callback_result::DONT_REFRESH; 
	});
}
