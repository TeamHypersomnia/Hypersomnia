#include "game/cosmos/solvers/standard_solver.h"
#include "game/modes/test_scene_mode.h"
#include "game/modes/mode_entropy.h"
#include "game/modes/mode_helpers.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/create_entity.hpp"
#include "game/detail/inventory/generate_equipment.h"

using input_type = test_scene_mode::input;

void test_scene_mode::init_spawned(const input in, const entity_id id, const logic_step step) {
	const auto handle = in.cosm[id];

	handle.dispatch_on_having_all<components::sentience>([&](const auto typed_handle) {
		::generate_equipment(in.vars.initial_eq, typed_handle, step);

		auto& sentience = typed_handle.template get<components::sentience>();

		for_each_through_std_get(sentience.meters, [](auto& m) { m.make_full(); });

		fill_range(sentience.learned_spells, true);
	});
}

void test_scene_mode::teleport_to_next_spawn(const input in, const entity_id id) {
	const auto handle = in.cosm[id];

	handle.dispatch_on_having_all<components::sentience>([&](const auto typed_handle) {
		const auto faction = typed_handle.get_official_faction();

		if (const auto spawn = ::find_faction_spawn(in.cosm, faction, current_spawn_index)) {
			const auto spawn_transform = spawn.get_logic_transform();
			typed_handle.set_logic_transform(spawn_transform);

			if (const auto crosshair = typed_handle.find_crosshair()) {
				crosshair->base_offset = vec2::from_degrees(spawn_transform.rotation) * 200;
			}

			current_spawn_index = (current_spawn_index + 1) % get_num_faction_spawns(in.cosm, faction);
		}
	});
}

entity_guid test_scene_mode::add_player(input_type in, const faction_type faction) {
	auto& cosm = in.cosm;

	if (const auto flavour = ::find_faction_character_flavour(cosm, faction); flavour.is_set()) {
		return cosmic::create_entity(
			cosm, 
			entity_flavour_id(flavour), 
			[&](const auto new_character) {
				teleport_to_next_spawn(in, new_character);
				pending_inits.push_back(new_character.get_guid());
			},
			[](auto&&...) {}
		);
	}

	return entity_guid::dead();
}

void test_scene_mode::remove_player(input_type in, const entity_guid guid) {
	cosmic::delete_entity(in.cosm[guid]);
}

void test_scene_mode::mode_pre_solve(input_type in, const mode_entropy& entropy, logic_step step) {
	(void)entropy;
	auto& cosm = in.cosm;

	const auto now = cosm.get_timestamp();
	const auto dt = cosm.get_fixed_delta();

	for (const auto& p : pending_inits) {
		init_spawned(in, cosm[p], step);
	}

	pending_inits.clear();

	cosm.for_each_having<components::sentience>([&](const auto typed_handle) {
		auto& sentience = typed_handle.template get<components::sentience>();

		if (sentience.when_knocked_out.was_set() && augs::is_ready(
			in.vars.respawn_after_ms,
			sentience.when_knocked_out,
			now,
			dt
		)) {
			teleport_to_next_spawn(in, typed_handle);
			init_spawned(in, typed_handle, step);

			sentience.when_knocked_out = {};
		}
	});
}
