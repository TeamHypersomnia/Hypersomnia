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
		const auto num_spawns = get_num_faction_spawns(in.cosm, faction);

		if (0 == num_spawns) {
			return;
		}

		auto normalize_spawn_index = [&]() {
			current_spawn = (current_spawn + 1) % num_spawns;
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

mode_player_id bomb_mode::add_player(input_type in, const entity_name_str& chosen_name) {
	auto& cosm = in.cosm;
	(void)cosm;

	const auto new_id = first_free_key(players);
	players.try_emplace(new_id, chosen_name);

	return mode_player_id::dead();
}

void bomb_mode::remove_player(input_type in, const mode_player_id& id) {
	const auto guid = lookup(id);

	if (const auto handle = in.cosm[guid]) {
		cosmic::delete_entity(handle);
	}

	erase_element(players, id);
}

entity_guid bomb_mode::lookup(const mode_player_id& id) const {
	if (const auto entry = mapped_or_nullptr(players, id)) {
		return entry->guid;
	}

	return entity_guid::dead();
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
