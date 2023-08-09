#include "augs/log.h"
#include "augs/ensure_rel.h"
#include "game/cosmos/solvers/standard_solver.h"
#include "game/messages/health_event.h"
#include "game/modes/arena_mode.hpp"
#include "game/modes/mode_entropy.h"
#include "game/modes/mode_helpers.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
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
#include "game/cosmos/delete_entity.h"
#include "augs/templates/logically_empty.h"

#include "game/detail/sentience/sentience_getters.h"
#include "game/detail/inventory/perform_wielding.hpp"
#include "game/detail/inventory/wielding_setup.hpp"
#include "game/detail/snap_interpolation_to_logical.h"

#include "game/cosmos/just_create_entity_functional.h"
#include "game/detail/visible_entities.hpp"
#include "game/messages/mode_notification.h"
#include "game/messages/hud_message.h"
#include "game/detail/sentience/sentience_logic.h"
#include "game/modes/detail/delete_with_held_items.hpp"
#include "game/modes/detail/hud_message_players.h"

using input_type = arena_mode::input;
using const_input_type = arena_mode::const_input;

int arena_mode_player_stats::calc_score() const {
	return 
		knockouts * 2 
		+ assists 
		+ bomb_plants * 2
		+ bomb_explosions * 2
		+ bomb_defuses * 4
	;
}

bool arena_mode_player::operator<(const arena_mode_player& b) const {
	const auto ao = arena_player_order { get_nickname(), stats.calc_score() };
	const auto bo = arena_player_order { b.get_nickname(), b.stats.calc_score() };

	return ao < bo;
}

std::size_t arena_mode::get_round_rng_seed(const cosmos& cosm) const {
	(void)cosm;
	return clock_before_setup.now.step + get_current_round_number(); 
}

std::size_t arena_mode::get_step_rng_seed(const cosmos& cosm) const {
	return get_round_rng_seed(cosm) + cosm.get_total_steps_passed();
}

faction_choice_result arena_mode::choose_faction(const const_input_type in, const mode_player_id& id, const faction_type faction) {
	if (const auto entry = find(id)) {
		const auto previous_faction = entry->get_faction();

		if (previous_faction == faction) {
			return faction_choice_result::THE_SAME;
		}

		entry->session.faction = faction;

		on_faction_changed_for(in, previous_faction, id);

		return faction_choice_result::CHANGED;
	}

	return faction_choice_result::FAILED;
}

arena_mode::participating_factions arena_mode::calc_participating_factions(const const_input_type in) const {
	const auto& cosm = in.cosm;
	const auto spawnable = calc_spawnable_factions(cosm);

	participating_factions output;

	cosm.template for_each_having<invariants::area_marker>([&](const auto& typed_handle) {
		const auto& marker = typed_handle.template get<invariants::area_marker>();

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

	if (!output.valid()) {
		output = participating_factions::fallback();
	}

	return output;
}

faction_type arena_mode::get_opposing_faction(const const_input_type in, const faction_type faction) const {
	const auto participating = calc_participating_factions(in);

	return faction == participating.bombing ? participating.defusing : participating.bombing;
}

faction_type arena_mode::calc_weakest_faction(const const_input_type in) const {
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

faction_type arena_mode::get_player_faction(const mode_player_id& id) const {
	if (const auto entry = find(id)) {
		return entry->get_faction();
	}

	return faction_type::SPECTATOR;
}

template <class H>
void arena_mode_set_transferred_item_meta(const H handle, int charges, const item_owner_meta& meta) {
	auto& item = *handle.template get<components::item>().component;
	item.charges = charges;
	item.owner_meta = meta;
}

void arena_mode::init_spawned(
	const input_type in, 
	const mode_player_id player_id,
	const entity_id id, 
	const logic_step step,
	const std::optional<transfer_meta> transferred
) {
	auto& cosm = in.cosm;
	const auto handle = cosm[id];
	const auto& faction_rules = in.rules.factions[handle.get_official_faction()];

	auto access = allocate_new_entity_access();

	handle.dispatch_on_having_all<components::sentience>([&](const auto& typed_handle) {
		if (levelling_enabled(in)) {
			reset_equipment_for(step, in, player_id, typed_handle);
		}
		else if (transferred.has_value() && transferred->player.survived) {
			const auto& eq = transferred->player.saved_eq;

			if (const auto sentience = typed_handle.template find<components::sentience>()) {
				sentience->learnt_spells = transferred->player.saved_spells;
			}

			std::vector<entity_id> created_items;

			for (const auto& i : eq.items) {
				const auto& idx = i.container_index;
				const auto target_container_id = 
					idx < created_items.size() ? 
					created_items[idx] : 
					entity_id(typed_handle.get_id())
				;

				const auto target_slot_id = inventory_slot_id { i.slot_type, target_container_id };
				const auto target_slot = cosm[target_slot_id];

				auto skip_creation = [&]() {
					transferred->msg.changes[i.source_entity_id] = entity_id();
					created_items.push_back(entity_id());
				};

				{
					/* 
						If we are refilling, don't create cartridges now,
						because we will do it manually for each magazine and chamber magazine.
					*/

					const auto target_container = cosm[target_container_id];

					if (in.rules.refill_all_mags_on_round_start) {
						if (::is_magazine_like(target_container)) {
							skip_creation();
							continue;
						}
					}

					if (in.rules.refill_chambers_on_round_start) {
						if (
							target_slot_id.type == slot_function::GUN_CHAMBER
							|| target_slot_id.type == slot_function::GUN_CHAMBER_MAGAZINE
						) {
							skip_creation();
							continue;
						}
					}
				}

				const auto new_item = just_create_entity(
					access,
					cosm,
					i.flavour,
					[&](const entity_handle handle) {
						arena_mode_set_transferred_item_meta(handle, i.charges, i.owner_meta);
					}
				);

				if (new_item) {
					{
						auto request = item_slot_transfer_request::standard(new_item, target_slot);
						request.params.bypass_mounting_requirements = true;
						request.params.play_transfer_sounds = false;
						request.params.play_transfer_particles = false;
						request.params.perform_recoils = false;

						const auto result = perform_transfer_no_step(request, cosm);
						result.notify_logical(step);
					}

					{
						const auto new_id = new_item.get_id();
						transferred->msg.changes[i.source_entity_id] = new_id;

						created_items.push_back(new_id);
					}

					auto fill_or_refill = [&cosm, &step, &typed_handle, access](const auto slot) {
						if (const auto charge_inside = slot.get_item_if_any()) {
							charge_inside.set_charges(charge_inside.num_charges_fitting_in(slot));
						}
						else {
							auto charge_flavour = slot->only_allow_flavour;

							if (!charge_flavour.is_set()) {
								charge_flavour = ::calc_default_charge_flavour(slot.get_container());
							}

							if (charge_flavour.is_set()) {
								const auto charge = just_create_entity(access, cosm, charge_flavour);
								set_original_owner(charge, typed_handle);

								charge.set_charges(charge.num_charges_fitting_in(slot));

								auto request = item_slot_transfer_request::standard(charge, slot);
								request.params.bypass_mounting_requirements = true;
								request.params.play_transfer_sounds = false;
								request.params.play_transfer_particles = false;
								request.params.perform_recoils = false;

								const auto result = perform_transfer_no_step(request, cosm);
								result.notify_logical(step);
							}
						}
					};

					if (in.rules.refill_all_mags_on_round_start) {
						if (::is_magazine_like(new_item)) {
							if (const auto depo = new_item[slot_function::ITEM_DEPOSIT]) {
								fill_or_refill(depo);
							}
						}
					}

					if (in.rules.refill_chambers_on_round_start) {
						const auto chamber = new_item[slot_function::GUN_CHAMBER];

						if (chamber) {
							fill_or_refill(chamber);

							if (const auto chamber_mag = new_item[slot_function::GUN_CHAMBER_MAGAZINE]) {
								if (const auto gun = new_item.template find<invariants::gun>()) {
									if (gun->allow_charge_in_chamber_magazine_when_chamber_loaded) {
										fill_or_refill(chamber_mag);
									}
								}
							}
						}
					}
				}
				else {
					skip_creation();
				}
			}
		}
		else {
			const auto& eq = 
				state == arena_mode_state::WARMUP
				? faction_rules.warmup_initial_eq
				: faction_rules.round_start_eq
			;

			eq.generate_for(access, typed_handle, step, 1);
		}

		::resurrect(step, typed_handle, in.rules.spawn_protection_ms);

		if (transferred.has_value()) {
			typed_handle.template get<components::movement>().flags = transferred->player.movement;

			/* Reset the wielding to hide some mags/bullets that were in hands due to reloading */
			::perform_wielding(
				step,
				typed_handle,
				wielding_setup::from_current(typed_handle)
			);
		}
	});
}

void arena_mode::teleport_to_next_spawn(const input in, const entity_id id) {
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
			snap_interpolated_to(typed_handle, spawn_transform);

			if (const auto crosshair = typed_handle.find_crosshair()) {
				crosshair->base_offset = spawn_transform.get_direction() * 200;
			}

			++spawn_idx;

			if (spawn_idx >= spawns.size()) {
				reshuffle();
			}
		}
	});
}

bool arena_mode::add_player_custom(const input_type in, const add_player_input& add_in) {
	auto& cosm = in.cosm;
	(void)cosm;

	const auto& new_id = add_in.id;

	auto& new_player = (*players.try_emplace(new_id).first).second;

	if (new_player.is_set()) {
		return false;
	}

	LOG_NVPS(new_id.value, next_session_id.value);
	new_player.session.nickname = add_in.name;
	new_player.session.id = next_session_id;
	++next_session_id;

	if (state == arena_mode_state::WARMUP) {
		new_player.stats.money = in.rules.economy.warmup_initial_money;
	}
	else {
		new_player.stats.money = in.rules.economy.initial_money;

		if (get_current_round_number() != 0) {
			new_player.stats.money /= 2;
		}
	}

	return true;
}

mode_player_id arena_mode::add_player(const input_type in, const entity_name_str& nickname) {
	if (const auto new_id = find_first_free_player(); new_id.is_set()) {
		const auto result = add_player_custom(in, { new_id, nickname });
		(void)result;
		ensure(result);
		return new_id;
	}

	return {};
}

void arena_mode::remove_player(input_type in, const logic_step step, const mode_player_id& id) {
	handle_duel_desertion(in, step, id);

	const auto controlled_character_id = lookup(id);

	::delete_with_held_items_except(in.rules.bomb_flavour, step, in.cosm[controlled_character_id]);

	if (const auto entry = find(id)) {
		const auto previous_faction = entry->get_faction();
		const auto free_color = entry->assigned_color;

		erase_element(players, id);
		assign_free_color_to_best_uncolored(in, previous_faction, free_color);
	}
}

mode_entity_id arena_mode::lookup(const mode_player_id& id) const {
	if (const auto entry = find(id)) {
		return entry->controlled_character_id;
	}

	return mode_entity_id::dead();
}

