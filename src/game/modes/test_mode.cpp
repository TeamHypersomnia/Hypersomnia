#include "game/cosmos/solvers/standard_solver.h"
#include "game/modes/test_mode.h"
#include "game/modes/mode_entropy.h"
#include "game/modes/mode_helpers.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/detail/inventory/generate_equipment.h"
#include "game/detail/snap_interpolation_to_logical.h"
#include "game/messages/mode_notification.h"
#include "augs/templates/logically_empty.h"
#include "game/modes/detail/delete_with_held_items.hpp"

#include "game/detail/sentience/sentience_logic.h"
#include "game/cosmos/create_entity.hpp"

using input_type = test_mode::input;

mode_entity_id test_mode::lookup(const mode_player_id& id) const {
	if (const auto entry = mapped_or_nullptr(players, id)) {
		return entry->controlled_character_id;
	}

	return mode_entity_id::dead();
}

mode_player_id test_mode::lookup(const entity_id& controlled_character_id) const {
	for (const auto& p : players) {
		if (p.second.controlled_character_id == controlled_character_id) {
			return p.first;
		}
	}

	return mode_player_id::dead();
}

void test_mode::init_spawned(const input_type in, const entity_id id, const logic_step step) {
	const auto handle = in.cosm[id];

	auto access = allocate_new_entity_access();

	handle.dispatch_on_having_all<components::sentience>([&](const auto typed_handle) {
		in.rules.factions[typed_handle.get_official_faction()].round_start_eq.generate_for(access, typed_handle, step, 1);

		if (auto crosshair = typed_handle.template find<components::crosshair>()) {
			crosshair->recoil = {};
		}

		::resurrect(step, typed_handle);
	});
}

void test_mode::teleport_to_next_spawn(const input_type in, const mode_player_id mode_id, const entity_id id) {
	const auto handle = in.cosm[id];

	handle.dispatch_on_having_all<components::sentience>([&](const auto typed_handle) {
		auto tp_to = [&](const auto spawn) {
			const auto spawn_transform = spawn.get_logic_transform();
			typed_handle.set_logic_transform(spawn_transform);
			::snap_interpolated_to(typed_handle, spawn_transform);

			if (const auto crosshair = typed_handle.find_crosshair()) {
				crosshair->base_offset = spawn_transform.get_direction() * 200;
			}
		};

		if (const auto p = find(mode_id)) {
			if (const auto dedicated = in.cosm[p->dedicated_spawn]) {
				tp_to(dedicated);
				return;
			}
		}

		const auto faction = typed_handle.get_official_faction();
		const auto num_spawns = get_num_faction_spawns(in.cosm, faction);

		/* Respawn in the same place during playtest */
		if (0 == num_spawns || playtesting_context.has_value()) {
			return;
		}

		auto normalize_spawn_index = [&]() {
			current_spawn_index = (current_spawn_index + 1) % num_spawns;
		};

		normalize_spawn_index();

		if (const auto spawn = ::find_faction_spawn(in.cosm, faction, current_spawn_index)) {
			tp_to(spawn);
			normalize_spawn_index();
		}
	});
}

mode_player_id test_mode::add_player(input_type in, const entity_name_str& name, const faction_type faction) {
	if (const auto new_id = first_free_key(players, mode_player_id::first()); new_id.is_set()) {
		add_player_custom(in, { new_id, name, faction });
		return new_id;
	}

	return mode_player_id::dead();
}

void test_mode::remove_player(input_type in, const logic_step step, const mode_player_id id) {
	const auto controlled_character_id = lookup(id);

	::delete_with_held_items_except({}, step, in.cosm[controlled_character_id]);

	erase_element(players, id);
}

