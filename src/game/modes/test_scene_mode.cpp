#include "game/cosmos/solvers/standard_solver.h"
#include "game/modes/test_scene_mode.h"
#include "game/modes/mode_entropy.h"
#include "game/modes/mode_helpers.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/create_entity.hpp"
#include "game/detail/inventory/generate_equipment.h"

using input_type = test_scene_mode::input;

entity_guid test_scene_mode::lookup(const mode_player_id& id) const {
	if (const auto entry = mapped_or_nullptr(players, id)) {
		return entry->guid;
	}

	return entity_guid::dead();
}

mode_player_id test_scene_mode::lookup(const entity_guid& guid) const {
	for (const auto& p : players) {
		if (p.second.guid == guid) {
			return p.first;
		}
	}

	return mode_player_id::dead();
}

void test_scene_mode::init_spawned(const input in, const entity_id id, const logic_step step) {
	const auto handle = in.cosm[id];

	handle.dispatch_on_having_all<components::sentience>([&](const auto typed_handle) {
		in.vars.initial_eq.generate_for(typed_handle, step);

		auto& sentience = typed_handle.template get<components::sentience>();

		for_each_through_std_get(sentience.meters, [](auto& m) { m.make_full(); });

		fill_range(sentience.learnt_spells, true);
	});
}

void test_scene_mode::teleport_to_next_spawn(const input in, const entity_id id) {
	const auto handle = in.cosm[id];

	handle.dispatch_on_having_all<components::sentience>([&](const auto typed_handle) {
		const auto faction = typed_handle.get_official_faction();
		const auto num_spawns = get_num_faction_spawns(in.cosm, faction);

		if (0 == num_spawns) {
			return;
		}

		auto normalize_spawn_index = [&]() {
			current_spawn_index = (current_spawn_index + 1) % num_spawns;
		};

		normalize_spawn_index();

		if (const auto spawn = ::find_faction_spawn(in.cosm, faction, current_spawn_index)) {
			const auto spawn_transform = spawn.get_logic_transform();
			typed_handle.set_logic_transform(spawn_transform);

			if (const auto crosshair = typed_handle.find_crosshair()) {
				crosshair->base_offset = vec2::from_degrees(spawn_transform.rotation) * 200;
			}

			normalize_spawn_index();
		}
	});
}

mode_player_id test_scene_mode::add_player(input_type in, const faction_type faction) {
	auto& cosm = in.cosm;

	if (const auto flavour = ::find_faction_character_flavour(cosm, faction); flavour.is_set()) {
		const auto new_id = first_free_key(players, mode_player_id::first());

		if (cosmic::create_entity(
			cosm, 
			entity_flavour_id(flavour), 
			[&](const auto new_character, auto&&...) {
				teleport_to_next_spawn(in, new_character);
				pending_inits.push_back(new_character.get_guid());

				cosmic::set_specific_name(new_character, "Player");

				players.try_emplace(new_id, new_character.get_guid());
			},
			[](auto&&...) {}
		)) {
			return new_id;
		}
	}

	return mode_player_id::dead();
}

void test_scene_mode::remove_player(input_type in, const mode_player_id id) {
	const auto guid = lookup(id);
	cosmic::delete_entity(in.cosm[guid]);

	erase_element(players, id);
}

void test_scene_mode::mode_pre_solve(input_type in, const mode_entropy& entropy, logic_step step) {
	(void)entropy;
	auto& cosm = in.cosm;

	if (players.empty()) {
		auto n = in.vars.spawned_chars;

		while (n--) {
			add_player(in, in.vars.spawned_faction);
		}
	}

	const auto& clk = cosm.get_clock();

	for (const auto& p : pending_inits) {
		if (const auto handle = cosm[p]) {
			init_spawned(in, handle, step);
		}
	}

	pending_inits.clear();

	cosm.for_each_having<components::sentience>([&](const auto typed_handle) {
		auto& sentience = typed_handle.template get<components::sentience>();

		if (sentience.when_knocked_out.was_set() && clk.is_ready(
			in.vars.respawn_after_ms,
			sentience.when_knocked_out
		)) {
			teleport_to_next_spawn(in, typed_handle);
			init_spawned(in, typed_handle, step);

			sentience.when_knocked_out = {};
		}
	});
}
