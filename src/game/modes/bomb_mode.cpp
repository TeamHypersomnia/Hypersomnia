#include "game/cosmos/solvers/standard_solver.h"
#include "game/messages/health_event.h"
#include "game/modes/bomb_mode.hpp"
#include "game/modes/mode_entropy.h"
#include "game/modes/mode_helpers.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/create_entity.hpp"
#include "game/detail/inventory/generate_equipment.h"
#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"
#include "game/messages/start_sound_effect.h"
#include "game/detail/get_knockout_award.h"
#include "game/detail/damage_origin.hpp"
#include "game/messages/changed_identities_message.h"
#include "game/messages/health_event.h"
#include "augs/string/format_enum.h"
#include "game/messages/battle_event_message.h"
#include "game/modes/detail/item_purchase_logic.hpp"
#include "game/detail/buy_area_in_range.h"

using input_type = bomb_mode::input;

bomb_mode_player* bomb_mode::find(const mode_player_id& id) {
	return mapped_or_nullptr(players, id);
}

const bomb_mode_player* bomb_mode::find(const mode_player_id& id) const {
	return mapped_or_nullptr(players, id);
}

int bomb_mode_player_stats::calc_score() const {
	return 
		knockouts * 2 
		+ assists 
		+ bomb_plants * 2
		+ bomb_explosions * 2
		+ bomb_defuses * 4
	;
}

bool bomb_mode_player::operator<(const bomb_mode_player& b) const {
	const auto as = stats.calc_score();
	const auto bs = b.stats.calc_score();

	if (as == bs) {
		return chosen_name < b.chosen_name;
	}

	return as > bs;
}

std::size_t bomb_mode::get_round_rng_seed(const cosmos& cosm) const {
	(void)cosm;
	return rng_seed_offset + clock_before_setup.now.step + get_round_num(); 
}

std::size_t bomb_mode::get_step_rng_seed(const cosmos& cosm) const {
	return get_round_rng_seed(cosm) + cosm.get_total_steps_passed();
}

faction_choice_result bomb_mode::choose_faction(const mode_player_id& id, const faction_type faction) {
	if (const auto entry = find(id)) {
		if (entry->faction == faction) {
			return faction_choice_result::THE_SAME;
		}

		entry->faction = faction;
		return faction_choice_result::CHANGED;
	}

	return faction_choice_result::FAILED;
}

template <class F>
decltype(auto) bomb_mode::on_bomb_entity(const input_type in, F callback) const {
	auto& vars = in.vars;
	auto& cosm = in.cosm;

	const auto bomb_flavour = vars.bomb_flavour;
	const auto& flavours = cosm.get_solvable_inferred().name;

	if (!bomb_flavour.is_set()) {
		return callback(std::nullopt);
	}

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

	return faction_type::SPECTATOR;
}