void test_mode::create_controlled_character_for(const input_type in, const mode_player_id id) {
	auto access = allocate_new_entity_access();

	auto entry = find(id);

	if (entry == nullptr) {
		return;
	}

	auto& cosm = in.cosm;
	const auto faction = entry->get_faction();

	if (const auto flavour = ::find_faction_character_flavour(cosm, faction); flavour.is_set()) {
		cosmic::specific_create_entity(
			access,
			cosm, 
			flavour, 
			[&](const auto new_character, auto&&...) {
				if (playtesting_context) {
					const auto spawn_transform = transformr(playtesting_context->initial_spawn_pos, 0);

					new_character.set_logic_transform(spawn_transform);
					::snap_interpolated_to(new_character, spawn_transform);

					if (const auto crosshair = new_character.find_crosshair()) {
						crosshair->base_offset = vec2::zero;
					}
				}
				else {
					teleport_to_next_spawn(in, id, new_character);
				}

				pending_inits.push_back(new_character);

				cosmic::set_specific_name(new_character, entry->get_nickname());
				entry->controlled_character_id = new_character.get_id();
			}
		);
	}
}

bool test_mode::add_player_custom(const input_type in, const add_player_input& add_in) {
	auto& cosm = in.cosm;
	(void)cosm;

	auto considered_faction = add_in.faction;

	if (playtesting_context) {
		const bool is_playtest_host = players.empty();

		if (is_playtest_host) {
			considered_faction = playtesting_context->first_player_faction;
		}
		else {
			considered_faction = playtesting_context->first_player_faction == faction_type::RESISTANCE ? faction_type::METROPOLIS : faction_type::RESISTANCE;
		}
	}
	else {
		// Default to metropolis

		if (considered_faction != faction_type::METROPOLIS && considered_faction != faction_type::RESISTANCE) {
			considered_faction = faction_type::METROPOLIS;
		}
	}

	const auto& new_id = add_in.id;

	auto& new_player = (*players.try_emplace(new_id).first).second;

	if (new_player.is_set()) {
		return false;
	}

	new_player.session.nickname = add_in.name;
	new_player.session.id = next_session_id;
	new_player.session.faction = considered_faction;
	++next_session_id;

	create_controlled_character_for(in, new_id);

	return true;
}

void test_mode::add_or_remove_players(const input_type in, const mode_entropy& entropy, const logic_step step) {
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

void test_mode::remove_old_lying_items(input_type in, logic_step) {
	const auto max_age_ms = 10000;

	auto& cosm = in.cosm;
	const auto& clk = cosm.get_clock();

	deletion_queue q;

	cosm.for_each_having<invariants::item>([&](const auto typed_handle) {
		if (typed_handle.get_current_slot().alive()) {
			return;
		}

		const auto when_born = typed_handle.when_born();
		const auto when_dropped = typed_handle.when_last_transferred();

		const bool was_created_somewhat_later = when_born.step > 10;

		if (was_created_somewhat_later && when_dropped.was_set()) {
			if (clk.is_ready(max_age_ms, when_dropped)) {
				q.push_back(entity_id(typed_handle.get_id()));

				typed_handle.for_each_contained_item_recursive(
					[&](const auto& contained) {
						q.push_back(entity_id(contained.get_id()));
					}
				);
			}
		}
	});

	::reverse_perform_deletions(q, cosm);
}

void test_mode::mode_pre_solve(input_type in, const mode_entropy& entropy, logic_step step) {
	add_or_remove_players(in, entropy, step);

	auto& cosm = in.cosm;

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
			in.rules.respawn_after_ms,
			sentience.when_knocked_out
		)) {
			const auto player_id = lookup(typed_handle.get_id());

			if (!find(player_id)->allow_respawn) {
				return;
			}

			teleport_to_next_spawn(in, player_id, typed_handle);
			init_spawned(in, typed_handle, step);

			sentience.when_knocked_out = {};
		}
	});

	remove_old_lying_items(in, step);

	if (auto character = in.cosm[infinite_ammo_for]) {
		auto guns = character.get_wielded_guns();

		for (auto g : guns) {
			const auto weapon = in.cosm[g];

			const auto ammo_piece_flavour = ::calc_purchasable_ammo_piece(weapon);

			auto total_ammo_for_this_weapon = 0;

			character.for_each_contained_item_recursive(
				[&](const auto& ammo_piece) {
					if (entity_flavour_id(ammo_piece.get_flavour_id()) == weapon.get_flavour_id()) {
						return recursive_callback_result::CONTINUE_DONT_RECURSE;
					}

					if (entity_flavour_id(ammo_piece.get_flavour_id()) == ammo_piece_flavour) {
						auto count_charge_stack = [&](const auto& ammo_stack) {
							if (ammo_stack.template has<invariants::cartridge>()) {
								total_ammo_for_this_weapon += ammo_stack.template get<components::item>().get_charges();
							}
						};

						count_charge_stack(ammo_piece);

						ammo_piece.for_each_contained_item_recursive(count_charge_stack);

						return recursive_callback_result::CONTINUE_DONT_RECURSE;
					}

					return recursive_callback_result::CONTINUE_AND_RECURSE;
				},
				std::nullopt
			);

			if (total_ammo_for_this_weapon == 0) {
				auto access = allocate_new_entity_access();
				requested_equipment eq;
				eq.non_standard_mag = ammo_piece_flavour;
				eq.num_given_ammo_pieces = 1;
				eq.perform_recoils = false;
				eq.generate_for(access, character, step, 0);
			}
		}
	}
}

