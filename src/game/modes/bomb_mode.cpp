#include "game/cosmos/solvers/standard_solver.h"
#include "game/modes/bomb_mode.h"
#include "game/modes/mode_entropy.h"
#include "game/modes/mode_helpers.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/create_entity.hpp"
#include "game/detail/inventory/generate_equipment.h"
#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"
#include "game/messages/start_sound_effect.h"

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

template <class F>
decltype(auto) bomb_mode::on_bomb_entity(const input_type in, F callback) const {
	auto& vars = in.vars;
	auto& cosm = in.cosm;

	const auto bomb_flavour = vars.bomb_flavour;
	const auto& flavours = cosm.get_solvable_inferred().name;

	return bomb_flavour.dispatch([&](const auto& typed_bomb_flavour_id) {
		const auto& bombs = flavours.get_entities_by_flavour_id(typed_bomb_flavour_id);

		if (bombs.size() == 1) {
			return callback(cosm[*bombs.begin()]);
		}
		else {
			return callback(std::nullopt);
		}
	});
}

bomb_mode::participating_factions bomb_mode::calc_participating_factions(const input_type in) const {
	const auto& cosm = in.cosm;
	const auto spawnable = calc_spawnable_factions(cosm);

	participating_factions output;

	cosm.template for_each_having<invariants::box_marker>([&](const auto& typed_handle) {
		const auto& marker = typed_handle.template get<invariants::box_marker>();

		if (::is_bombsite(marker.type)) {
			output.bombing = typed_handle.get_official_faction();
			return callback_result::ABORT;
		}

		return callback_result::CONTINUE;
	});

	for_each_faction([&](const faction_type t) {
		if (spawnable[t] && t != output.bombing) {
			output.defusing = t;
		}
	});

	return output;
}

faction_type bomb_mode::calc_weakest_faction(const input_type in) const {
	const auto participating = calc_participating_factions(in);

	struct {
		faction_type type;
		std::size_t count;
	} weakest { 
		participating.bombing,
		num_players_in(participating.bombing)
	};

	participating.for_each([&](const auto f) { 
		const auto n = num_players_in(f);

		if (n < weakest.count) {
			weakest = { f, n };
		}
	});

	return weakest.type;
}

faction_type bomb_mode::get_player_faction(const mode_player_id& id) const {
	if (const auto entry = find(id)) {
		return entry->faction;
	}

	return faction_type::NONE;
}