void bomb_mode::init_spawned(
	const input in, 
	const entity_id id, 
	const logic_step step,
	const std::optional<transfer_meta> transferred
) {
	auto& cosm = in.cosm;
	const auto handle = cosm[id];
	const auto& faction_vars = in.vars.factions[handle.get_official_faction()];

	handle.dispatch_on_having_all<components::sentience>([&](const auto typed_handle) {
		if (transferred != std::nullopt && transferred->player.survived) {
			const auto& eq = transferred->player.saved_eq;

			if (const auto sentience = typed_handle.template find<components::sentience>()) {
				sentience->learnt_spells = transferred->player.saved_spells;
			}

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

				auto request = item_slot_transfer_request::standard(new_item.get_id(), target_slot);
				request.params.bypass_mounting_requirements = true;
				
				const auto result = perform_transfer_no_step(request, cosm);
				result.notify_logical(step);

				const auto new_id = new_item.get_id();
				transferred->msg.changes[i.source_entity_id] = new_id;

				created_items.push_back(new_id);
			}
		}
		else {
			faction_vars.initial_eq.generate_for(typed_handle, step);
		}

		{
			auto& sentience = typed_handle.template get<components::sentience>();
			for_each_through_std_get(sentience.meters, [](auto& m) { m.make_full(); });
		}

		if (transferred != std::nullopt) {
			typed_handle.template get<components::movement>().flags = transferred->player.movement;
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

	const auto new_id = first_free_key(players, mode_player_id::first());

	players.try_emplace(new_id, chosen_name);
	recently_added_players.push_back(new_id);

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
	auto rng = randomization(get_step_rng_seed(cosm) + static_cast<int>(faction));

	auto& faction_state = factions[faction];

	auto& spawns = faction_state.shuffled_spawns;
	const auto last_spawn = spawns.empty() ? entity_guid() : spawns.back();

	spawns.clear();

	auto adder = [&](const auto typed_spawn) {
		spawns.push_back(typed_spawn.get_guid());
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

template <class C, class F>
void bomb_mode::for_each_player_handle_in(C& cosm, const faction_type faction, F&& callback) const {
	for_each_player_in(faction, [&](const auto&, const auto& data) {
		if (const auto handle = cosm[data.guid]) {
			return handle.template dispatch_on_having_all_ret<components::sentience>([&](const auto& typed_player) {
				if constexpr(is_nullopt_v<decltype(typed_player)>) {
					return callback_result::CONTINUE;
				}
				else {
					return continue_or_callback_result(std::forward<F>(callback), typed_player);
				}
			});
		}

		return callback_result::CONTINUE;
	});
}

template <class C, class F>
decltype(auto) bomb_mode::on_player_handle(C& cosm, const mode_player_id& id, F&& callback) const {
	if (const auto player_data = find(id)) {
		if (const auto handle = cosm[player_data->guid]) {
			return callback(handle);
		}
	}

	return callback(std::nullopt);
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

faction_choice_result bomb_mode::auto_assign_faction(const input_type in, const mode_player_id& id) {
	if (const auto entry = find(id)) {
		auto& f = entry->faction;
		const auto previous_faction = f;
		f = faction_type::SPECTATOR;

		/* Now if factions were all even, it will assign to the same faction and return false for "no change" */
		f = calc_weakest_faction(in);

		const bool faction_changed = f != previous_faction;
		return faction_changed ? faction_choice_result::CHANGED : faction_choice_result::BEST_BALANCE_ALREADY;
	}

	return faction_choice_result::FAILED;
}

void bomb_mode::set_players_frozen(const input_type in, const bool flag) {
	auto& current_flag = current_round.cache_players_frozen;

	if (current_flag == flag) {
		return;
	}

	current_flag = flag;

	for (auto& it : players) {
		auto& player_data = it.second;

		if (const auto handle = in.cosm[player_data.guid]) {
			handle.set_frozen(flag);
		}
	}
}

void bomb_mode::release_triggers_of_weapons_of_players(const input_type in) {
	for (auto& it : players) {
		auto& player_data = it.second;

		if (const auto handle = in.cosm[player_data.guid]) {
			handle.for_each_contained_item_recursive(
				[&](const auto contained_item) {
					unset_input_flags_of_orphaned_entity(contained_item);
				}
			);
		}
	}
}

void bomb_mode::spawn_bomb_near_players(const input_type in) {
	vec2 avg_pos;
	vec2 avg_dir;

	std::size_t n = 0;

	const auto p = calc_participating_factions(in);
	auto& cosm = in.cosm;

	for_each_faction_spawn(cosm, p.bombing, [&](const auto& typed_spawn) {
		const auto& tr = typed_spawn.get_logic_transform();

		avg_pos += tr.pos;
		avg_dir += tr.get_direction();

		++n;
	});

	avg_pos /= n;
	avg_dir /= n;

	const auto new_bomb_entity = spawn_bomb(in);

	if (new_bomb_entity) {
		new_bomb_entity.set_logic_transform(transformr(avg_pos));
		new_bomb_entity.get<components::rigid_body>().apply_impulse(avg_dir * 100);
	}
}


entity_handle bomb_mode::spawn_bomb(const input_type in) {
	return cosmic::create_entity(in.cosm, in.vars.bomb_flavour);
}

bool bomb_mode::give_bomb_to_random_player(const input_type in, const logic_step step) {
	static const auto tried_slots = std::array<slot_function, 3> {
		slot_function::BACK,

		slot_function::PRIMARY_HAND,
		slot_function::SECONDARY_HAND
	};

	const auto p = calc_participating_factions(in);

	auto& cosm = in.cosm;

	const auto viable_players = [&]() {
		std::vector<typed_entity_id<player_character_type>> result;

		for_each_player_handle_in(cosm, p.bombing, [&](const auto& typed_player) {
			for (const auto& t : tried_slots) {
				if (typed_player[t].is_empty_slot()) {
					result.push_back(typed_player.get_id());
					return;
				}
			}
		});

		return result;
	}();

	if (viable_players.empty()) {
		return false;
	}

	const auto chosen_bomber_idx = get_step_rng_seed(cosm) % viable_players.size();
	const auto typed_player = cosm[viable_players[chosen_bomber_idx]];

	for (const auto& t : tried_slots) {
		if (typed_player[t].is_empty_slot()) {
			auto request = item_slot_transfer_request::standard(spawn_bomb(in).get_id(), typed_player[t].get_id());
			request.params.bypass_mounting_requirements = true;
			perform_transfer(request, step);
			break;
		}
	}

	return true;
}


entity_id bomb_mode::create_character_for_player(
	const input_type in, 
	const logic_step step,
	const mode_player_id id,
	const std::optional<transfer_meta> transferred
) {
	if (auto player_data = find(id)) {
		auto& p = *player_data;
		auto& cosm = in.cosm;

		const auto handle = [&]() {
			const auto faction = p.faction;

			if (faction == faction_type::SPECTATOR) {
				return cosm[entity_id()];
			}

			if (const auto flavour = ::find_faction_character_flavour(cosm, faction); flavour.is_set()) {
				auto new_player = cosmic::create_entity(
					cosm, 
					entity_flavour_id(flavour), 
					[](auto&&...) {},
					[&](const auto new_character) {
						teleport_to_next_spawn(in, new_character);
						init_spawned(in, new_character, step, transferred);
					}
				);

				return new_player;
			}

			return cosm[entity_id()];
		}();

		if (handle.alive()) {
			cosmic::set_specific_name(handle, p.chosen_name);
			p.guid = handle.get_guid();
			return handle.get_id();
		}
		else {
			p.guid = {};
		}
	}
	
	return {};
}

void bomb_mode::setup_round(
	const input_type in, 
	const logic_step step, 
	const bomb_mode::round_transferred_players& transfers
) {
	clear_players_round_state(in);

	auto& cosm = in.cosm;
	clock_before_setup = cosm.get_clock();

	const auto former_ids = [&]() {
		std::unordered_map<mode_player_id, entity_id> result;

		for (const auto& p : players) {
			const auto new_id = cosm[p.second.guid].get_id();

			if (new_id.is_set()) {
				result.try_emplace(p.first, new_id);
			}
		}

		return result;
	}();

	cosm.set(in.initial_signi);
	remove_test_characters(cosm);

	if (in.vars.clear_dropped_items) {
		remove_test_dropped_items(cosm);
	}

	current_round = {};

	for_each_faction([&](const auto faction) {
		reshuffle_spawns(cosm, faction);
	});


	messages::changed_identities_message msg;

	for (auto& it : players) {
		const auto id = it.first;
		const auto transferred = mapped_or_nullptr(transfers, id);

		const auto meta = 
			transferred != nullptr ? 
			std::make_optional(transfer_meta{ *transferred, msg }) :
		   	std::optional<transfer_meta>()
		;

		const auto new_id = create_character_for_player(in, step, id, meta);

		if (const auto former_id = mapped_or_nullptr(former_ids, id)) {
			msg.changes.try_emplace(*former_id, new_id);
		}
	}

	if (msg.changes.size() > 0) {
		step.post_message(msg);
	}

	if (in.vars.freeze_secs > 0.f) {
		if (state != arena_mode_state::WARMUP) {
			set_players_frozen(in, true);

			release_triggers_of_weapons_of_players(in);
		}
	}
	else {
		play_sound_for(in, step, battle_event::START);
	}

	if (state != arena_mode_state::WARMUP) {
		if (!give_bomb_to_random_player(in, step)) {
			spawn_bomb_near_players(in);
		}
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

			if (const auto sentience = handle.find<components::sentience>()) {
				pm.saved_spells = sentience->learnt_spells;
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

					const auto source_entity_id = typed_item.get_id();
					items.push_back({ flavour_id, charges, container_index, slot.get_type(), source_entity_id });

					return recursive_callback_result::CONTINUE_AND_RECURSE;
				}
			);
		}
	}

	return result;
}

void bomb_mode::start_next_round(const input_type in, const logic_step step, const round_start_type type) {
	state = arena_mode_state::LIVE;

	if (type == round_start_type::KEEP_EQUIPMENTS) {
		setup_round(in, step, make_transferred_players(in));
	}
	else {
		setup_round(in, step);
	}
}

mode_player_id bomb_mode::lookup(const entity_guid& guid) const {
	for (const auto& p : players) {
		if (p.second.guid == guid) {
			return p.first;
		}
	}

	return mode_player_id::dead();
}

void bomb_mode::count_knockout(const input_type in, const entity_guid victim, const components::sentience& sentience) {
	const auto& cosm = in.cosm;
	const auto& clk = cosm.get_clock();
	const auto& origin = sentience.knockout_origin;
	const auto knockouter = origin.get_guilty_of_damaging(cosm[victim]);

	ensure(knockouter.alive());

	auto assists = sentience.damage_owners;

	bomb_mode_knockout ko;

	for (const auto& candidate : assists) {
		if (const auto who = cosm[candidate.who]) {
			if (who != knockouter) {
				if (candidate.amount >= in.vars.minimal_damage_for_assist) {
					ko.assist = lookup(cosm[assists.front().who]);
					break;
				}
			}
		}
	}

	ko.when = clk;
	ko.origin = origin;

	ko.knockouter = lookup(knockouter);
	ko.victim = lookup(victim);

	count_knockout(in, ko);
}

void bomb_mode::count_knockout(const input_type in, const arena_mode_knockout ko) {
	current_round.knockouts.push_back(ko);

	auto faction_of = [&](const auto a) {
		return in.cosm[lookup(a)].get_official_faction();
	};

	auto same_faction = [&](const auto a, const auto b) {
		return faction_of(a) == faction_of(b);
	};

	auto stats_of = [&](const auto& id) -> auto& {
		return players[id].stats;
	};

	{
		int knockouts_dt = 1;

		if (ko.knockouter == ko.victim) {
			knockouts_dt = 0;
		}
		else if (same_faction(ko.knockouter, ko.victim)) {
			knockouts_dt = -1;
			post_award(in, ko.knockouter, in.vars.economy.team_kill_penalty * -1);
		}

		if (knockouts_dt > 0) {
			auto& cosm = in.cosm;

			const auto award = ko.origin.on_tool_used(cosm, [&](const auto& tool) {
				if constexpr(is_spell_v<decltype(tool)>) {
					return tool.common.adversarial.knockout_award;
				}
				else {
					return ::get_knockout_award(tool);
				}
			});

			if (award != std::nullopt) {
				post_award(in, ko.knockouter, *award);
			}
		}

		stats_of(ko.knockouter).knockouts += knockouts_dt;
		stats_of(ko.victim).deaths += 1;
	}

	if (ko.assist.is_set()) {
		int assists_dt = 1;

		if (same_faction(ko.assist, ko.victim)) {
			assists_dt = -1;
		}

		stats_of(ko.assist).assists += assists_dt;
	}
}

void bomb_mode::count_knockouts_for_unconscious_players_in(const input_type in, const faction_type faction) {
	for_each_player_handle_in(in.cosm, faction, [&](const auto& typed_player) {
		const auto& sentience = typed_player.template get<components::sentience>();

		if (sentience.unconscious_but_alive()) {
			count_knockout(in, typed_player, sentience);
		}
	});
}

bool bomb_mode::is_halfway_round(const input_type in) const {
	const auto n = in.vars.get_num_rounds();
	const auto current_round = get_round_num();

	return current_round == n / 2;
}

bool bomb_mode::is_final_round(const input_type in) const {
	const auto n = in.vars.get_num_rounds();
	const auto current_round = get_round_num();

	bool someone_has_over_half = false;

	const auto p = calc_participating_factions(in);
	
	p.for_each([&](const auto f) {
		if (get_score(f) > n / 2) {
			someone_has_over_half = true;
		}
	});

	return someone_has_over_half || current_round == n;
}

void bomb_mode::make_win(const input_type in, const faction_type winner) {
	const auto p = calc_participating_factions(in);
	const auto loser = winner == p.defusing ? p.bombing : p.defusing;

	++factions[winner].score;
	factions[winner].consecutive_losses = 0;

	state = arena_mode_state::ROUND_END_DELAY;
	current_round.last_win = { in.cosm.get_clock(), winner };

	auto& consecutive_losses = factions[loser].consecutive_losses;
	++consecutive_losses;

	auto winner_award = in.vars.economy.winning_faction_award;
	auto loser_award = in.vars.economy.losing_faction_award;

	if (consecutive_losses > 1) {
		loser_award += std::min(
			consecutive_losses - 1, 
			in.vars.economy.max_consecutive_loss_bonuses
		) * in.vars.economy.consecutive_loss_bonus;
	}

	if (loser == p.bombing) {
		if (current_round.bomb_planter.is_set()) {
			loser_award += in.vars.economy.lost_but_bomb_planted_team_bonus;
			winner_award += in.vars.economy.defused_team_bonus;
		}
	}
	
	for (auto& p : players) {
		const auto& player_id = p.first;
		const auto faction = p.second.faction;

		post_award(in, player_id, faction == winner ? winner_award : loser_award);
	}

	if (is_halfway_round(in) || is_final_round(in)) {
		state = arena_mode_state::MATCH_SUMMARY;
		set_players_frozen(in, true);
		release_triggers_of_weapons_of_players(in);
	}
}

void bomb_mode::play_faction_sound(const const_logic_step step, const faction_type f, const assets::sound_id id) const {
	sound_effect_input effect;
	effect.id = id;

	sound_effect_start_input input;
	input.listener_faction = f;
	input.variation_number = get_step_rng_seed(step.get_cosmos());

	effect.start(step, input);
}

void bomb_mode::play_win_sound(const input_type in, const const_logic_step step, const faction_type winner) const {
	const auto p = calc_participating_factions(in);

	p.for_each([&](const faction_type t) {
		if (const auto sound_id = in.vars.view.win_sounds[t][winner]; sound_id.is_set()) {
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
	if (const auto sound_id = in.vars.view.event_sounds[t][event]; sound_id.is_set()) {
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
			start.variation_number = get_round_rng_seed(in.cosm);
		}

		auto& effects = msg.payload.inputs;
		effects.reserve(2);

		if (const auto defused_id = in.vars.view.event_sounds[t][battle_event::BOMB_DEFUSED]; defused_id.is_set()) {
			sound_effect_input effect;
			effect.id = defused_id;

			effects.emplace_back(std::move(effect));
		}

		if (const auto win_id = in.vars.view.win_sounds[t][winner]; win_id.is_set()) {
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
		const auto planting_player = current_round.bomb_planter;
		++players[planting_player].stats.bomb_explosions;
		post_award(in, planting_player, in.vars.economy.bomb_explosion_award);
		standard_win(p.bombing);
		return;
	}

	if (const auto character_who_defused = cosm[get_character_who_defused_bomb(in)]) {
		const auto winner = p.defusing;
		const auto defusing_player = lookup(character_who_defused.get_guid());
		++players[defusing_player].stats.bomb_defuses;
		post_award(in, defusing_player, in.vars.economy.bomb_defuse_award);
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

void bomb_mode::execute_player_commands(const input_type in, const mode_entropy& entropy, const logic_step step) {
	const auto current_round = get_round_num();
	auto& cosm = in.cosm;

	for (const auto& p : entropy.players) {
		const auto& commands = p.second;
		const auto id = p.first;

		if (const auto player_data = find(id)) {
			if (get_buy_seconds_left(in) > 0.f) {
				on_player_handle(cosm, id, [&](const auto& player_handle) {
					if constexpr(!is_nullopt_v<decltype(player_handle)>) {
						if (!buy_area_in_range(player_handle)) {
							return;
						}

						const auto& purchases = commands.queues.get_queue<mode_commands::item_purchase>();
						auto& stats = player_data->stats;
						auto& money = stats.money;
						auto& done_purchases = stats.round_state.done_purchases;

						for (const auto& p : purchases) {
							if (const auto f_id = p.item; ::is_alive(cosm, f_id)) {
								if (!factions_compatible(player_handle, f_id)) {
									continue;
								}

								const auto price = ::find_price_of(cosm, f_id);
								const bool price_correct = price && *price != 0;

								if (!price_correct) {
									continue;
								}

								if (money >= *price) {
									if (::num_carryable_pieces(player_handle, ::get_buy_slot_opts(), f_id) == 0) {
										continue;
									}

									requested_equipment eq;

									if (::is_magazine_like(cosm, f_id)) {
										eq.non_standard_mag = f_id;
										eq.num_given_ammo_pieces = 1;
									}
									else {
										if (found_in(done_purchases, f_id)) {
											eq.num_given_ammo_pieces = 1;
										}
										else {
											done_purchases.push_back(f_id);
										}

										eq.weapon = f_id;
									}

									eq.generate_for(player_handle, step);

									money -= *price;
								}

								continue;
							}

							if (const auto s_id = p.spell; ::is_alive(cosm, s_id)) {
								if (!factions_compatible(player_handle, s_id)) {
									continue;
								}

								const auto price = ::find_price_of(cosm, s_id);
								const bool price_correct = price && *price != 0;

								if (!price_correct) {
									continue;
								}

								const auto& sentience = player_handle.template get<components::sentience>();

								if (money >= *price && !sentience.is_learnt(s_id)) {
									requested_equipment eq;

									eq.spells_to_give[s_id.get_index()] = true;
									eq.generate_for(player_handle, step);

									::play_learnt_spell_effect(player_handle, s_id, step);

									money -= *price;
								}

								continue;
							}
						}
					}
					else {
						(void)player_handle;
					}
				});
			}

			if (const auto& new_choice = commands.team_choice; new_choice != std::nullopt) {
				const auto previous_faction = player_data->faction;

				const auto f = new_choice->target_team;

				// TODO: Notify GUI about the result.

				const auto result = [&]() {
					if (f != faction_type::SPECTATOR) {
						/* This is a serious choice */
						if (player_data->round_when_chosen_faction == current_round) {
							return faction_choice_result::CHOOSING_TOO_FAST;
						}

						player_data->round_when_chosen_faction = current_round;
					}

					if (f == faction_type::COUNT) {
						/* Auto-select */
						return auto_assign_faction(in, id);
					}

					return choose_faction(id, f);
				}();

				LOG(format_enum(result));

				if (result == faction_choice_result::CHANGED) {
					LOG("Changed from %x to %x", format_enum(previous_faction), format_enum(f));

					on_player_handle(cosm, id, [&](const auto& player_handle) {
						if constexpr(!is_nullopt_v<decltype(player_handle)>) {
							if (const auto tr = player_handle.find_logic_transform()) {
								if (const auto sentience = player_handle.template find<components::sentience>()) {
									if (!sentience->is_dead()) {
										damage_origin origin;
										origin.cause.flavour = player_handle.get_flavour_id();
										origin.cause.entity = player_handle.get_id();
										origin.sender.set(player_handle);

										const auto death_request = messages::health_event::request_death(
											player_handle.get_id(),
											tr->get_direction(),
											tr->pos,
											origin
										);

										step.post_message(death_request);
									}
								}
							}
						}
						else {
							(void)player_handle;
						}
					});
				}
			}
		}
	}
}

void bomb_mode::spawn_recently_added_players(const input_type in, const logic_step step) {
	for (const auto& id : recently_added_players) {
		if (const auto player_data = find(id)) {
			if (player_data->guid.is_set()) {
				continue;
			}

			auto do_spawn = [&]() {
				create_character_for_player(in, step, id);
			};

			if (state == arena_mode_state::WARMUP) {
				/* Always spawn in warmup */
				do_spawn();
			}
			else if (state == arena_mode_state::LIVE) {
				const auto passed = get_round_seconds_passed(in);

				if (passed <= in.vars.allow_spawn_after_secs_after_starting) {
					do_spawn();
				}
			}
		}
	}

	recently_added_players.clear();
}

void bomb_mode::mode_pre_solve(const input_type in, const mode_entropy& entropy, const logic_step step) {
	spawn_recently_added_players(in, step);

	if (state == arena_mode_state::INIT) {
		restart(in, step);
	}
	else if (state == arena_mode_state::WARMUP) {
		respawn_the_dead(in, step, in.vars.warmup_respawn_after_ms);

		if (get_warmup_seconds_left(in) <= 0.f) {
			if (!current_round.cache_players_frozen) {
				set_players_frozen(in, true);
#if CLEAR_TRIGGERS_AFTER_WARMUP_ENDS
				release_triggers_of_weapons_of_players(in);
#endif
			}

			if (get_match_begins_in_seconds(in) <= 0.f) {
				state = arena_mode_state::LIVE;
				reset_players_stats(in);
				setup_round(in, step);
			}
		}
	}
	else if (state == arena_mode_state::LIVE) {
		if (get_freeze_seconds_left(in) <= 0.f) {
			if (current_round.cache_players_frozen) {
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
	else if (state == arena_mode_state::MATCH_SUMMARY) {
		if (get_match_summary_seconds_left(in) <= 0.f) {
			const auto p = calc_participating_factions(in);

			for (auto& it : players) {
				auto& player_data = it.second;
				p.make_swapped(player_data.faction);
			}

			if (is_final_round(in)) {
				restart(in, step);
			}
			else {
				/* Switch sides */
				std::swap(factions[p.bombing].score, factions[p.defusing].score);

				p.for_each([&](const auto f) {
					factions[f].clear_for_next_half();
				});

				set_players_money_to_initial(in);
				start_next_round(in, step, round_start_type::DONT_KEEP_EQUIPMENTS);
			}
		}
	}

	execute_player_commands(in, entropy, step);
}

void bomb_mode::mode_post_solve(const input_type in, const mode_entropy& entropy, const const_logic_step step) {
	(void)entropy;
	auto& cosm = in.cosm;

	{
		/* Request to play the battle event sounds */
		const auto& events = step.get_queue<messages::battle_event_message>();

		for (const auto& e : events) {
			const auto event = e.event;

			if (const auto subject = cosm[e.subject]) {
				const auto faction = subject.get_official_faction();

				if (event == battle_event::INTERRUPTED_DEFUSING) {
					LOG("IOnter");
					if (const auto sound_id = in.vars.view.event_sounds[faction][battle_event::IM_DEFUSING_THE_BOMB]; sound_id.is_set()) {
						messages::stop_sound_effect stop;
						stop.match_effect_id = sound_id;
						step.post_message(stop);
					}
				}
				else {
					if (const auto sound_id = in.vars.view.event_sounds[faction][event]; sound_id.is_set()) {
						play_faction_sound(step, faction, sound_id);
					}
				}
			}
		}
	}

	{
		const auto& events = step.get_queue<messages::health_event>();

		for (const auto& e : events) {
			if (const auto victim = cosm[e.subject]) {
				auto make_it_count = [&]() {
					count_knockout(in, victim, victim.get<components::sentience>());
				};

				if (e.special_result == messages::health_event::result_type::LOSS_OF_CONSCIOUSNESS) {
					make_it_count();
				}
				else if (e.special_result == messages::health_event::result_type::DEATH) {
					/* Don't count two kills on a single character. */
					if (e.was_conscious) {
						make_it_count();
					}
				}
			}
		}
	}

	if (state == arena_mode_state::LIVE) {
		if (bomb_planted(in)) {
			on_bomb_entity(in, [&](const auto& typed_bomb) {
				if constexpr(!is_nullopt_v<decltype(typed_bomb)>) {
					const auto& clk = cosm.get_clock();

					if (typed_bomb.template get<components::hand_fuse>().when_armed == clk.now) {
						play_sound_for(in, step, battle_event::BOMB_PLANTED);

						auto& planter = current_round.bomb_planter;
						planter = lookup(cosm[typed_bomb.template get<components::sender>().capability_of_sender].get_guid());
						++players[planter].stats.bomb_plants;

						post_award(in, planter, in.vars.economy.bomb_plant_award);
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
			init_spawned(in, typed_handle, step, std::nullopt);

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

float bomb_mode::get_round_seconds_passed(const input_type in) const {
	return get_total_seconds(in) - static_cast<float>(in.vars.freeze_secs);
}

float bomb_mode::get_freeze_seconds_left(const input_type in) const {
	return static_cast<float>(in.vars.freeze_secs) - get_total_seconds(in);
}

float bomb_mode::get_buy_seconds_left(const input_type in) const {
	return static_cast<float>(in.vars.freeze_secs + in.vars.buy_secs_after_freeze) - get_total_seconds(in);
}

float bomb_mode::get_round_seconds_left(const input_type in) const {
	return static_cast<float>(in.vars.round_secs) + in.vars.freeze_secs - get_total_seconds(in);
}

float bomb_mode::get_seconds_since_win(const input_type in) const {
	const auto& last_win = current_round.last_win;

	if (!last_win.was_set()) {
		return -1.f;
	}

	const auto& clk = in.cosm.get_clock();

	return clk.diff_seconds(last_win.when);
}

float bomb_mode::get_match_summary_seconds_left(const input_type in) const {
	if (const auto since_win = get_seconds_since_win(in); since_win != -1.f) {
		return static_cast<float>(in.vars.match_summary_seconds) - since_win;
	}

	return -1.f;
}

float bomb_mode::get_round_end_seconds_left(const input_type in) const {
	if (!current_round.last_win.was_set()) {
		return -1.f;
	}

	return static_cast<float>(in.vars.round_end_secs) - get_seconds_since_win(in);
}

bool bomb_mode::bomb_exploded(const input_type in) const {
	return on_bomb_entity(in, [&](const auto& t) {
		/* 
			The bomb could only have stopped existing through one way: 
			it has exploded.
		*/

		return is_nullopt_v<decltype(t)>;
	});
}

entity_id bomb_mode::get_character_who_defused_bomb(const input_type in) const {
	return on_bomb_entity(in, [&](const auto& typed_bomb) {
		if constexpr(is_nullopt_v<decltype(typed_bomb)>) {
			return entity_id();
		}
		else {
			const auto& fuse = typed_bomb.template get<components::hand_fuse>();
			
			if (fuse.defused()) {
				return fuse.character_now_defusing;
			}

			return entity_id();
		}
	});
}

bool bomb_mode::bomb_planted(const input_type in) const {
	return on_bomb_entity(in, [&](const auto& typed_bomb) {
		if constexpr(is_nullopt_v<decltype(typed_bomb)>) {
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
		if constexpr(is_nullopt_v<decltype(typed_bomb)>) {
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
		if constexpr(is_nullopt_v<decltype(typed_bomb)>) {
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
		total += get_score(f);
	});

	return total;
}

unsigned bomb_mode::get_score(const faction_type f) const {
	return factions[f].score;
}

std::optional<arena_mode_match_result> bomb_mode::calc_match_result(const input_type in) const {
	if (state != arena_mode_state::MATCH_SUMMARY) {
		return std::nullopt;
	}

	const auto p = calc_participating_factions(in);

	if (get_score(p.bombing) == get_score(p.defusing)) {
		return arena_mode_match_result::make_tie();
	}

	arena_mode_match_result result;
	result.winner = get_score(p.bombing) > get_score(p.defusing) ? p.bombing : p.defusing;
	return result;
}

void bomb_mode::request_restart() {
	state = arena_mode_state::INIT;
}

void bomb_mode::restart(const input_type in, const logic_step step) {
	reset_players_stats(in);
	factions = {};

	if (in.vars.warmup_secs > 4) {
		state = arena_mode_state::WARMUP;
	}
	else {
		state = arena_mode_state::LIVE;
	}

	setup_round(in, step);
}

unsigned bomb_mode::calc_max_faction_score() const {
	unsigned maximal = 0;

	for_each_faction([&](const auto f) {
		maximal = std::max(maximal, factions[f].score);
	});

	return maximal;
}


void bomb_mode::clear_players_round_state(const input_type in) {
	(void)in;

	for (auto& it : players) {
		it.second.stats.round_state = {};
	}
}

void bomb_mode::set_players_money_to_initial(const input_type in) {
	for (auto& it : players) {
		auto& p = it.second;
		p.stats.money = in.vars.economy.initial_money;
	}
}

void bomb_mode::reset_players_stats(const input_type in) {
	for (auto& it : players) {
		auto& p = it.second;
		p.stats = {};
	}

	set_players_money_to_initial(in);
	clear_players_round_state(in);
}

void bomb_mode::post_award(const input_type in, const mode_player_id id, money_type amount) {
	auto& stats = players[id].stats;
	auto& current_money = stats.money;
	amount = std::clamp(amount, -current_money, in.vars.economy.maximum_money - current_money);

	if (amount != 0) {
		current_money += amount;

		const auto award = arena_mode_award {
			in.cosm.get_clock(), id, amount 
		};

		stats.round_state.awards.emplace_back(award);
	}
}
