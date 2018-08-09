#include "game/cosmos/solvers/standard_solver.h"
#include "game/modes/bomb_mode.h"
#include "game/modes/mode_entropy.h"
#include "game/modes/mode_helpers.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/create_entity.hpp"
#include "game/detail/inventory/generate_equipment.h"

using input_type = bomb_mode::input;

void bomb_mode::init_spawned(const input in, const entity_id id, const logic_step step) {
	const auto handle = in.cosm[id];
	const auto& faction_vars = in.vars.factions[handle.get_official_faction()];

	handle.dispatch_on_having_all<components::sentience>([&](const auto typed_handle) {
		::generate_equipment(faction_vars.initial_eq, typed_handle, step);

		auto& sentience = typed_handle.template get<components::sentience>();

		for_each_through_std_get(sentience.meters, [](auto& m) { m.make_full(); });

		fill_range(sentience.learned_spells, true);
	});
}

void bomb_mode::teleport_to_next_spawn(const input in, const entity_id id) {
	const auto handle = in.cosm[id];
	auto& faction = factions[handle.get_official_faction()];
	auto& current_spawn = faction.current_spawn_index;

	handle.dispatch_on_having_all<components::sentience>([&](const auto typed_handle) {
		const auto faction = typed_handle.get_official_faction();

		auto normalize_spawn_index = [&]() {
			current_spawn = (current_spawn + 1) % get_num_faction_spawns(in.cosm, faction);
		};

		normalize_spawn_index();

		if (const auto spawn = ::find_faction_spawn(in.cosm, faction, current_spawn)) {
			const auto spawn_transform = spawn.get_logic_transform();
			typed_handle.set_logic_transform(spawn_transform);

			if (const auto crosshair = typed_handle.find_crosshair()) {
				crosshair->base_offset = vec2::from_degrees(spawn_transform.rotation) * 200;
			}

			normalize_spawn_index();
		}
	});
}

void bomb_mode::add_player(input_type in, const faction_type faction) {
	auto& cosm = in.cosm;

	if (const auto flavour = ::find_faction_character_flavour(cosm, faction); flavour.is_set()) {
		cosmic::create_entity(
			cosm, 
			entity_flavour_id(flavour), 
			[&](const auto new_character) {
				teleport_to_next_spawn(in, new_character);
				pending_inits.push_back(new_character.get_guid());
			},
			[](auto&&...) {}
		);
	}
}

void bomb_mode::remove_player(input_type in, const entity_guid guid) {
	cosmic::delete_entity(in.cosm[guid]);
}

void bomb_mode::mode_pre_solve(input_type in, const mode_entropy& entropy, logic_step step) {
	(void)entropy;
	auto& cosm = in.cosm;

	const auto now = cosm.get_timestamp();
	const auto dt = cosm.get_fixed_delta();

	(void)now;
	(void)dt;

	for (const auto& p : pending_inits) {
		if (const auto handle = cosm[p]) {
			init_spawned(in, handle, step);
		}
	}

	pending_inits.clear();
}