void bomb_mode::init_spawned(
	const input in, 
	const entity_id id, 
	const logic_step step,
	const bomb_mode::round_transferred_player* const transferred
) {
	auto& cosm = in.cosm;
	const auto handle = cosm[id];
	const auto& faction_vars = in.vars.factions[handle.get_official_faction()];

	handle.dispatch_on_having_all<components::sentience>([&](const auto typed_handle) {
		if (transferred != nullptr && transferred->survived) {
			const auto& eq = transferred->saved_eq;

			std::vector<entity_id> created_items;

			for (const auto& i : eq.items) {
				const auto& idx = i.container_index;
				const auto target_container = 
					idx < created_items.size() ? 
					created_items[idx] : 
					entity_id(typed_handle.get_id())
				;

				const auto target_slot = inventory_slot_id { i.slot_type, target_container };

				const auto new_item = cosmic::create_entity(
					cosm,
					i.flavour,
					[&](const auto&, auto& raw_entity) {
						auto& item = raw_entity.template get<components::item>();
						item.charges = i.charges;
						//item.current_slot = target_slot;
					},
					[&](const auto&) {

					}
				);

				const auto result = perform_transfer_no_step(item_slot_transfer_request::standard(new_item.get_id(), target_slot), cosm);
				result.notify_logical(step);
				created_items.push_back(new_item.get_id());
			}
		}
		else {
			::generate_equipment(faction_vars.initial_eq, typed_handle, step);
		}

		auto& sentience = typed_handle.template get<components::sentience>();

		for_each_through_std_get(sentience.meters, [](auto& m) { m.make_full(); });

		fill_range(sentience.learned_spells, true);

		if (transferred != nullptr) {
			typed_handle.template get<components::movement>().flags = transferred->movement;
		}
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

	const auto f = mode_player_id::first();
	const auto new_id = first_free_key(players, f.value);

	players.try_emplace(new_id, chosen_name);

	return new_id;
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

template <class F>
void bomb_mode::for_each_player_in(const faction_type faction, F callback) const {
	for (auto& it : players) {
		if (it.second.faction == faction) {
			callback(it.first, it.second);
		}
	}
}

template <class C, class F>
void bomb_mode::for_each_player_handle_in(C& cosm, const faction_type faction, F callback) const {
	for_each_player_in(faction, [&](const auto&, const auto& data) {
		if (const auto handle = cosm[data.guid]) {
			handle.template dispatch_on_having_all<components::sentience>([&](const auto& typed_player) {
				callback(typed_player);
			});
		}
	});
}

std::size_t bomb_mode::num_conscious_players_in(const cosmos& cosm, const faction_type faction) const {
	auto total = std::size_t(0);

	for_each_player_handle_in(cosm, faction, [&](const auto& handle) {
		if (handle.template get<components::sentience>().is_conscious()) {
			++total;
		}
	});

	return total;
}

std::size_t bomb_mode::num_players_in(const faction_type faction) const {
	auto total = std::size_t(0);

	for_each_player_in(faction, [&](auto&&...) {
		++total;
	});

	return total;
}

bool bomb_mode::auto_assign_faction(const input_type in, const mode_player_id& id) {
	if (const auto entry = find(id)) {
		auto& f = entry->faction;
		const auto previous_faction = f;
		f = faction_type::NONE;

		/* Now if factions were all even, it will assign to the same faction and return false for "no change" */
		f = calc_weakest_faction(in);

		const bool faction_changed = f != previous_faction;
		return faction_changed;
	}

	return false;
}

void bomb_mode::set_players_frozen(const input_type in, const bool flag) {
	if (cache_players_frozen == flag) {
		return;
	}

	cache_players_frozen = flag;

	for (auto& it : players) {
		auto& player_data = it.second;

		if (const auto handle = in.cosm[player_data.guid]) {
			handle.set_frozen(flag);
		}
	}
}

void bomb_mode::setup_round(
	const input_type in, 
	const logic_step step, 
	const bomb_mode::round_transferred_players& transfers
) {
	auto& cosm = in.cosm;
	clock_before_setup = cosm.get_clock();
	cosm.set(in.initial_signi);
	remove_test_characters(cosm);
	remove_test_dropped_items(cosm);

	cache_players_frozen = false;
	last_win = {};

	for_each_faction([&](const auto faction) {
		reshuffle_spawns(cosm, faction);
	});

	auto create_player = [&](const auto faction, const auto& id) {
		if (const auto flavour = ::find_faction_character_flavour(cosm, faction); flavour.is_set()) {
			auto new_player = cosmic::create_entity(
				cosm, 
				entity_flavour_id(flavour), 
				[](auto&&...) {},
				[&](const auto new_character) {
					teleport_to_next_spawn(in, new_character);
					init_spawned(in, new_character, step, mapped_or_nullptr(transfers, id));
				}
			);

			return new_player;
		}

		return cosm[entity_id()];
	};

	for (auto& it : players) {
		const auto id = it.first;
		auto& player_data = it.second;

		if (const auto handle = create_player(player_data.faction, id)) {
			cosmic::set_specific_name(handle, player_data.chosen_name);
			player_data.guid = handle.get_guid();
		}
	}

	if (in.vars.freeze_secs > 0.f) {
		if (state != arena_mode_state::WARMUP) {
			set_players_frozen(in, true);
		}
	}
	else {
		play_sound_for(in, step, battle_event::START);
	}
}

bomb_mode::round_transferred_players bomb_mode::make_transferred_players(const input_type in) const {
	round_transferred_players result;

	for (auto& it : players) {
		const auto id = it.first;
		auto& player_data = it.second;

		if (const auto handle = in.cosm[player_data.guid]) {
			auto& pm = result[id];
			pm.movement = handle.get<components::movement>().flags;

			if (handle.sentient_and_unconscious()) {
				continue;
			}

			auto& eq = pm.saved_eq;
			auto& items = eq.items;
			pm.survived = true;

			handle.for_each_contained_item_recursive(
				[&](const auto typed_item) {
					const auto flavour_id = typed_item.get_flavour_id();

					if (entity_flavour_id(flavour_id) == entity_flavour_id(in.vars.bomb_flavour)) {
						return recursive_callback_result::CONTINUE_DONT_RECURSE;
					}

					const auto slot = typed_item.get_current_slot();
					const auto container_id = slot.get_container().get_id();
					const auto container_index = mapped_or_default(
						eq.id_to_container_idx, 
						container_id, 
						static_cast<std::size_t>(-1)
					);
					
					if (container_index == static_cast<std::size_t>(-1)) {
						ensure_eq(handle.get_id(), container_id);
					}

					const auto charges = typed_item.template get<components::item>().get_charges();

					if (typed_item.template has<invariants::container>()) {
						const auto new_idx = eq.items.size();
						eq.id_to_container_idx.try_emplace(typed_item.get_id(), new_idx);
					}

					items.push_back({ flavour_id, charges, container_index, slot.get_type() });

					return recursive_callback_result::CONTINUE_AND_RECURSE;
				}
			);
		}
	}

	return result;
}

void bomb_mode::start_next_round(const input_type in, const logic_step step) {
	state = arena_mode_state::LIVE;
	setup_round(in, step, make_transferred_players(in));
}

void bomb_mode::make_win(const input_type in, const faction_type winner) {
	const auto p = calc_participating_factions(in);
	(void)p;

	++factions[winner].score;

	state = arena_mode_state::ROUND_END_DELAY;
	last_win = { in.cosm.get_clock(), winner };
}

void bomb_mode::play_faction_sound(const const_logic_step step, const faction_type f, const assets::sound_id id) const {
	sound_effect_input effect;
	effect.id = id;

	sound_effect_start_input input;
	input.listener_faction = f;
	input.variation_number = clock_before_setup.now.step + get_round_num();

	effect.start(step, input);
}

void bomb_mode::play_win_sound(const input_type in, const const_logic_step step, const faction_type winner) const {
	const auto p = calc_participating_factions(in);

	p.for_each([&](const faction_type t) {
		if (const auto sound_id = in.vars.win_sounds[t][winner]; sound_id.is_set()) {
			play_faction_sound(step, t, sound_id);
		}
	});
}

void bomb_mode::play_sound_for(const input_type in, const const_logic_step step, const battle_event event) const {
	const auto p = calc_participating_factions(in);

	p.for_each([&](const faction_type t) {
		play_faction_sound_for(in, step, event, t);
	});
}

void bomb_mode::play_faction_sound_for(const input_type in, const const_logic_step step, const battle_event event, const faction_type t) const {
	if (const auto sound_id = in.vars.event_sounds[t][event]; sound_id.is_set()) {
		play_faction_sound(step, t, sound_id);
	}
}

void bomb_mode::play_bomb_defused_sound(const input_type in, const const_logic_step step, const faction_type winner) const {
	const auto p = calc_participating_factions(in);

	p.for_each([&](const faction_type t) {
		messages::start_multi_sound_effect msg;

		{
			auto& start = msg.payload.start;

			start.listener_faction = t;
			start.variation_number = clock_before_setup.now.step + get_round_num();
		}

		auto& effects = msg.payload.inputs;
		effects.reserve(2);

		if (const auto defused_id = in.vars.event_sounds[t][battle_event::BOMB_DEFUSED]; defused_id.is_set()) {
			sound_effect_input effect;
			effect.id = defused_id;

			effects.emplace_back(std::move(effect));
		}

		if (const auto win_id = in.vars.win_sounds[t][winner]; win_id.is_set()) {
			sound_effect_input effect;
			effect.id = win_id;

			effects.emplace_back(std::move(effect));
		}

		step.post_message(msg);
	});
}

void bomb_mode::process_win_conditions(const input_type in, const logic_step step) {
	auto& cosm = in.cosm;

	const auto p = calc_participating_factions(in);

	auto standard_win = [&](const auto winner) {
		make_win(in, winner);
		play_win_sound(in, step, winner);
	};

	/* Bomb-based win-conditions */

	if (bomb_exploded(in)) {
		standard_win(p.bombing);
		return;
	}

	if (bomb_defused(in)) {
		const auto winner = p.defusing;
		make_win(in, winner);

		play_bomb_defused_sound(in, step, winner);
		return;
	}

	if (!bomb_planted(in) && get_round_seconds_left(in) <= 0.f) {
		/* Time-out */
		standard_win(p.defusing);
		return;
	}

	/* Kill-based win-conditions */

	if (num_players_in(p.bombing) > 0) {
		if (!bomb_planted(in) && 0 == num_conscious_players_in(cosm, p.bombing)) {
			/* All bombing players have been neutralized. */
			standard_win(p.defusing);
			return;
		}
	}

	if (num_players_in(p.defusing) > 0) {
		if (0 == num_conscious_players_in(cosm, p.defusing)) {
			/* All defusing players have been neutralized. */
			standard_win(p.bombing);
			return;
		}
	}
}

void bomb_mode::mode_pre_solve(const input_type in, const mode_entropy& entropy, const logic_step step) {
	(void)entropy;

	if (state == arena_mode_state::INIT) {
		if (in.vars.warmup_secs > 4) {
			state = arena_mode_state::WARMUP;
		}
		else {
			state = arena_mode_state::LIVE;
		}

		setup_round(in, step);
	}
	else if (state == arena_mode_state::WARMUP) {
		respawn_the_dead(in, step, in.vars.warmup_respawn_after_ms);

		if (get_warmup_seconds_left(in) <= 0.f) {
			set_players_frozen(in, true);

			if (get_match_begins_in_seconds(in) <= 0.f) {
				state = arena_mode_state::LIVE;
				setup_round(in, step);
			}
		}
	}
	else if (state == arena_mode_state::LIVE) {
		if (get_freeze_seconds_left(in) <= 0.f) {
			if (cache_players_frozen) {
				play_sound_for(in, step, battle_event::START);
			}

			set_players_frozen(in, false);
		}

		process_win_conditions(in, step);
	}
	else if (state == arena_mode_state::ROUND_END_DELAY) {
		if (get_round_end_seconds_left(in) <= 0.f) {
			start_next_round(in, step);
		}
	}
}

void bomb_mode::mode_post_solve(const input_type in, const mode_entropy& entropy, const const_logic_step step) {
	(void)entropy;

	if (state == arena_mode_state::LIVE) {
		if (bomb_planted(in)) {
			on_bomb_entity(in, [&](const auto& typed_bomb) {
				if constexpr(!std::is_same_v<decltype(typed_bomb), const std::nullopt_t&>) {
					auto& cosm = in.cosm;
					const auto& clk = cosm.get_clock();

					if (typed_bomb.template get<components::hand_fuse>().when_armed == clk.now) {
						play_sound_for(in, step, battle_event::BOMB_PLANTED);
					}
				}
			});
		}
	}
}

void bomb_mode::respawn_the_dead(const input_type in, const logic_step step, const unsigned after_ms) {
	auto& cosm = in.cosm;
	const auto& clk = cosm.get_clock();

	cosm.for_each_having<components::sentience>([&](const auto typed_handle) {
		auto& sentience = typed_handle.template get<components::sentience>();

		if (sentience.when_knocked_out.was_set() && clk.is_ready(
			after_ms,
			sentience.when_knocked_out
		)) {
			teleport_to_next_spawn(in, typed_handle);
			init_spawned(in, typed_handle, step, nullptr);

			sentience.when_knocked_out = {};
		}
	});
}

const float match_begins_in_secs = 4.f;

float bomb_mode::get_warmup_seconds_left(const input_type in) const {
	if (state == arena_mode_state::WARMUP) {
		return static_cast<float>(in.vars.warmup_secs) - get_total_seconds(in);
	}

	return -1.f;
}

float bomb_mode::get_match_begins_in_seconds(const input_type in) const {
	if (state == arena_mode_state::WARMUP) {
		const auto secs = get_total_seconds(in);
		const auto warmup_secs = static_cast<float>(in.vars.warmup_secs);

		if (secs >= warmup_secs) {
			/* It's after the warmup. */
			return warmup_secs + match_begins_in_secs - secs;
		}
	}

	return -1.f;
}

float bomb_mode::get_total_seconds(const input_type in) const {
	const auto& start_clk = in.initial_signi.clk;
	const auto& clk = in.cosm.get_clock();

	return clk.diff_seconds(start_clk);
}

float bomb_mode::get_freeze_seconds_left(const input_type in) const {
	return static_cast<float>(in.vars.freeze_secs) - get_total_seconds(in);
}

float bomb_mode::get_round_seconds_left(const input_type in) const {
	return static_cast<float>(in.vars.round_secs) + in.vars.freeze_secs - get_total_seconds(in);
}

float bomb_mode::get_round_end_seconds_left(const input_type in) const {
	if (!last_win.was_set()) {
		return -1.f;
	}

	const auto& clk = in.cosm.get_clock();

	return static_cast<float>(in.vars.round_end_secs) - clk.diff_seconds(last_win.when);
}

bool bomb_mode::bomb_exploded(const input_type in) const {
	return on_bomb_entity(in, [&](const auto& t) {
		/* 
			The bomb could only have stopped existing through one way: 
			it has exploded.
		*/

		return std::is_same_v<decltype(t), const std::nullopt_t&>;
	});
}

bool bomb_mode::bomb_defused(const input_type in) const {
	return on_bomb_entity(in, [&](const auto& typed_bomb) {
		if constexpr(std::is_same_v<decltype(typed_bomb), const std::nullopt_t&>) {
			return false;
		}
		else {
			return typed_bomb.template get<components::hand_fuse>().defused();
		}
	});
}

bool bomb_mode::bomb_planted(const input_type in) const {
	return on_bomb_entity(in, [&](const auto& typed_bomb) {
		if constexpr(std::is_same_v<decltype(typed_bomb), const std::nullopt_t&>) {
			return false;
		}
		else {
			return typed_bomb.template get<components::hand_fuse>().armed();
		}
	});
}

float bomb_mode::get_critical_seconds_left(const input_type in) const {
	if (!bomb_planted(in)) {
		return -1.f;
	}

	auto& cosm = in.cosm;
	const auto& clk = cosm.get_clock();

	return on_bomb_entity(in, [&](const auto& typed_bomb) {
		if constexpr(std::is_same_v<decltype(typed_bomb), const std::nullopt_t&>) {
			return -1.f;
		}
		else {
			const auto& fuse_def = typed_bomb.template get<invariants::hand_fuse>();
			const auto& fuse = typed_bomb.template get<components::hand_fuse>();

			const auto when_armed = fuse.when_armed;

			return clk.get_remaining_secs(fuse_def.fuse_delay_ms, when_armed);
		}
	});
}

float bomb_mode::get_seconds_since_planting(const input_type in) const {
	if (!bomb_planted(in)) {
		return -1.f;
	}

	auto& cosm = in.cosm;
	const auto& clk = cosm.get_clock();

	return on_bomb_entity(in, [&](const auto& typed_bomb) {
		if constexpr(std::is_same_v<decltype(typed_bomb), const std::nullopt_t&>) {
			return -1.f;
		}
		else {
			const auto& fuse = typed_bomb.template get<components::hand_fuse>();
			const auto when_armed = fuse.when_armed;

			return (clk.now - when_armed).in_seconds(clk.dt);
		}
	});
}

unsigned bomb_mode::get_round_num() const {
	unsigned total = 0;

	for_each_faction([&](const auto f) {
		total += factions[f].score;
	});

	return total;
}
