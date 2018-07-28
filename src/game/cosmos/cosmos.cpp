#include <sol2/sol.hpp>

#include "augs/readwrite/memory_stream.h"

#include "augs/misc/randomization.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"

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

void cosmos::set(const cosmos_solvable_significant& signi) {
	cosmic::change_solvable_significant(*this, [&](cosmos_solvable_significant& s){ 
		{
			auto scope = measure_scope(profiler.duplication);
			s = signi; 
		}

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
		ensure(cosm.empty());

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
	void write_object_lua(Archive /* ar */, const cosmos& /* cosm */) {
#if TODO
		{
			auto common_table = ar.create();
			ar["common"] = common_table;

			write_lua(common_table, cosm.get_common_significant());
		}
		
		ensure(false);
		// TODO: write completely separate tables per each entity type?

		{
			auto pool_meta_table = ar.create();
			ar["pool_meta"] = pool_meta_table;
			pool_meta_table["reserved_entities_count"] = static_cast<unsigned>(cosm.get_solvable().get_maximum_entities());
		}

		auto entities_table = ar.create();
		ar["entities"] = entities_table;
		
		int entity_table_counter = 1;

		for (const auto& ent : cosm.get_solvable().get_entity_pool()) {
			auto this_entity_table = entities_table.create();
	
			ent.for_each_component(
				[&](const auto& comp) {
					using component_type = remove_cref<decltype(comp)>;

					const auto this_component_name = get_type_name_strip_namespace<component_type>();

					auto this_component_table = this_entity_table.create();
					this_entity_table[this_component_name] = this_component_table;

					write_lua(this_component_table, comp);
				},
				cosm.get_solvable()
			);

			entities_table[entity_table_counter++] = this_entity_table;
		}
#endif
	}

	template <class Archive>
	void read_object_lua(Archive /* ar */, cosmos& /* cosm */) {
		ensure(false);
#if TODO
		ensure(cosm.empty());

		/* TODO: Fix it to use tuples of initial values when creating entities */
		/* TODO: Fix it to read guids properly instead of entity ids */
		auto refresh_when_done = augs::scope_guard([&cosm]() {
			cosm.reinfer_solvable();
		});

		{
			sol::object reserved_count = ar["pool_meta"]["reserved_entities_count"];
			cosm.reserve_storage_for_entities(reserved_count.as<unsigned>());
		}

		cosm.change_common_significant([&](cosmos_common_significant& common) {
			read_lua(ar["common"], common);

			return changer_callback_result::DONT_REFRESH;
		});

		int entity_table_counter = 1;
		auto entities_table = ar["entities"];

		while (true) {
			sol::table maybe_next_entity = entities_table[entity_table_counter];

			if (maybe_next_entity.valid()) {
				const auto new_entity = cosmic::create_entity(cosm);

				for (auto key_value_pair : maybe_next_entity) {
					const auto component_name = key_value_pair.first.as<std::string>();

					for_each_component_type(
						[&](auto c) {
							using component_type = decltype(c);

							const auto this_component_name = get_type_name_strip_namespace<component_type>();

							if (this_component_name == component_name) {
								component_type c;
								read_lua(key_value_pair.second, c);
								new_entity.get({}).add<component_type>(c, cosm.get_solvable());
							}
						}
					);
				}
			}
			else {
				break;
			};

			++entity_table_counter;
		}
#endif
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

