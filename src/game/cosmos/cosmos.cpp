#include <sol2/sol.hpp>

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

void cosmos::read_solvable_from(augs::cref_memory_stream& ss) {
	cosmic::change_solvable_significant(*this, [&](cosmos_solvable_significant& current_signi) { 
		augs::read_bytes(ss, current_signi);
		return changer_callback_result::REFRESH; 
	});
}

namespace augs {
	template <class Archive>
	void write_object_bytes(Archive& into, const cosmos& cosm) {
		auto& profiler = cosm.profiler;

		if constexpr(can_reserve_v<Archive>) {
			augs::byte_counter_stream counter_stream;

			{
				auto scope = measure_scope(profiler.size_calculation_pass);
				augs::write_bytes(counter_stream, cosm.get_common_significant());
				augs::write_bytes(counter_stream, cosm.get_solvable().significant);
			}

			auto scope = measure_scope(profiler.memory_allocation_pass);
			
			into.reserve(into.get_write_pos() + counter_stream.size());
		}

		{
			auto scope = measure_scope(profiler.serialization_pass);
			augs::write_bytes(into, cosm.get_common_significant());
			augs::write_bytes(into, cosm.get_solvable().significant);
		}
	}

	template <class Archive>
	void read_object_bytes(Archive& from, cosmos& cosm) {
		cosmic::clear(cosm);

		auto& profiler = cosm.profiler;

		auto scope = measure_scope(profiler.deserialization_pass);

		cosm.change_common_significant([&](cosmos_common_significant& common) {
			augs::read_bytes(from, common);
			return changer_callback_result::DONT_REFRESH;
		});

		cosmic::change_solvable_significant(cosm, [&](cosmos_solvable_significant& significant) {
			augs::read_bytes(from, significant);
			return changer_callback_result::REFRESH;
		});
	}

	template <class Archive>
	void write_object_lua(Archive into, const cosmos& cosm) {
		{
			auto common_table = into.create();
			into["common"] = common_table;

			write_lua_table(common_table, cosm.get_common_significant());
		}
		
		{
			auto solvable_table = into.create();
			into["solvable"] = solvable_table;

			write_lua_table(solvable_table, cosm.get_solvable().significant);
		}
	}

	template <class Archive>
	void read_object_lua(Archive from, cosmos& cosm) {
		cosmic::clear(cosm);

		cosm.change_common_significant([&](cosmos_common_significant& common) {
			auto common_table = from["common"];

			if (common_table.valid()) {
				augs::read_lua_table(common_table, common);
			}

			return changer_callback_result::DONT_REFRESH;
		});

		cosmic::change_solvable_significant(cosm, [&](cosmos_solvable_significant& significant) {
			auto solvable_table = from["common"];

			if (solvable_table.valid()) {
				augs::read_lua_table(solvable_table, significant);
			}

			return changer_callback_result::REFRESH;
		});
	}
}

template void augs::write_object_bytes(augs::memory_stream&, const cosmos&);
template void augs::read_object_bytes(augs::memory_stream&, cosmos&);

template void augs::write_object_bytes(augs::ref_memory_stream&, const cosmos&);
template void augs::read_object_bytes(augs::cref_memory_stream&, cosmos&);

template void augs::write_object_bytes(std::ofstream&, const cosmos&);
template void augs::read_object_bytes(std::ifstream&, cosmos&);

template void augs::write_object_lua(sol::table, const cosmos&);
template void augs::read_object_lua(sol::table, cosmos&);

