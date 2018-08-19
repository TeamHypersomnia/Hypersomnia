#include "game/cosmos/solvers/standard_solver.h"
#include "game/modes/bomb_mode.h"
#include "game/modes/mode_entropy.h"
#include "game/modes/mode_helpers.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/create_entity.hpp"
#include "game/detail/inventory/generate_equipment.h"

using input_type = bomb_mode::input;

bomb_mode_player* bomb_mode::find(const mode_player_id& id) {
	return mapped_or_nullptr(players, id);
}

const bomb_mode_player* bomb_mode::find(const mode_player_id& id) const {
	return mapped_or_nullptr(players, id);
}

bool bomb_mode::choose_faction(const mode_player_id& id, const faction_type faction) {
	if (const auto entry = find(id)) {
		if (entry->faction == faction) {
			return true;
		}

		entry->faction = faction;
		return true;
	}

	return false;
}

faction_type bomb_mode::get_player_faction(const mode_player_id& id) const {
	if (const auto entry = find(id)) {
		return entry->faction;
	}

	return faction_type::NONE;
}

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
	auto& cosm = in.cosm;
	const auto handle = cosm[id];

	handle.dispatch_on_having_all<components::sentience>([&](const auto typed_handle) {
		const auto faction = typed_handle.get_official_faction();
		auto& faction_state = factions[faction];

		auto& spawns = faction_state.shuffled_spawns;
		auto& spawn_idx = faction_state.current_spawn_index;

		auto reshuffle = [&]() {
			reshuffle_spawns(cosm, faction);
		};

		if (spawns.empty()) {
			reshuffle();

			if (spawns.empty()) {
				return;
			}
		}

		spawn_idx %= spawns.size();

		const auto spawn = cosm[spawns[spawn_idx]];

		if (spawn.dead()) {
			reshuffle();
		}
		else {
			const auto spawn_transform = spawn.get_logic_transform();
			typed_handle.set_logic_transform(spawn_transform);

			if (const auto crosshair = typed_handle.find_crosshair()) {
				crosshair->base_offset = vec2::from_degrees(spawn_transform.rotation) * 200;
			}

			++spawn_idx;

			if (spawn_idx >= spawns.size()) {
				reshuffle();
			}
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
	if (const auto entry = find(id)) {
		return entry->guid;
	}

	return entity_guid::dead();
}

void bomb_mode::reshuffle_spawns(const cosmos& cosm, const faction_type faction) {
	auto rng = randomization(cosm.get_total_steps_passed() + static_cast<int>(faction));

	auto& faction_state = factions[faction];

	auto& spawns = faction_state.shuffled_spawns;
	const auto last_spawn = spawns.empty() ? entity_guid() : spawns.back();

	spawns.clear();

	auto adder = [&](const auto typed_spawn) {
		spawns.push_back(typed_spawn.get_guid());
		return callback_result::CONTINUE;
	};

	for_each_faction_spawn(cosm, faction, adder);

	shuffle_range(spawns, rng.generator);

	if (last_spawn.is_set() && spawns.size() > 1) {
		if (spawns.back() == last_spawn) {
			std::swap(spawns.front(), spawns.back());
		}
	}

	faction_state.current_spawn_index = 0;
}

std::size_t bomb_mode::num_players_in_faction(const faction_type type) const {
	auto for_each_player = [&](auto callback) {
		for (auto& it : players) {
			callback(it.first, it.second);
		}
	};

	std::size_t total = 0;

	for_each_player([&](const auto&, const auto& player_state) {
		if (player_state.faction == type) {
			++total;
		}
	});

	return total;
}

bool bomb_mode::auto_assign_faction(const cosmos& cosm, const mode_player_id& id) {
	if (const auto entry = find(id)) {
		auto& f = entry->faction;
		const auto previous_faction = f;
		f = faction_type::NONE;

		/* Now if factions were all even, it will assign to the same faction and return false for "no change" */
		f = ::calc_weakest_faction(cosm, [this](const faction_type ff) { return num_players_in_faction(ff); });

		const bool faction_changed = f != previous_faction;
		return faction_changed;
	}

	return false;
}

void bomb_mode::setup_round(input_type in, const logic_step step) {
	auto& cosm = in.cosm;
	cosm.set(in.initial_signi);

	for_each_faction([&](const auto faction) {
		reshuffle_spawns(cosm, faction);
	});

	auto create_player = [&](const auto faction) {
		if (const auto flavour = ::find_faction_character_flavour(cosm, faction); flavour.is_set()) {
			auto new_player = cosmic::create_entity(
				cosm, 
				entity_flavour_id(flavour), 
				[](auto&&...) {},
				[&](const auto new_character) {
					teleport_to_next_spawn(in, new_character);
					init_spawned(in, new_character, step);
				}
			);

			return new_player;
		}

		return cosm[entity_id()];
	};

	for (auto& it : players) {
		auto& player_data = it.second;

		if (const auto handle = create_player(player_data.faction)) {
			cosmic::set_specific_name(handle, player_data.chosen_name);
			
			if (in.vars.freeze_secs > 0.f) {
				handle.set_frozen(true);
			}

			player_data.guid = handle.get_guid();
		}
	}

	unfrozen_already = false;
}

void bomb_mode::mode_pre_solve(input_type in, const mode_entropy& entropy, const logic_step step) {
	auto& cosm = in.cosm;

	if (!unfrozen_already && get_freeze_seconds_left(in) <= 0.f) {
		for (const auto& it : players) {
			cosm[it.second.guid].set_frozen(false);
		}

		unfrozen_already = true;
	}

	if (start_scheduled || get_round_seconds_left(in) <= 0.f) {
		setup_round(in, step);
		start_scheduled = false;
	}

	(void)entropy;
}

float bomb_mode::get_total_seconds(const input_type in) const {
	const auto& start_clk = in.initial_signi.clk;
	const auto& clk = in.cosm.get_clock();

	return clk.diff_seconds(start_clk);
}

float bomb_mode::get_freeze_seconds_left(const input_type in) const {
	return in.vars.freeze_secs - get_total_seconds(in);
}

float bomb_mode::get_round_seconds_left(const input_type in) const {
	return in.vars.round_secs + in.vars.freeze_secs - get_total_seconds(in);
}

unsigned bomb_mode::get_round_num() const {
	unsigned total = 0;

	for_each_faction([&](const auto f) {
		total += factions[f].score;
	});

	return total;
}