void arena_mode::reshuffle_spawns(const cosmos& cosm, const faction_type faction) {
	const auto reshuffle_seed = get_step_rng_seed(cosm) + static_cast<unsigned>(faction);
	auto rng = randomization(reshuffle_seed);

	auto& faction_state = factions[faction];

	auto& spawns = faction_state.shuffled_spawns;
	const auto last_spawn = spawns.empty() ? mode_entity_id::dead() : spawns.back();

	spawns.clear();

	auto adder = [&](const auto typed_spawn) {
		spawns.push_back(typed_spawn);
	};

	for_each_faction_spawn(cosm, faction, adder);

	shuffle_range(spawns, rng);

	if (last_spawn.is_set() && spawns.size() > 1) {
		if (spawns.back() == last_spawn) {
			std::swap(spawns.front(), spawns.back());
		}
	}

	faction_state.current_spawn_index = 0;
}

template <class C, class F>
void arena_mode::for_each_player_handle_in(C& cosm, const faction_type faction, F&& callback) const {
	for_each_player_in(faction, [&](const auto&, const auto& data) {
		if (const auto handle = cosm[data.controlled_character_id]) {
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

std::size_t arena_mode::num_conscious_players_in(const cosmos& cosm, const faction_type faction) const {
	auto total = std::size_t(0);

	for_each_player_handle_in(cosm, faction, [&](const auto& handle) {
		if (handle.template get<components::sentience>().is_conscious()) {
			++total;
		}
	});

	return total;
}

std::size_t arena_mode::num_players_in(const faction_type faction) const {
	auto total = std::size_t(0);

	for_each_player_in(faction, [&](auto&&...) {
		++total;
	});

	return total;
}

void arena_mode::assign_free_color_to_best_uncolored(const const_input_type in, const faction_type in_faction, const rgba free_color) {
	const auto fallback_color = in.rules.excess_player_color;

	if (free_color == rgba::zero) {
		return;
	}

	if (free_color == fallback_color) {
		return;
	}

	bool found_any_player = false;
	int best_score = std::numeric_limits<int>::min();
	mode_player_id best_player = mode_player_id();

	for_each_player_in(in_faction, [&](const auto& mode_id, const auto& data) {
		if (data.get_faction() == in_faction && data.assigned_color == fallback_color) {
			const int score = data.stats.calc_score();

			if (score > best_score || !found_any_player)
			{
				best_score = score;
				best_player = mode_id;
				found_any_player = true;
			}
		}

		return callback_result::CONTINUE;
	});

	if (found_any_player) {
		if (const auto entry = find(best_player)) {
			entry->assigned_color = free_color;
		}
	}
}

void arena_mode::on_faction_changed_for(const const_input_type in, const faction_type previous_faction, const mode_player_id& id) { 
	if (const auto entry = find(id)) {
		{
			const auto free_color = entry->assigned_color;
			assign_free_color_to_best_uncolored(in, previous_faction, free_color);
		}

		entry->assigned_color = rgba::zero;

		if (entry->get_faction() == faction_type::SPECTATOR) {
			return;
		}

		if (in.rules.enable_player_colors) {
			for (const auto& candidate_color : in.rules.player_colors) {
				bool collision_found = false;

				for (const auto& it : players) {
					const auto& player_data = it.second;

					if (player_data.get_faction() == entry->get_faction()) {
						if (player_data.assigned_color == candidate_color) {
							collision_found = true;
							break;
						}
					}
				}

				if (!collision_found) {
					entry->assigned_color = candidate_color;
					break;
				}
			}

			if (entry->assigned_color == rgba::zero) {
				const auto fallback_color = in.rules.excess_player_color;
				entry->assigned_color = fallback_color;
			}
		}
		else {
			entry->assigned_color = in.rules.default_player_color;
		}
	}
}

faction_choice_result arena_mode::auto_assign_faction(const input_type in, const mode_player_id& id) {
	if (const auto entry = find(id)) {
		auto& f = entry->session.faction;
		const auto previous_faction = f;
		f = faction_type::SPECTATOR;

		/* Now if factions were all even, it will assign to the same faction and return false for "no change" */
		f = calc_weakest_faction(in);

		const bool faction_changed = f != previous_faction;

		if (faction_changed) {
			on_faction_changed_for(in, previous_faction, id);
			return faction_choice_result::CHANGED;
		}

		return faction_choice_result::BEST_BALANCE_ALREADY;
	}

	return faction_choice_result::FAILED;
}

void arena_mode::set_players_frozen(const input_type in, const bool flag) {
	auto& current_flag = current_round.cache_players_frozen;

	if (current_flag == flag) {
		return;
	}

	current_flag = flag;

	for (auto& it : players) {
		auto& player_data = it.second;

		if (const auto handle = in.cosm[player_data.controlled_character_id]) {
			handle.set_frozen(flag);
		}
	}
}

void arena_mode::release_triggers_of_weapons_of_players(const input_type in) {
	for (auto& it : players) {
		auto& player_data = it.second;

		if (const auto handle = in.cosm[player_data.controlled_character_id]) {
			handle.for_each_contained_item_recursive(
				[&](const auto contained_item) {
					unset_input_flags_of_orphaned_entity(contained_item);
				}
			);
		}
	}
}

void arena_mode::spawn_bomb_near_players(const input_type in) {
	auto avg_pos = vec2::zero;
	auto avg_dir = vec2::zero;

	std::size_t n = 0;

	const auto p = calc_participating_factions(in);
	auto& cosm = in.cosm;

	for_each_faction_spawn(cosm, p.bombing, [&](const auto& typed_spawn) {
		const auto& tr = typed_spawn.get_logic_transform();

		avg_pos += tr.pos;
		avg_dir += tr.get_direction();

		++n;
	});

	if (n != 0) {
		avg_pos /= n;
		avg_dir /= n;
	}

	const auto new_bomb_entity = spawn_bomb(in);

	if (new_bomb_entity) {
		new_bomb_entity.set_logic_transform(transformr(avg_pos));
		new_bomb_entity.get<components::rigid_body>().apply_impulse(avg_dir * 100);
	}
}


entity_handle arena_mode::spawn_bomb(const input_type in) {
	auto access = allocate_new_entity_access();

	const auto new_bomb = just_create_entity(access, in.cosm, in.rules.bomb_flavour);
	bomb_entity = new_bomb;
	return new_bomb;
}

bool arena_mode::give_bomb_to_random_player(const input_type in, const logic_step step) {
	auto& cosm = in.cosm;

	bool existing_bomb_found = false;

	cosm.template for_each_having<invariants::hand_fuse>([&](const auto& typed_handle) {
		if (typed_handle.get_flavour_id() == in.rules.bomb_flavour) {
			bomb_entity = typed_handle;
			existing_bomb_found = true;
		}
	});

	if (existing_bomb_found) {
		return true;
	}

	static const auto tried_slots = std::array<slot_function, 3> {
		slot_function::OVER_BACK,

		slot_function::PRIMARY_HAND,
		slot_function::SECONDARY_HAND
	};

	const auto p = calc_participating_factions(in);

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
			const auto spawned_bomb = spawn_bomb(in);

			if (spawned_bomb.dead()) {
				return false;
			}

			auto request = item_slot_transfer_request::standard(spawned_bomb.get_id(), typed_player[t].get_id());
			request.params.bypass_mounting_requirements = true;
			perform_transfer(request, step);
			break;
		}
	}

	return true;
}


entity_id arena_mode::create_character_for_player(
	const input_type in, 
	const logic_step step,
	const mode_player_id id,
	const std::optional<transfer_meta> transferred
) {
	auto access = allocate_new_entity_access();

	if (auto player_data = find(id)) {
		auto& p = *player_data;
		auto& cosm = in.cosm;

		const auto handle = [&]() {
			const auto faction = p.get_faction();

			if (faction == faction_type::SPECTATOR) {
				return cosm[entity_id()];
			}

			if (const auto flavour = ::find_faction_character_flavour(cosm, faction); flavour.is_set()) {
				auto new_player = just_create_entity(
					access,
					cosm, 
					entity_flavour_id(flavour), 
					[](entity_handle) {},
					[&](const entity_handle new_character) {
						teleport_to_next_spawn(in, new_character);
						init_spawned(in, id, new_character, step, transferred);
					}
				);

				return new_player;
			}

			return cosm[entity_id()];
		}();

		if (handle.alive()) {
			cosmic::set_specific_name(handle, p.get_nickname());

			if (const auto sentience = handle.template find<components::sentience>()) {
				if (in.rules.enable_player_colors) {
					sentience->last_assigned_color = p.assigned_color;
				}
				else {
					sentience->last_assigned_color = in.rules.default_player_color;
				}
			}

			p.controlled_character_id = handle;

			if (state == arena_mode_state::LIVE) {
				if (get_freeze_seconds_left(in) > 0.f) {
					handle.set_frozen(true);
				}
			}

			if (state == arena_mode_state::WARMUP) {
				if (get_warmup_seconds_left(in) <= 0.f) {
					handle.set_frozen(true);
				}
			}

			return p.controlled_character_id;
		}
		else {
			p.controlled_character_id.unset();
		}
	}
	
	return entity_id::dead();
}

void arena_mode::play_start_round_sound(const input_type in, const const_logic_step step) { 
	const auto start_event = 
		is_first_round_in_half(in) ?
		battle_event::PREPARE_TO_FIGHT :
		battle_event::START
	;

	if (start_event == battle_event::PREPARE_TO_FIGHT) {
		// Custom logic: Distribute "PREPARE" sounds evenly

		const auto p = calc_participating_factions(in);

		p.for_each([&](const faction_type t) {
			if (const auto sound_id = in.rules.view.event_sounds[t][start_event]; sound_id.is_set()) {
				sound_effect_input effect;
				effect.id = sound_id;
				effect.modifier.always_direct_listener = true;

				sound_effect_start_input input;
				input.variation_number = prepare_to_fight_counter;
				input.listener_faction = t;

				effect.start(step, input, always_predictable_v);
			}
		});

		++prepare_to_fight_counter;
	}
	else {
		play_sound_for(in, step, start_event, always_predictable_v);
	}
}

void arena_mode::setup_round(
	const input_type in, 
	const logic_step step, 
	const arena_mode::round_transferred_players& transfers
) {
	auto access = allocate_new_entity_access();

	clear_players_round_state(in);

	auto& cosm = in.cosm;
	clock_before_setup = cosm.get_clock();

	const auto former_ids = [&]() {
		std::unordered_map<mode_player_id, entity_id> result;

		for (const auto& p : players) {
			const auto new_id = cosm[p.second.controlled_character_id].get_id();

			if (new_id.is_set()) {
				result.try_emplace(p.first, new_id);
			}
		}

		return result;
	}();

	round_speeds = in.rules.speeds;

	cosm.set(in.clean_round_state);

	/* 
		If there are any entries in message queues, 
		they become invalid when we assign the clean_round_state.
	*/

	step.transient.flush_everything();

	cosm.set_fixed_delta(round_speeds.calc_fixed_delta());

	remove_test_characters(cosm);

	if (
		(in.rules.delete_lying_items_on_warmup && state == arena_mode_state::WARMUP)
		|| in.rules.delete_lying_items_on_round_start
	) {
		remove_all_dropped_items(cosm);
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

	spawn_and_kick_bots(in, step);
	spawn_characters_for_recently_assigned(in, step);

	if (msg.changes.size() > 0) {
		step.post_message(msg);
	}

	if (in.rules.freeze_secs > 0.f) {
		if (state != arena_mode_state::WARMUP) {
			set_players_frozen(in, true);
			release_triggers_of_weapons_of_players(in);
		}
	}
	else {
		play_start_round_sound(in, step);
	}

	if (state != arena_mode_state::WARMUP) {
		if (in.rules.has_bomb_mechanics()) {
			if (!give_bomb_to_random_player(in, step)) {
				spawn_bomb_near_players(in);
			}
		}
	}

	if (state == arena_mode_state::WARMUP) {
		const auto theme = in.rules.view.warmup_theme;

		just_create_entity(access, cosm, theme);
	}

	step.post_message(messages::hud_message { messages::special_hud_command::CLEAR });
}

arena_mode::round_transferred_players arena_mode::make_transferred_players(const input_type in) const {
	round_transferred_players result;

	const auto& cosm = in.cosm;

	for (const auto& it : players) {
		const auto id = it.first;
		const auto& player_data = it.second;

		if (const auto handle = cosm[player_data.controlled_character_id]) {
			auto& pm = result[id];
			pm.movement = handle.get<components::movement>().flags;

			if (::sentient_and_unconscious(handle)) {
				continue;
			}

			if (const auto sentience = handle.find<components::sentience>()) {
				pm.saved_spells = sentience->learnt_spells;
			}

			auto& eq = pm.saved_eq;
			auto& items = eq.items;
			pm.survived = true;

			std::unordered_map<entity_id, std::size_t> id_to_container_idx;

			handle.for_each_contained_item_recursive(
				[&](const auto& typed_item) {
					const auto flavour_id = typed_item.get_flavour_id();

					if (entity_flavour_id(flavour_id) == entity_flavour_id(in.rules.bomb_flavour)) {
						return recursive_callback_result::CONTINUE_DONT_RECURSE;
					}

					const auto slot = typed_item.get_current_slot();
					const auto container_id = slot.get_container().get_id();
					const auto container_index = mapped_or_default(
						id_to_container_idx, 
						container_id, 
						static_cast<std::size_t>(-1)
					);
					
					if (container_index == static_cast<std::size_t>(-1)) {
						//ensure_eq(handle.get_id(), container_id);
					}

					if (typed_item.template has<invariants::container>()) {
						const auto new_idx = eq.items.size();
						id_to_container_idx.try_emplace(typed_item.get_id(), new_idx);
					}

					const auto source_entity_id = typed_item.get_id();
					const auto& item = typed_item.template get<components::item>();

					items.push_back({ flavour_id, item.get_charges(), item.get_raw_component().owner_meta, container_index, slot.get_type(), source_entity_id });

					return recursive_callback_result::CONTINUE_AND_RECURSE;
				}
			);
		}
	}

	return result;
}

void arena_mode::start_next_round(const input_type in, const logic_step step, const round_start_type type) {
	state = arena_mode_state::LIVE;

	if (type == round_start_type::KEEP_EQUIPMENTS) {
		setup_round(in, step, make_transferred_players(in));
	}
	else {
		setup_round(in, step);
	}
}

mode_player_id arena_mode::lookup(const entity_id& controlled_character_id) const {
	for (const auto& p : players) {
		if (p.second.controlled_character_id == controlled_character_id) {
			return p.first;
		}
	}

	return mode_player_id::dead();
}

arena_mode_player_stats* arena_mode::stats_of(const mode_player_id& id) {
	if (const auto p = find(id)) {
		return std::addressof(p->stats);
	}

	return nullptr;
}

template <class E>
static void delete_all_owned_items(const E handle) {
	deletion_queue q;

	auto& cosm = handle.get_cosmos();

	handle.for_each_contained_item_recursive(
		[&](const auto& contained) {
			q.push_back(entity_id(contained.get_id()));
		}
	);

	::reverse_perform_deletions(q, cosm);
}

template <class H>
void arena_mode::reset_equipment_for(const logic_step step, const input_type in, const mode_player_id id, H player_handle) {
	auto access = allocate_new_entity_access();

	if (const auto player_data = find(id)) {
		auto& stats = player_data->stats;

		if (!levelling_enabled(in)) {
			return;
		}

		if (auto gg = std::get_if<gun_game_rules>(&in.rules.subrules)) {
			const bool is_final_level = stats.level == gg->get_final_level();

			::delete_all_owned_items(player_handle);

			if (auto transfers = player_handle.template find<components::item_slot_transfers>()) {
				transfers->allow_drop_and_pick = false;

				if (is_final_level) {
					transfers->allow_melee_throws = gg->can_throw_melee_on_final_level;
				}
				else {
					transfers->allow_melee_throws = true;
				}
			}

			const auto faction = player_data->get_faction();

			if (stats.level < static_cast<int>(gg->progression.size())) {
				const auto next = gg->progression[stats.level];

				auto eq = gg->basic_eq[faction];
				eq.perform_recoils = false;
				eq.weapon = next;
				eq.generate_for(access, player_handle, step, 1);
			}
			else {
				if (is_final_level) {
					auto eq = gg->final_eq[faction];
					eq.perform_recoils = false;
					eq.generate_for(access, player_handle, step, 1);
				}
				else {
					/* Ignore if we're above the final level as this basically means a win. */
				}
			}
		}
	}
}

void arena_mode::count_knockout(const logic_step step, const input_type in, const entity_id victim, const components::sentience& sentience) {
	const auto& cosm = in.cosm;
	const auto& clk = cosm.get_clock();
	const auto& origin = sentience.knockout_origin;
	const auto victim_handle = cosm[victim];

	if (victim_handle.dead()) {
		return;
}

	const auto knockouter = [&]() {
		if (const auto candidate = origin.get_guilty_of_damaging(victim_handle)) {
			return candidate;
		}

		LOG("The knockouter had disconnected before the kill occurred. Counting this a suicide.");

		return victim_handle;
	}();

	auto assists = sentience.damage_owners;

	arena_mode_knockout ko;

	auto make_participant = [&](auto& participant, const auto& handle) {
		participant = {};

		if (const auto mode_id = lookup(handle.get_id()); mode_id.is_set()) {
			if (const auto player_data = find(mode_id)) {
				participant.id = mode_id;
				participant.name = player_data->get_nickname();
				participant.faction = player_data->get_faction();
			}
		}
	};

	for (const auto& candidate : assists) {
		if (const auto candidate_assistant = cosm[candidate.who]) {
			if (candidate_assistant != knockouter) {
				if (candidate.applied_damage >= in.rules.minimal_damage_for_assist) {
					make_participant(ko.assist, candidate_assistant);
					break;
				}
			}
		}
	}

	ko.when = clk;
	ko.origin = origin;

	make_participant(ko.knockouter, knockouter);
	make_participant(ko.victim, victim_handle);

	count_knockout(step, in, ko);
}

void arena_mode::count_knockout(const logic_step step, const input_type in, const arena_mode_knockout ko) {
	current_round.knockouts.push_back(ko);

	auto announce_knockout = [&](const auto stats, const auto knockouts_dt) {
		const auto controlled_character_id = lookup(ko.knockouter.id);

		if (state == arena_mode_state::WARMUP) {
			return;
		}

		if (knockouts_dt > 0) {
			if (const auto controlled = in.cosm[controlled_character_id]) {
				/* Count streaks only if we are conscious, not ones post-mortem */

				if (sentient_and_conscious(controlled)) {
					stats->knockout_streak += knockouts_dt;
				}

				/*
					Priority:
					- First Blood
					- Streak
					- Headshot
					- Humiliation
				*/

				if (had_first_blood) {
					const auto num_kos = stats->knockout_streak;
					const auto current_streak = in.rules.view.find_streak(num_kos);

					bool streak_messaged = false;

					auto preffix = std::string("");

					if (ko.origin.circumstances.headshot) {
						preffix = "[color=orange]::HEADSHOT:: [/color]";
					}
					else if (ko.origin.cause.is_humiliating(in.cosm)) {
						preffix = "[color=orange]HUMILIATION!!! [/color]";
					}

					if (current_streak == nullptr && in.rules.view.past_all_streaks(num_kos)) {
						hud_message_1_player(step, preffix, typesafe_sprintf(" is on a killstreak with [color=orange]%x[/color] kills!", num_kos), find(ko.knockouter.id), true);
						streak_messaged = true;
					}

					if (current_streak) {
						play_sound_globally(step, current_streak->announcement_sound, never_predictable_v);
						hud_message_1_player(step, preffix, ": " + current_streak->message, find(ko.knockouter.id), true);
					}
					else if (ko.origin.circumstances.headshot) {
						play_sound_for(in, step, battle_event::HEADSHOT, never_predictable_v);

						if (!streak_messaged) {
							hud_message_2_players(step, "[color=orange]::HEADSHOT:: [/color]", " just owned ", " !", find(ko.knockouter.id), find(ko.victim.id), true);
						}
					}
					else if (ko.origin.cause.is_humiliating(in.cosm)) {
						play_sound_for(in, step, battle_event::HUMILIATION, never_predictable_v);

						if (!streak_messaged) {
							hud_message_2_players(step, "[color=orange]HUMILIATION!!! [/color]", " sliced and diced ", " !", find(ko.knockouter.id), find(ko.victim.id), true);
						}
					}
				}
			}
		}
	};

	auto announce_death = [&](const auto stats) {
		stats->knockout_streak = 0;

		if (state == arena_mode_state::WARMUP) {
			return;
		}

		if (!had_first_blood) {
			play_sound_for(in, step, battle_event::FIRST_BLOOD, never_predictable_v);
			hud_message_2_players(step, "[color=orange]FIRST BLOOD!![/color] ", " drew first blood on ", " !", find(ko.knockouter.id), find(ko.victim.id), true);

			had_first_blood = true;
		}

		if (in.rules.respawn_after_ms > 0.0f) {
			return;
		}

		if (const auto victim_info = find(ko.victim.id)) {
			const auto victim_faction = victim_info->get_faction();

			if (1 == num_conscious_players_in(in.cosm, victim_faction)) {
				const auto p = calc_participating_factions(in);

				bool any_enemy_has_nonzero = false;
				bool any_enemy_has_more_than_one = false;

				std::optional<int> victim_faction_hp;
				std::optional<mode_player_id> victim_faction_id;

				for_each_player_handle_in(in.cosm, victim_faction, [&](const auto& handle) {
					if (sentient_and_conscious(handle)) {
						victim_faction_hp = std::max(1, static_cast<int>(handle.template get<components::sentience>().template get<health_meter_instance>().value));
						victim_faction_id = lookup(handle.get_id());
					}
				});

				std::optional<int> enemy_hp;
				std::optional<mode_player_id> enemy_id;
				
				int total_enemies = 0;

				p.for_each([&](const auto faction) {
					if (faction != victim_faction) {
						const auto n = num_conscious_players_in(in.cosm, faction);

						total_enemies += n;

						if (n > 0) {
							any_enemy_has_nonzero = true;
						}

						if (n > 1) {
							any_enemy_has_more_than_one = true;
						}

						if (n == 1) {
							for_each_player_handle_in(in.cosm, faction, [&](const auto& handle) {
								if (sentient_and_conscious(handle)) {
									enemy_hp = std::max(1, static_cast<int>(handle.template get<components::sentience>().template get<health_meter_instance>().value));
									enemy_id = lookup(handle.get_id());
								}
							});
						}
					}
				});

				if (any_enemy_has_more_than_one) {
					play_faction_sound_for(in, step, battle_event::ONE_VERSUS_MANY, victim_faction, never_predictable_v);

					if (victim_faction_id.has_value()) {
						hud_message_1_player(step, "", typesafe_sprintf(" is clutching against [color=orange]%x[/color] enemies! [color=yellow]All depends on you![/color]", total_enemies), find(*victim_faction_id), true);
					}
				}
				else if (any_enemy_has_nonzero) {
					/* All enemies have at most one. It's a duel/truel situation. */
					play_sound_for(in, step, battle_event::ONE_VERSUS_ONE, never_predictable_v);

					if (victim_faction_id.has_value() && enemy_id.has_value()) {
						auto get_hp_col = [&](const auto hp) {
							if (hp > 70.f) {
								return "lightgreen";
							}

							if (hp > 40.f) {
								return "lightyellow";
							}

							if (hp > 20.f) {
								return "orange";
							}

							return "lightred";
						};

						hud_message_2_players(step, "[color=yellow]..::THE FINAL DUEL::.. [/color]", typesafe_sprintf("[color=%x] (%x HP)[/color] VS ", get_hp_col(*victim_faction_hp), *victim_faction_hp), typesafe_sprintf("[color=%x] (%x HP)[/color] !", get_hp_col(enemy_hp), enemy_hp), find(*victim_faction_id), find(*enemy_id), true);

					}
				}
			}
		}
	};

	{
		int knockouts_dt = 1;

		if (ko.knockouter.id == ko.victim.id) {
			knockouts_dt = 0;
		}
		else if (ko.knockouter.faction == ko.victim.faction) {
			knockouts_dt = -1;
			give_monetary_award(in, ko.knockouter.id, in.rules.economy.team_kill_penalty * -1);
		}

		if (knockouts_dt > 0) {
			auto& cosm = in.cosm;

			const auto award = ko.origin.on_tool_used(cosm, [&](const auto& tool) -> std::optional<money_type> {
				if constexpr(is_spell_v<decltype(tool)>) {
					return tool.common.adversarial.knockout_award;
				}
				else if constexpr(is_nullopt_v<decltype(tool)>) {
					return std::nullopt;
				}
				else {
					return ::get_knockout_award(cosm, tool);
				}
			});

			if (award.has_value()) {
				give_monetary_award(in, ko.knockouter.id, *award);
			}
		}

		if (const auto s = stats_of(ko.knockouter.id)) {
			s->knockouts += knockouts_dt;

			announce_knockout(s, knockouts_dt);

			if (levelling_enabled(in)) {
				requested_equipment final_eq;

				const auto final_level = std::visit([&]<typename S>(const S& subrules) {
					if constexpr(std::is_same_v<S, gun_game_rules>) {
						final_eq = subrules.final_eq[ko.knockouter.faction];
						return subrules.get_final_level();
					}

					return 0;
				}, in.rules.subrules);

				auto before_level = s->level;

				if (knockouts_dt > 0) {
					bool weapon_requirement_met = true;

					if (s->level == final_level) {
						weapon_requirement_met  = ko.origin.on_tool_used(in.cosm, [&](const auto& tool) {
							if constexpr(is_nullopt_v<decltype(tool)>) {

							}
							else if constexpr(is_spell_v<decltype(tool)>) {

							}
							else {
								if (!final_eq.has_weapon(entity_flavour_id(tool))) {
									return false;
								}
							}

							return true;
						});
					}

					if (weapon_requirement_met) {
						const bool tool_humiliating = ko.origin.cause.is_humiliating(in.cosm);

						if (tool_humiliating) {
							s->level += 2;

							/* Prevent jumping from the one before final straight to victory */
							if (s->level == final_level + 1) {
								s->level = final_level;
							}

							if (const auto v = stats_of(ko.victim.id)) {
								v->level -= 1;
								v->level = std::max(0, v->level);
							}
						}
						else {
							s->level += 1;
						}
					}
				}
				else if (knockouts_dt < 0) {
					s->level -= 1;
					s->level = std::max(0, s->level);
				}

				if (before_level != s->level) {
					const bool victory_already = s->level > final_level;

					if (victory_already) {
						standard_victory(in, step, ko.knockouter.faction);
					}
					else {
						on_player_handle(in.cosm, ko.knockouter.id, [&](const auto& player_handle) {
							if constexpr(!is_nullopt_v<decltype(player_handle)>) {
								if (s->level > before_level) {
									auto start = sound_effect_start_input::at_listener(player_handle);
									start.listener_faction = ko.knockouter.faction;

									in.rules.view.level_up_sound.start(step, start, never_predictable_v);
								}

								reset_equipment_for(step, in, ko.knockouter.id, player_handle);
							}
						});
					}
				}
			}
		}

		if (const auto s = stats_of(ko.victim.id)) {
			s->deaths += 1;
			announce_death(s);
		}
	}

	if (ko.assist.id.is_set()) {
		int assists_dt = 1;

		if (ko.assist.faction == ko.victim.faction) {
			assists_dt = -1;
		}

		if (const auto s = stats_of(ko.assist.id)) {
			s->assists += assists_dt;
		}
	}
}

bool arena_mode::is_halfway_round(const const_input_type in) const {
	const auto max_rounds = in.rules.get_num_rounds();
	const auto current_round = get_current_round_number();

	return current_round == max_rounds / 2;
}

bool arena_mode::is_final_round(const const_input_type in) const {
	const auto max_rounds = in.rules.get_num_rounds();
	const auto current_round = get_current_round_number();

	bool someone_has_over_half = false;

	const auto p = calc_participating_factions(in);
	
	p.for_each([&](const auto f) {
		if (get_score(f) > max_rounds / 2) {
			someone_has_over_half = true;
		}
	});

	return someone_has_over_half || current_round >= max_rounds;
}

void arena_mode::count_win(const input_type in, const const_logic_step step, const faction_type winner) {
	const auto p = calc_participating_factions(in);
	const auto loser = winner == p.defusing ? p.bombing : p.defusing;

	++factions[winner].score;
	factions[winner].consecutive_losses = 0;

	state = arena_mode_state::ROUND_END_DELAY;
	current_round.last_win = { in.cosm.get_clock(), winner };

	auto& consecutive_losses = factions[loser].consecutive_losses;
	++consecutive_losses;

	auto winner_award = in.rules.economy.winning_faction_award;
	auto loser_award = in.rules.economy.losing_faction_award;

	if (consecutive_losses > 1) {
		loser_award += std::min(
			consecutive_losses - 1, 
			in.rules.economy.max_consecutive_loss_bonuses
		) * in.rules.economy.consecutive_loss_bonus;
	}

	if (loser == p.bombing) {
		if (current_round.bomb_planter.is_set()) {
			loser_award += in.rules.economy.lost_but_bomb_planted_team_bonus;
			winner_award += in.rules.economy.defused_team_bonus;
		}
	}
	
	for (auto& p : players) {
		const auto& player_id = p.first;
		const auto faction = p.second.get_faction();

		give_monetary_award(in, player_id, faction == winner ? winner_award : loser_award);
	}

	if (is_halfway_round(in) || is_final_round(in)) {
		state = arena_mode_state::MATCH_SUMMARY;
		set_players_frozen(in, true);
		release_triggers_of_weapons_of_players(in);

		if (is_final_round(in)) {
			report_match_result(in, step);
		}
	}
}

void arena_mode::play_sound_globally(const const_logic_step step, const assets::sound_id sound_id, const predictability_info info) const {
	sound_effect_input effect;
	effect.id = sound_id;
	effect.modifier.always_direct_listener = true;

	sound_effect_start_input input;
	input.variation_number = get_step_rng_seed(step.get_cosmos());

	effect.start(step, input, info);
}

void arena_mode::play_faction_sound(const const_logic_step step, const faction_type f, const assets::sound_id id, const predictability_info info) const {
	sound_effect_input effect;
	effect.id = id;
	effect.modifier.always_direct_listener = true;

	sound_effect_start_input input;
	input.variation_number = get_step_rng_seed(step.get_cosmos());
	input.listener_faction = f;

	effect.start(step, input, info);
}

void arena_mode::play_win_theme(const input_type in, const const_logic_step step, const faction_type winner) const {
	if (const auto sound_id = in.rules.view.win_themes[winner]; sound_id.is_set()) {
		sound_effect_input effect;
		effect.id = sound_id;
		effect.modifier.always_direct_listener = true;

		sound_effect_start_input input;
		input.variation_number = get_step_rng_seed(step.get_cosmos());

		effect.start(step, input, never_predictable_v);
	}
}

void arena_mode::play_win_sound(const input_type in, const const_logic_step step, const faction_type winner) const {
	const auto p = calc_participating_factions(in);

	p.for_each([&](const faction_type t) {
		if (const auto sound_id = in.rules.view.win_sounds[t][winner]; sound_id.is_set()) {
			play_faction_sound(step, t, sound_id, never_predictable_v);
		}
	});
}

void arena_mode::play_sound_for(const input_type in, const const_logic_step step, const battle_event event, const predictability_info info) const {
	const auto p = calc_participating_factions(in);

	p.for_each([&](const faction_type t) {
		play_faction_sound_for(in, step, event, t, info);
	});
}

void arena_mode::play_faction_sound_for(const input_type in, const const_logic_step step, const battle_event event, const faction_type t, const predictability_info info) const {
	if (const auto sound_id = in.rules.view.event_sounds[t][event]; sound_id.is_set()) {
		play_faction_sound(step, t, sound_id, info);
	}
}

void arena_mode::play_bomb_defused_sound(const input_type in, const const_logic_step step, const faction_type winner) const {
	const auto p = calc_participating_factions(in);

	p.for_each([&](const faction_type t) {
		auto msg = messages::start_multi_sound_effect(never_predictable_v);

		{
			auto& start = msg.payload.start;

			start.listener_faction = t;
			start.variation_number = get_round_rng_seed(in.cosm);
		}

		auto& effects = msg.payload.inputs;
		effects.reserve(2);

		if (const auto defused_id = in.rules.view.event_sounds[t][battle_event::BOMB_DEFUSED]; defused_id.is_set()) {
			sound_effect_input effect;
			effect.id = defused_id;

			effects.emplace_back(std::move(effect));
		}

		if (const auto win_id = in.rules.view.win_sounds[t][winner]; win_id.is_set()) {
			sound_effect_input effect;
			effect.id = win_id;

			effects.emplace_back(std::move(effect));
		}

		step.post_message(msg);
	});
}

void arena_mode::standard_victory(const input_type in, const const_logic_step step, const faction_type winner, const bool announce, const bool play_theme) {
	count_win(in, step, winner);

	if (play_theme) {
		play_win_theme(in, step, winner);
	}

	if (announce) {
		play_win_sound(in, step, winner);
	}
}

void arena_mode::process_win_conditions(const input_type in, const logic_step step) {
	auto& cosm = in.cosm;

	const auto p = calc_participating_factions(in);

	auto victory_for = [&](const auto winner) {
		standard_victory(in, step, winner);
	};

	if (in.rules.has_bomb_mechanics()) {
		/* Bomb-based win-conditions */

		auto stop_bomb_detonation_theme = [&]() {
			if (cosm[bomb_detonation_theme]) {
				step.queue_deletion_of(bomb_detonation_theme, "Bomb detonation theme interrupted.");
				bomb_detonation_theme.unset();
			}
		};

		if (bomb_exploded(in)) {
			stop_bomb_detonation_theme();
			const auto planting_player = current_round.bomb_planter;

			if (const auto s = stats_of(planting_player)) {
				s->bomb_explosions += 1;
			}

			give_monetary_award(in, planting_player, in.rules.economy.bomb_explosion_award);
			victory_for(p.bombing);
			return;
		}

		if (const auto character_who_defused = cosm[get_character_who_defused_bomb(in)]) {
			stop_bomb_detonation_theme();
			const auto winner = p.defusing;
			const auto defusing_player = lookup(character_who_defused);

			if (const auto s = stats_of(defusing_player)) {
				s->bomb_defuses += 1;
			}

			give_monetary_award(in, defusing_player, in.rules.economy.bomb_defuse_award);
			standard_victory(in, step, winner, false);
			play_bomb_defused_sound(in, step, winner);
			return;
		}

		if (!bomb_planted(in) && get_round_seconds_left(in) <= 0.f) {
			/* Time-out */
			victory_for(p.defusing);
			return;
		}

		/* Kill-based win-conditions */

		if (num_players_in(p.bombing) > 0) {
			if (!bomb_planted(in) && 0 == num_conscious_players_in(cosm, p.bombing)) {
				/* All bombing players have been neutralized. */
				victory_for(p.defusing);
				return;
			}
		}

		if (num_players_in(p.defusing) > 0) {
			if (0 == num_conscious_players_in(cosm, p.defusing)) {
				/* All defusing players have been neutralized. */
				victory_for(p.bombing);
				return;
			}
		}
	}

#if 0
	auto process_subrules_conditions = [&](const S& subrules) {
		if constexpr(std::is_same_v<S, gun_game_rules>) {

		}
	};

	std::visit(process_subrules_conditions, in.rules.subrules);
#endif
}

void arena_mode::swap_assigned_factions(const arena_mode::participating_factions& p) {
	for (auto& it : players) {
		auto& player_data = it.second;
		auto& faction = player_data.session.faction;
		faction = p.get_swapped(faction);
	}
}

void arena_mode::scramble_assigned_factions(const arena_mode::participating_factions& p) {
	auto rng = randomization(scramble_counter++);

	std::vector<arena_mode_player*> ptrs;
	ptrs.reserve(players.size());

	for (auto& it : players) {
		ptrs.push_back(std::addressof(it.second));
	}

	shuffle_range(ptrs, rng);

	const auto off = rng.randval(0, 1); 

	for (std::size_t i = 0; i < ptrs.size(); ++i) {
		ptrs[i]->session.faction = (i + off) % 2 ? p.defusing : p.bombing;
	}
}

void arena_mode::handle_special_commands(const input_type in, const mode_entropy& entropy, const logic_step step) {
	const auto& g = entropy.general;

	std::visit(
		[&](const auto& cmd) {
			using C = remove_cref<decltype(cmd)>;

			if constexpr(std::is_same_v<C, std::monostate>) {

			}
			else if constexpr(std::is_same_v<C, match_command>) {
				switch (cmd) {
					case C::RESTART_MATCH:
						restart_match(in, step);
						break;

					case C::RESTART_MATCH_NO_WARMUP:
						restart_match(in, step);
						end_warmup_and_go_live(in, step);
						break;

					case C::SWAP_TEAMS:
						swap_assigned_factions(calc_participating_factions(in));
						restart_match(in, step);
						break;

					case C::SCRAMBLE_TEAMS:
						scramble_assigned_factions(calc_participating_factions(in));
						restart_match(in, step);
						break;

					case C::TEST_ANNOUNCE_DUEL:
						check_duel_of_honor(in, step);
						break;

					case C::TEST_ANNOUNCE_MATCH_RESULT:
						report_match_result(in, step);
						break;

					default: break;
				}
			}
			else {
				static_assert(always_false_v<C>, "Unhandled command type!");
			}
		},
		g.special_command
	);
}

arena_migrated_session arena_mode::emigrate() const {
	arena_migrated_session session;

	for (const auto& emigrated_player : players) {
		arena_migrated_player_entry entry;
		entry.mode_id = emigrated_player.first;
		entry.data = emigrated_player.second.session;

		session.players.emplace_back(std::move(entry));
	}

	session.next_session_id = next_session_id;
	return session;
}

void arena_mode::migrate(const input_type in, const arena_migrated_session& session) {
	ensure(players.empty());
	ensure_eq(0u, get_current_round_number());

	std::map<faction_type, faction_type> faction_mapping;

	for (const auto& migrated_player : session.players) {
		const auto f = migrated_player.data.faction;

		if (f != faction_type::SPECTATOR) {
			faction_mapping[f] = f;
		}
	}

	const auto participating = calc_participating_factions(in).get_all();
	std::size_t i = 0;

	for (auto& f : faction_mapping) {
		const auto new_faction = participating[std::min(participating.size() - 1, i)];

		LOG("Migrating faction: %x to faction: %x", format_enum(f.second), format_enum(new_faction));
		f.second = new_faction;
		++i;
	}

	faction_mapping[faction_type::SPECTATOR] = faction_type::SPECTATOR;

	for (const auto& migrated_player : session.players) {
		const auto mode_id = migrated_player.mode_id;
		const auto& it = players.try_emplace(mode_id);
		auto& new_player = (*it.first).second;

		ensure(it.second);
		ensure(!new_player.is_set());

		new_player.session = migrated_player.data;

		new_player.session.faction = faction_mapping.at(new_player.session.faction);
		on_faction_changed_for(in, faction_type::DEFAULT, mode_id);
		LOG("Moving %x to %x", new_player.get_nickname(), format_enum(new_player.get_faction()));
	}

	next_session_id = session.next_session_id;
}

void arena_mode::add_or_remove_players(const input_type in, const mode_entropy& entropy, const logic_step step) {
	const auto& g = entropy.general;

	if (logically_set(g.added_player)) {
		const auto& a = g.added_player;
		const auto result = add_player_custom(in, a);
		(void)result;

		if (const auto entry = find(a.id)) {
			messages::mode_notification notification;

			notification.subject_mode_id = a.id;
			notification.subject_name = entry->get_nickname();
			notification.payload = messages::joined_or_left::JOINED;

			step.post_message(std::move(notification));
		}

		if (a.faction == faction_type::DEFAULT) {
			auto_assign_faction(in, a.id);
		}
		else {
			choose_faction(in, a.id, a.faction);
		}
	}

	if (logically_set(g.removed_player)) {
		if (const auto entry = find(g.removed_player)) {
			messages::mode_notification notification;

			notification.subject_mode_id = g.removed_player;
			notification.subject_name = entry->get_nickname();
			notification.payload = messages::joined_or_left::LEFT;

			step.post_message(std::move(notification));
		}

		remove_player(in, step, g.removed_player);
	}

	if (logically_set(g.removed_player) && logically_set(g.added_player)) {
		ensure(g.removed_player != g.added_player.id);
	}
}

mode_player_id arena_mode::find_first_free_player() const {
	return first_free_key(players, mode_player_id::first());
}

void arena_mode::execute_player_commands(const input_type in, mode_entropy& entropy, const logic_step step) {
	auto access = allocate_new_entity_access();

	const auto current_round = get_current_round_number();
	auto& cosm = in.cosm;

	for (const auto& p : entropy.players) {
		const auto& command_variant = p.second;
		const auto id = p.first;

		if (const auto player_data = find(id)) {
			auto command_callback = [&](const auto& typed_command) {
				using namespace mode_commands;
				using C = remove_cref<decltype(typed_command)>;

				if constexpr(is_monostate_v<C>) {

				}
				else if constexpr(is_one_of_v<C, item_purchase, spell_purchase, special_purchase>) {
					if (get_buy_seconds_left(in) <= 0.f) {
						return;
					}

					on_player_handle(cosm, id, [&](const auto& player_handle) {
						if constexpr(!is_nullopt_v<decltype(player_handle)>) {
							if (!buy_area_in_range(player_handle)) {
								return;
							}

							auto& stats = player_data->stats;
							auto& money = stats.money;
							auto& done_purchases = stats.round_state.done_purchases;

							if constexpr(std::is_same_v<C, item_purchase>) {
								const auto f_id = typed_command;

								if (!::is_alive(cosm, f_id)) {
									return;
								}

								if (!factions_compatible(player_handle, f_id)) {
									return;
								}

								const auto price = ::find_price_of(cosm, f_id);
								const bool price_correct = price && *price != 0;

								if (!price_correct) {
									return;
								}

								if (money >= *price) {
									if (::num_carryable_pieces(player_handle, ::get_buy_slot_opts(), f_id) == 0) {
										return;
									}

									requested_equipment eq;

									if (::is_magazine_like(cosm, f_id)) {
										eq.non_standard_mag = f_id;
										eq.num_given_ammo_pieces = 1;
									}
									else if (::is_armor_like(cosm, f_id)) {
										eq.armor_wearable = f_id;
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

									const auto result_weapon = eq.generate_for(access, player_handle, step);

									if (const auto w = cosm[result_weapon]; w.alive() && ::is_weapon_like(w)) {
										auto requested_wielding = wielding_setup::bare_hands();
										requested_wielding.hand_selections[0] = w;

										::perform_wielding(
											step,
											player_handle,
											requested_wielding
										);
									}

									money -= *price;
								}

								return;
							}
							else if constexpr(std::is_same_v<C, spell_purchase>) {
								const auto s_id = typed_command;
								
								if (!::is_alive(cosm, s_id)) {
									return;
								}

								if (!factions_compatible(player_handle, s_id)) {
									return;
								}

								const auto price = ::find_price_of(cosm, s_id);
								const bool price_correct = price && *price != 0;

								if (!price_correct) {
									return;
								}

								const auto& sentience = player_handle.template get<components::sentience>();

								if (money >= *price && !sentience.is_learnt(s_id)) {
									requested_equipment eq;

									eq.spells_to_give[s_id.get_index()] = true;
									eq.generate_for(access, player_handle, step);

									::play_learnt_spell_effect(player_handle, s_id, step);

									money -= *price;
								}
							}
							else if constexpr(std::is_same_v<C, special_purchase_request>) {
								switch (typed_command) {
									case special_purchase_request::REBUY_PREVIOUS: {

										break;
									}

									default: break;
								}
							}
							else {
								static_assert(always_false_v<C>, "Non-exhaustive std::visit");
							}
						}
						else {
							(void)player_handle;
						}
					});
				}
				else if constexpr(std::is_same_v<C, team_choice>) {
					const auto previous_faction = player_data->get_faction();

					const auto requested_faction = typed_command;

					if (requested_faction == faction_type::COUNT) {
						return;
					}

					// TODO: Notify GUI about the result.

					using R = messages::health_event;

					const auto death_request = on_player_handle(cosm, id, [&](const auto& player_handle) -> std::optional<R> {
						if constexpr(!is_nullopt_v<decltype(player_handle)>) {
							if (const auto tr = player_handle.find_logic_transform()) {
								if (const auto sentience = player_handle.template find<components::sentience>()) {
									if (!sentience->is_dead()) {
										damage_origin origin;
										origin.cause.flavour = player_handle.get_flavour_id();
										origin.cause.entity = player_handle.get_id();
										origin.sender.set(player_handle);

										return R::request_death(
											player_handle.get_id(),
											tr->get_direction(),
											tr->pos,
											origin
										);
									}
								}
							}
						}

						(void)player_handle;
						return std::nullopt;
					});

					const auto result = [&]() {
						if (previous_faction == requested_faction) {
							return faction_choice_result::THE_SAME;
						}

						if (in.rules.forbid_going_to_spectator_unless_character_alive) {
							if (death_request == std::nullopt) {
								if (requested_faction == faction_type::SPECTATOR) {
									return faction_choice_result::NEED_TO_BE_ALIVE_FOR_SPECTATOR;
								}
							}
						}

						if (previous_faction == faction_type::SPECTATOR) {
							const auto game_limit = get_max_num_active_players(in);

							if (game_limit && get_num_active_players() >= game_limit) {
								return faction_choice_result::TEAM_IS_FULL;
							}
						}

						if (requested_faction != faction_type::SPECTATOR) {
							/* This is a serious choice */

							{
								const auto& team_limit = in.rules.max_players_per_team;

								if (team_limit && num_players_in(requested_faction) >= team_limit) {
									return faction_choice_result::TEAM_IS_FULL;
								}
							}

							if (death_request == std::nullopt) {
								/* If we are already dead, don't allow to re-choose twice during the same round. */

								if (player_data->round_when_chosen_faction == current_round) {
									return faction_choice_result::CHOOSING_TOO_FAST;
								}
							}

							player_data->round_when_chosen_faction = current_round;
						}

						if (requested_faction == faction_type::DEFAULT) {
							/* Auto-select */
							return auto_assign_faction(in, id);
						}

						return choose_faction(in, id, requested_faction);
					}();

					const auto final_faction = player_data->get_faction();

					if (result == faction_choice_result::CHANGED) {
						/* 
							Note: moving to AFK is also implemented in terms of mode_commands::team choice,
							so in practice, there are no more ways to defect the duel.
						*/

						handle_duel_desertion(in, step, id);

						LOG("Changed from %x to %x", format_enum(previous_faction), format_enum(final_faction));

						if (death_request.has_value()) {
							step.post_message(*death_request);
						}
					}

					messages::faction_choice choice;
					choice.result = result;
					choice.target_faction = final_faction;

					messages::mode_notification notification;
					notification.subject_mode_id = id;
					notification.subject_name = player_data->get_nickname();
					notification.payload = choice;

					step.post_message(std::move(notification));
				}
				else {
					static_assert(always_false_v<C>, "Non-exhaustive std::visit");
				}
			};

			std::visit(command_callback, command_variant);
		}
	}
}

void arena_mode::spawn_and_kick_bots(const input_type in, const logic_step step) {
	const auto& names = in.rules.bot_names;
	const auto requested_bots = std::min(
		in.rules.bot_quota,
		static_cast<unsigned>(names.size())
	);

	if (current_num_bots == requested_bots) {
		return;
	}

	if (current_num_bots > requested_bots) {
		std::vector<mode_player_id> to_erase;

		for (const auto& p : reverse(players)) {
			if (p.second.is_bot) {
				to_erase.push_back(p.first);

				if (to_erase.size() == current_num_bots - requested_bots) {
					break;
				}
			}
		}

		for (const auto& t : to_erase) {
			remove_player(in, step, t);
		}

		current_num_bots = requested_bots;
	}

	while (current_num_bots < requested_bots) {
		const auto new_id = add_player(in, names[current_num_bots++]);
		auto_assign_faction(in, new_id);
		
		players.at(new_id).is_bot = true;
	}
}

void arena_mode::spawn_characters_for_recently_assigned(const input_type in, const logic_step step) {
	for (const auto& it : players) {
		const auto& player_data = it.second;
		const auto id = it.first;

		if (player_data.controlled_character_id.is_set()) {
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

			if (players.size() == 1 || passed <= in.rules.allow_spawn_for_secs_after_starting) {
				do_spawn();
			}
		}
	}
}

void arena_mode::handle_game_commencing(const input_type in, const logic_step step) {
	if (commencing_timer_ms != -1.f) {
		commencing_timer_ms -= step.get_delta().in_milliseconds();

		if (commencing_timer_ms <= 0.f) {
			commencing_timer_ms = -1.f;
			restart_match(in, step);
		}

		return;
	}

	const bool are_factions_ready = [&]() {
		const auto p = calc_participating_factions(in);

		bool all_have_at_least_one = true;

		p.for_each([&](const faction_type f) {
			if (num_players_in(f) == 0) {
				all_have_at_least_one = false;
			}
		});

		return all_have_at_least_one;
	}();

	if (!should_commence_when_ready && !are_factions_ready) {
		should_commence_when_ready = true;
		return;
	}

	if (should_commence_when_ready && are_factions_ready) {
		commencing_timer_ms = static_cast<real32>(in.rules.game_commencing_seconds * 1000);
		should_commence_when_ready = false;
		return;
	}
}

void arena_mode::end_warmup_and_go_live(const input_type in, const logic_step step) {
	if (state != arena_mode_state::WARMUP) {
		return;
	}

	state = arena_mode_state::LIVE;
	reset_players_stats(in);
	setup_round(in, step);

	post_team_match_start(in, step);
	check_duel_of_honor(in, step);
}

bool arena_mode::is_first_round_in_half(const const_input_type in) const { 
	return is_halfway_round(in) || get_current_round_number() == 0;
}

mode_player_id arena_mode::find_best_player_in(faction_type faction) const {
	auto best = mode_player_id::dead();
	std::optional<int> best_score;

	for_each_player_in(faction, [&](const auto& id, const auto& data) {
		const int score = data.stats.calc_score();

		if (best_score == std::nullopt || score > *best_score) {
			best = id;
			best_score = score;
		}
	});

	return best;
}

void arena_mode::report_match_result(const input_type in, const const_logic_step step) {
	const auto p = calc_participating_factions(in);

	const auto result = calc_match_result(in);

	const bool tied = result.is_tie();

	if (!tied) {
		if (!result.winner.has_value() || !result.loser.has_value()) {
			LOG("Wrong match result.");

			return;
		}
	}

	const auto first_team  = tied ? p.defusing  : *result.winner;
	const auto second_team = tied ? p.bombing   : *result.loser;

	auto make_entry = [](const auto& player) {
		messages::match_summary_message::player_entry new_entry;

		new_entry.kills = player.stats.knockouts;
		new_entry.assists = player.stats.assists;
		new_entry.deaths = player.stats.deaths;
		new_entry.nickname = player.get_nickname();
		new_entry.score = player.stats.calc_score();

		return new_entry;
	};

	auto strongest_in_first = mode_player_id::dead();
	auto strongest_in_second = mode_player_id::dead();

	messages::match_summary_message summary;

	summary.first_team_score = result.winner_score;
	summary.second_team_score = result.loser_score;

	for_each_player_best_to_worst_in(
		first_team,
		[&](const auto& id, const auto& player) {
			if (strongest_in_first == mode_player_id::dead()) {
				strongest_in_first = id;
			}

			summary.first_faction.emplace_back(make_entry(player));
		}
	);

	for_each_player_best_to_worst_in(
		second_team,
		[&](const auto& id, const auto& player) {
			if (strongest_in_second == mode_player_id::dead()) {
				strongest_in_second = id;
			}

			summary.second_faction.emplace_back(make_entry(player));
		}
	);

	auto strongest_1 = find(strongest_in_first);
	auto strongest_2 = find(strongest_in_second);

	if (strongest_1 == nullptr || strongest_2 == nullptr) {
		LOG("There are no opposing players. No match summary to be reported");

		return;
	}

	// Note it's LESS because stronger come first
	const auto stronger_of_the_two = (*strongest_1 < *strongest_2) ? strongest_in_first : strongest_in_second;
	summary.mvp_player_id = result.is_tie() ? stronger_of_the_two : strongest_in_first;

	if (stronger_of_the_two == strongest_in_second) {
		// Flip teams so that the first is where the mvp is
		summary.flip_teams();
	}

	step.post_message(summary);
}

void arena_mode::post_team_match_start(const input_type in, const logic_step step) {
	const auto p = calc_participating_factions(in);

	messages::team_match_start_message start;

	auto setup_team = [&](auto& into, const auto faction) {
		for_each_player_in(faction, [&](const auto&, const auto& data) {
			into.push_back({ data.get_nickname() });

			return callback_result::CONTINUE;
		});
	};

	setup_team(start.team_1, p.bombing);
	setup_team(start.team_2, p.defusing);

	step.post_message(start);
}

void arena_mode::check_duel_of_honor(const input_type in, const logic_step step) {
	const auto p = calc_participating_factions(in);

	LOG("Checking if there is a duel of honor.");

	if (p.size() == 2) {
		if (num_players_in(p.bombing) == 1) {
			if (num_players_in(p.defusing) == 1) {
				LOG("There is a duel indeed.");
				const auto first_id = find_best_player_in(p.bombing);
				const auto second_id = find_best_player_in(p.defusing);

				const auto first = find(first_id);
				const auto second = find(second_id);

				if (first != nullptr && second != nullptr) {
					hud_message_2_players(step, "", " and ", " have agreed to a [color=orange]duel of honor[/color].", first, second, true);

					duellist_1 = first_id;
					duellist_2 = second_id;

					messages::duel_of_honor_message duel;
					duel.first_player = first->get_nickname();
					duel.second_player = second->get_nickname();

					step.post_message(duel);
				}
			}
		}
	}
}

void arena_mode::mode_pre_solve(const input_type in, const mode_entropy& entropy, const logic_step step) {
	if (state == arena_mode_state::INIT) {
		restart_match(in, step);
	}

	if (state != arena_mode_state::WARMUP) {
		auto specific_presolve = [&]<typename S>(const S&) {
			if constexpr(std::is_same_v<S, gun_game_rules>) {
				auto& cosm = step.get_cosmos();

				const auto delay_for_mags = 2000.0f;
				remove_all_dropped_items(cosm, delay_for_mags, true);
			}
		};

		std::visit(specific_presolve, in.rules.subrules);
	}

	spawn_and_kick_bots(in, step);
	add_or_remove_players(in, entropy, step);
	handle_special_commands(in, entropy, step);
	spawn_characters_for_recently_assigned(in, step);

	if (in.rules.allow_game_commencing) {
		handle_game_commencing(in, step);
	}
	else {
		commencing_timer_ms = -1;
	}

	if (state == arena_mode_state::WARMUP) {
		respawn_the_dead(in, step, in.rules.warmup_respawn_after_ms);

		if (get_warmup_seconds_left(in) <= 0.f) {
			if (!current_round.cache_players_frozen) {
				set_players_frozen(in, true);
				release_triggers_of_weapons_of_players(in);
			}

			if (get_match_begins_in_seconds(in) <= 0.f) {
				end_warmup_and_go_live(in, step);
			}
		}
	}
	else if (state == arena_mode_state::LIVE) {
		if (in.rules.respawn_after_ms > 0) {
			/* 
				In case the character is dead due to changing factions,
				this will delete the character and respawn it with the proper faction.

				Note this already worked like this in the warmup deathmatch.
			*/

			respawn_the_dead(in, step, in.rules.respawn_after_ms);
		}

		if (get_freeze_seconds_left(in) <= 0.f) {
			if (current_round.cache_players_frozen) {
				play_start_round_sound(in, step);
			}

			set_players_frozen(in, false);
		}

		if (!is_game_commencing()) {
			process_win_conditions(in, step);
		}
	}
	else if (state == arena_mode_state::ROUND_END_DELAY) {
		if (get_round_end_seconds_left(in) <= 0.f) {
			{

			}

			start_next_round(in, step);
		}
	}
	else if (state == arena_mode_state::MATCH_SUMMARY) {
		if (get_match_summary_seconds_left(in) <= 0.f) {
			const auto p = calc_participating_factions(in);

			messages::match_summary_ended summary_ended;

			if (is_final_round(in)) {
				restart_match(in, step);

				summary_ended.is_final = true;
			}
			else {
				summary_ended.is_final = false;

				swap_assigned_factions(p);

				had_first_blood = false;

				std::swap(factions[p.bombing].score, factions[p.defusing].score);

				p.for_each([&](const auto f) {
					factions[f].clear_for_next_half();
				});

				set_players_money_to_initial(in);
				set_players_level_to_initial(in);

				start_next_round(in, step, round_start_type::DONT_KEEP_EQUIPMENTS);
			}

			step.post_message(summary_ended);
		}
	}
}

void arena_mode::mode_post_solve(const input_type in, const mode_entropy& entropy, const logic_step step) {
	auto access = allocate_new_entity_access();

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
					if (const auto sound_id = in.rules.view.event_sounds[faction][battle_event::IM_DEFUSING_THE_BOMB]; sound_id.is_set()) {
						auto stop = messages::stop_sound_effect(predictable_only_by(subject));
						stop.match_effect_id = sound_id;
						step.post_message(stop);
					}
				}
				else {
					if (const auto sound_id = in.rules.view.event_sounds[faction][event]; sound_id.is_set()) {
						play_faction_sound(step, faction, sound_id, predictable_only_by(subject));
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
					count_knockout(step, in, victim, victim.get<components::sentience>());
				};

				/*
					For now, loss of consciousness does not imply a knockout.

					if (e.special_result == messages::health_event::result_type::LOSS_OF_CONSCIOUSNESS) {
						make_it_count();
					}
				*/
				if (e.special_result == messages::health_event::result_type::DEATH) {
					/* Don't count two kills on a single character. */
					if (e.was_conscious) {
						make_it_count();
					}
				}
			}
		}
	}

	const bool is_bomb_planted = bomb_planted(in);

	if (is_bomb_planted) {
		on_bomb_entity(in, [&](const auto& typed_bomb) {
			if constexpr(!is_nullopt_v<decltype(typed_bomb)>) {
				const auto& clk = cosm.get_clock();

				if (typed_bomb.template get<components::hand_fuse>().when_armed == clk.now) {
					if (state == arena_mode_state::LIVE) {
						play_sound_for(in, step, battle_event::BOMB_PLANTED, never_predictable_v);
					}

					auto& planter = current_round.bomb_planter;
					planter = lookup(cosm[typed_bomb.template get<components::sender>().capability_of_sender]);

					if (const auto s = stats_of(planter)) {
						s->bomb_plants += 1;
					}

					give_monetary_award(in, planter, in.rules.economy.bomb_plant_award);
				}
			}
		});
	}

	if (state == arena_mode_state::LIVE) {
		if (is_bomb_planted) {
			if (get_critical_seconds_left(in) <= in.rules.view.secs_until_detonation_to_start_theme) {
				if (!bomb_detonation_theme.is_set()) {
					const auto theme = in.rules.view.bomb_soon_explodes_theme;

					if (theme.is_set()) {
						LOG("Bomb detonation theme started.");

						bomb_detonation_theme = just_create_entity(access, cosm, theme);
					}
				}
			}
		}
	}
}

void arena_mode::respawn_the_dead(const input_type in, const logic_step step, const unsigned after_ms) {
	auto& cosm = in.cosm;
	const auto& clk = cosm.get_clock();

	for (auto& it : players) {
		auto& player_data = it.second;
		const auto id = it.first;

		on_player_handle(cosm, id, [&](const auto& player_handle) {
			if constexpr(!is_nullopt_v<decltype(player_handle)>) {
				const auto& sentience = player_handle.template get<components::sentience>();

				if (sentience.when_knocked_out.was_set() && clk.is_ready(
					after_ms,
					sentience.when_knocked_out
				)) {
					round_transferred_player transfer;
					transfer.movement = player_handle.template get<components::movement>().flags;

					::delete_with_held_items_except(in.rules.bomb_flavour, step, player_handle);
					player_data.controlled_character_id.unset();

					messages::changed_identities_message dummy_msg;
					const auto meta = transfer_meta { transfer, dummy_msg };
					create_character_for_player(in, step, id, meta);
				}
			}
		});
	}
}

const float match_begins_in_secs_v = 4.f;

float arena_mode::get_warmup_seconds_left(const const_input_type in) const {
	if (state == arena_mode_state::WARMUP) {
		return static_cast<float>(in.rules.warmup_secs) - get_seconds_passed_in_cosmos(in);
	}

	return -1.f;
}

float arena_mode::get_match_begins_in_seconds(const const_input_type in) const {
	if (state == arena_mode_state::WARMUP) {
		const auto secs = get_seconds_passed_in_cosmos(in);
		const auto warmup_secs = static_cast<float>(in.rules.warmup_secs);

		if (secs >= warmup_secs) {
			const auto match_begins_in_secs = match_begins_in_secs_v;
			return warmup_secs + match_begins_in_secs - secs;
		}
	}

	return -1.f;
}

float arena_mode::get_seconds_passed_in_cosmos(const const_input_type in) const {
	return in.cosm.get_clock().now.in_seconds(round_speeds.calc_ticking_delta());
}

float arena_mode::get_round_seconds_passed(const const_input_type in) const {
	return get_seconds_passed_in_cosmos(in) - static_cast<float>(in.rules.freeze_secs);
}

float arena_mode::get_freeze_seconds_left(const const_input_type in) const {
	return static_cast<float>(in.rules.freeze_secs) - get_seconds_passed_in_cosmos(in);
}

float arena_mode::get_buy_seconds_left(const const_input_type in) const {
	if (!in.rules.has_economy()) {
		return 0.f;
	}

	if (state == arena_mode_state::WARMUP) {
		if (!in.rules.warmup_enable_item_shop) {
			return 0.f;
		}

		return get_warmup_seconds_left(in);
	}

	if (!in.rules.enable_item_shop) {
		return 0.f;
	}

	return static_cast<float>(in.rules.freeze_secs + in.rules.buy_secs_after_freeze) - get_seconds_passed_in_cosmos(in);
}

float arena_mode::get_round_seconds_left(const const_input_type in) const {
	return static_cast<float>(in.rules.round_secs) + in.rules.freeze_secs - get_seconds_passed_in_cosmos(in);
}

float arena_mode::get_seconds_since_win(const const_input_type in) const {
	const auto& last_win = current_round.last_win;

	if (!last_win.was_set()) {
		return -1.f;
	}

	auto clk = in.cosm.get_clock();
	clk.dt = round_speeds.calc_ticking_delta();
	return clk.diff_seconds(last_win.when);
}

float arena_mode::get_match_summary_seconds_left(const const_input_type in) const {
	if (const auto since_win = get_seconds_since_win(in); since_win != -1.f) {
		return static_cast<float>(in.rules.match_summary_seconds) - since_win;
	}

	return -1.f;
}

float arena_mode::get_round_end_seconds_left(const const_input_type in) const {
	if (!current_round.last_win.was_set()) {
		return -1.f;
	}

	return static_cast<float>(in.rules.round_end_secs) - get_seconds_since_win(in);
}

bool arena_mode::bomb_exploded(const const_input_type in) const {
	if (!in.rules.has_bomb_mechanics()) {
		return false;
	}

	return on_bomb_entity(in, [&](const auto& t) {
		/* 
			The bomb could have stopped existing through only one way: 
			it has exploded.
		*/

		return is_nullopt_v<decltype(t)>;
	});
}

entity_id arena_mode::get_character_who_defused_bomb(const const_input_type in) const {
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

bool arena_mode::bomb_planted(const const_input_type in) const {
	return on_bomb_entity(in, [&](const auto& typed_bomb) {
		if constexpr(is_nullopt_v<decltype(typed_bomb)>) {
			return false;
		}
		else {
			return typed_bomb.template get<components::hand_fuse>().armed();
		}
	});
}

real32 arena_mode::get_critical_seconds_left(const const_input_type in) const {
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

			return clk.get_remaining_secs(fuse.fuse_delay_ms, when_armed);
		}
	});
}