arena_migrated_session test_mode::emigrate() const {
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

void test_mode::migrate(const input_type in, const arena_migrated_session& session) {
	ensure(players.empty());

	for (const auto& migrated_player : session.players) {
		const auto mode_id = migrated_player.mode_id;
		const auto& it = players.try_emplace(mode_id);
		auto& new_player = (*it.first).second;

		ensure(it.second);
		ensure(!new_player.session.is_set());

		new_player.session = migrated_player.data;
		new_player.session.faction = new_player.session.faction;

		create_controlled_character_for(in, mode_id);
	}

	next_session_id = session.next_session_id;
}

template <class S, class E>
auto test_mode::find_player_by_impl(S& self, const E& identifier) {
	using R = maybe_const_ptr_t<std::is_const_v<S>, std::pair<const mode_player_id, test_mode_player>>;

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
		else if constexpr(std::is_same_v<entity_id, E>) {
			if (player_data.controlled_character_id == identifier) {
				return std::addressof(it);
			}
		}
	}

	return R(nullptr);
}

test_mode_player* test_mode::find(const mode_player_id& id) {
	return mapped_or_nullptr(players, id);
}

const test_mode_player* test_mode::find(const mode_player_id& id) const {
	return mapped_or_nullptr(players, id);
}

mode_player_id test_mode::lookup(const session_id_type& session_id) const {
	if (const auto r = find_player_by_impl(*this, session_id)) {
		return r->first;
	}

	return {};
}

const test_mode_player* test_mode::find(const session_id_type& session_id) const {
	if (const auto r = find_player_by_impl(*this, session_id)) {
		return std::addressof(r->second);
	}

	return nullptr;
}

test_mode_player* test_mode::find_player_by(const entity_name_str& nickname) {
	if (const auto r = find_player_by_impl(*this, nickname)) {
		return std::addressof(r->second);
	}

	return nullptr;
}

const test_mode_player* test_mode::find_player_by(const entity_name_str& nickname) const {
	if (const auto r = find_player_by_impl(*this, nickname)) {
		return std::addressof(r->second);
	}

	return nullptr;
}

std::size_t test_mode::num_players_in(const faction_type faction) const {
	auto total = std::size_t(0);

	for_each_player_in(faction, [&](auto&&...) {
		++total;
	});

	return total;
}

uint32_t test_mode::get_num_players() const {
	return players.size();
}

uint32_t test_mode::get_num_active_players() const {
	return get_num_players() - num_players_in(faction_type::SPECTATOR);
}

uint32_t test_mode::get_max_num_active_players(const const_input) const {
	const auto max_players_per_team = 32;
	return max_players_per_team * 2;
}

bool test_mode_player::operator<(const test_mode_player& b) const {
	const auto ao = arena_player_order { get_nickname(), stats.calc_score() };
	const auto bo = arena_player_order { b.get_nickname(), b.stats.calc_score() };

	return ao < bo;
}