float arena_mode::get_seconds_since_planting(const const_input_type in) const {
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

unsigned arena_mode::get_current_round_number() const {
	unsigned total = 0;

	for_each_faction([&](const auto f) {
		total += get_score(f);
	});

	return total;
}

unsigned arena_mode::get_score(const faction_type f) const {
	return factions[f].score;
}

arena_mode_match_result arena_mode::calc_match_result(const const_input_type in) const {
	const auto p = calc_participating_factions(in);

	if (get_score(p.bombing) == get_score(p.defusing)) {
		auto tie = arena_mode_match_result::make_tie();
		tie.winner_score = tie.loser_score = get_score(p.bombing);

		return tie;
	}

	arena_mode_match_result result;

	if (get_score(p.bombing) > get_score(p.defusing)) {
		result.winner = p.bombing;
		result.loser  = p.defusing;
	}
	else {
		result.winner = p.defusing;
		result.loser  = p.bombing;
	}

	result.winner_score = get_score(*result.winner);
	result.loser_score =  get_score(*result.loser);

	return result;
}

template <class S, class E>
auto arena_mode::find_player_by_impl(S& self, const E& identifier) {
	using R = maybe_const_ptr_t<std::is_const_v<S>, std::pair<const mode_player_id, arena_mode_player>>;

	for (auto& it : self.players) {
		auto& player_data = it.second;

		if constexpr(std::is_same_v<entity_name_str, E>) {
			if (player_data.session.nickname == identifier) {
				return std::addressof(it);
			}
		}
		else if constexpr(std::is_same_v<session_id_type, E>) {
			if (player_data.session.id == identifier) {
				return std::addressof(it);
			}
		}
	}

	return R(nullptr);
}

arena_mode_player* arena_mode::find(const mode_player_id& id) {
	return mapped_or_nullptr(players, id);
}

const arena_mode_player* arena_mode::find(const mode_player_id& id) const {
	return mapped_or_nullptr(players, id);
}

mode_player_id arena_mode::lookup(const session_id_type& session_id) const {
	if (const auto r = find_player_by_impl(*this, session_id)) {
		return r->first;
	}

	return {};
}

const arena_mode_player* arena_mode::find(const session_id_type& session_id) const {
	if (const auto r = find_player_by_impl(*this, session_id)) {
		return std::addressof(r->second);
	}

	return nullptr;
}

arena_mode_player* arena_mode::find_player_by(const entity_name_str& nickname) {
	if (const auto r = find_player_by_impl(*this, nickname)) {
		return std::addressof(r->second);
	}

	return nullptr;
}

const arena_mode_player* arena_mode::find_player_by(const entity_name_str& nickname) const {
	if (const auto r = find_player_by_impl(*this, nickname)) {
		return std::addressof(r->second);
	}

	return nullptr;
}

void arena_mode::restart_match(const input_type in, const logic_step step) {
	reset_players_stats(in);
	factions = {};
	had_first_blood = false;
	prepare_to_fight_counter = 0;
	clear_duel();

	if (in.rules.warmup_secs > 4) {
		state = arena_mode_state::WARMUP;

		for (auto& p : players) {
			p.second.stats.money = in.rules.economy.warmup_initial_money;
		}
	}
	else {
		state = arena_mode_state::LIVE;
	}

	setup_round(in, step);
}

unsigned arena_mode::calc_max_faction_score() const {
	unsigned maximal = 0;

	for_each_faction([&](const auto f) {
		maximal = std::max(maximal, factions[f].score);
	});

	return maximal;
}


void arena_mode::clear_players_round_state(const input_type in) {
	(void)in;

	for (auto& it : players) {
		it.second.stats.round_state = {};
	}

	set_players_level_to_initial(in);
}

void arena_mode::set_players_money_to_initial(const input_type in) {
	for (auto& it : players) {
		auto& p = it.second;
		p.stats.money = in.rules.economy.initial_money;
	}
}

void arena_mode::set_players_level_to_initial(const input_type in) {
	(void)in;

	for (auto& it : players) {
		auto& p = it.second;
		p.stats.level = 0;
	}
}

void arena_mode::reset_players_stats(const input_type in) {
	for (auto& it : players) {
		auto& p = it.second;
		p.stats = {};
		p.round_when_chosen_faction = static_cast<uint32_t>(-1);
	}

	clear_players_round_state(in);
	set_players_money_to_initial(in);
}

void arena_mode::give_monetary_award(const input_type in, const mode_player_id id, money_type amount) {
	if (state == arena_mode_state::WARMUP) {
		return;
	}

	if (const auto stats = stats_of(id)) {
		auto& current_money = stats->money;
		amount = std::clamp(amount, -current_money, in.rules.economy.maximum_money - current_money);

		if (amount != 0) {
			current_money += amount;

			const auto award = arena_mode_award {
				in.cosm.get_clock(), id, amount 
			};

			stats->round_state.awards.emplace_back(award);
		}
	}
}

bool arena_mode::suitable_for_spectating(
	const const_input in, 
	const mode_player_id& who, 
	const mode_player_id& by, 
	const real32 limit_in_seconds
) const {
	if (const auto by_data = find(by)) {
		const auto spectator_faction = by_data->get_faction();
		const bool can_watch_anybody = spectator_faction == faction_type::SPECTATOR && in.rules.allow_spectator_to_see_both_teams;

		const auto num_conscious_teammates = num_conscious_players_in(in.cosm, spectator_faction);
		const bool all_teammates_unconscious = in.rules.allow_spectate_enemy_if_no_conscious_players && 0 == num_conscious_teammates;

		if (const auto who_data = find(who)) {
			if (can_watch_anybody || all_teammates_unconscious || who_data->get_faction() == spectator_faction) {
				return conscious_or_can_still_spectate(in, who, limit_in_seconds);
			}
		}
	}

	return false;
}

bool arena_mode::conscious_or_can_still_spectate(
	const const_input in, 
	const mode_player_id& who, 
	const real32 limit_in_seconds
) const {
	const auto max_secs = std::min(limit_in_seconds, static_cast<real32>(in.rules.view.can_spectate_dead_body_for_secs));
	const auto& clk = in.cosm.get_clock();

	return on_player_handle(in.cosm, who, [&](const auto& player_handle) {
		if constexpr(!is_nullopt_v<decltype(player_handle)>) {
			const auto& sentience = player_handle.template get<components::sentience>();

			if (sentience.is_conscious()) {
				return true;
			}

			return clk.lasts(
				max_secs * 1000,
				sentience.when_knocked_out
			); 
		}

		return false;
	});
}

mode_player_id arena_mode::get_next_to_spectate(
	const const_input_type in, 
	const arena_player_order_info& after, 
	const faction_type& spectator_faction, 
	const int offset,
	const real32 limit_in_seconds
) const {
	std::vector<arena_player_order_info> sorted_players;

	auto can_still_spectate = [&](const auto& who) {
		return conscious_or_can_still_spectate(in, who, limit_in_seconds);
	};

	auto gather_candidates_from_all_players = [&]() {
		for (const auto& p : players) {
			if (p.second.get_faction() != faction_type::SPECTATOR) {
				if (can_still_spectate(p.first)) {
					sorted_players.emplace_back(p.second.get_order());
				}
			}
		}
	};

	if (spectator_faction == faction_type::SPECTATOR) {
		if (!in.rules.allow_spectator_to_see_both_teams) {
			return {};
		}

		gather_candidates_from_all_players();
	}
	else {
		const auto num_conscious_teammates = num_conscious_players_in(in.cosm, spectator_faction);

		if (in.rules.allow_spectate_enemy_if_no_conscious_players && num_conscious_teammates == 0) {
			gather_candidates_from_all_players();
		}
		else {
			for (const auto& p : players) {
				if (p.second.get_faction() == spectator_faction) {
					if (can_still_spectate(p.first)) {
						sorted_players.emplace_back(p.second.get_order());
					}
				}
			}
		}
	}

	if (sorted_players.empty()) {
		return mode_player_id();
	}

	sort_range(sorted_players);

	if (offset < 0) {
		reverse_range(sorted_players);
	}

	auto cmp = [&](const auto& a, const auto& b){
		if (offset > 0) { 
			return a < b;
		}
	
		return b < a;
	};

	auto it = std::upper_bound(sorted_players.begin(), sorted_players.end(), after, cmp);

	if (it == sorted_players.end()) {
		it = sorted_players.begin();
	}

	// TODO: Optimize
	if (const auto player = find_player_by(it->nickname)) {
		return lookup(player->controlled_character_id);
	}

	return {};
}

augs::maybe<rgba> arena_mode::get_current_fallback_color_for(const const_input_type in, const faction_type faction) const {
	const auto& r = in.rules;

	if (!r.enable_player_colors) {
		return r.default_player_color;
	}

	if (faction == faction_type::SPECTATOR) {
		return {};
	}

	int faction_count = 0;

	for (const auto& it : players) {
		const auto& player_data = it.second;

		if (player_data.get_faction() == faction) {
			++faction_count;
		}
	}

	if (faction_count > static_cast<int>(r.player_colors.size())) {
		return r.excess_player_color;
	}

	return {};
}

uint32_t arena_mode::get_num_players() const {
	return players.size();
}

uint32_t arena_mode::get_num_active_players() const {
	return get_num_players() - num_players_in(faction_type::SPECTATOR);
}

uint32_t arena_mode::get_max_num_active_players(const const_input_type in) const {
	return in.rules.max_players_per_team * 2;
}

void arena_mode::handle_duel_desertion(const input_type in, const logic_step step, const mode_player_id& deserter_id) { 
	if (is_match_summary() && is_final_round(in)) {
		/* Not a desertion when the outcome is known already */
		return;
	}

	if (is_a_duellist(deserter_id)) {
		auto deserter_state = find(deserter_id);
		auto opponent_state = find(get_opponent_duellist(deserter_id));

		if (deserter_state != nullptr && opponent_state != nullptr) {
			messages::duel_interrupted_message duel_interrupted;

			/* 
				Desertion might have been due to a changed faction,
				so take the opponent's faction as reference.
			*/

			const auto opponent_faction = opponent_state->get_faction();
			const auto deserter_faction = get_opposing_faction(in, opponent_faction);

			duel_interrupted.deserter_score = get_score(deserter_faction);
			duel_interrupted.opponent_score = get_score(opponent_faction);
			duel_interrupted.deserter_nickname = deserter_state->get_nickname();
			duel_interrupted.opponent_nickname = opponent_state->get_nickname();

			step.post_message(duel_interrupted);
		}

		clear_duel();
	}
}

mode_player_id arena_mode::get_opponent_duellist(const mode_player_id& id) const {
	 return id == duellist_1 ? duellist_2 : duellist_1;
}

bool arena_mode::is_a_duellist(const mode_player_id& id) const {
	return id == duellist_1 || id == duellist_2;
}

void arena_mode::clear_duel() {
	duellist_1 = mode_player_id::dead();
	duellist_2 = mode_player_id::dead();
}

game_mode_name_type arena_mode::get_name(const_input_type in) const {
	return std::visit([](const auto& s) { return s.get_name(); }, in.rules.subrules);
}

bool arena_mode_ruleset::has_economy() const {
	return std::visit([](const auto& s) { return s.has_economy(); }, subrules);
}

bool arena_mode::levelling_enabled(const_input_type in) const {
	return state != arena_mode_state::WARMUP && std::holds_alternative<gun_game_rules>(in.rules.subrules);
}
