#pragma once
#include "game/organization/all_entity_types.h"
#include "game/components/attitude_component.h"
#include "game/common_state/entity_flavours.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/for_each_entity.h"
#include "augs/templates/continue_or_callback_result.h"
#include "game/detail/explosive/like_explosive.h"
#include "game/detail/melee/like_melee.h"

template <class F>
void for_each_faction(F callback) {
	callback(faction_type::METROPOLIS);
	callback(faction_type::ATLANTIS);
	callback(faction_type::RESISTANCE);
}

inline auto calc_spawnable_factions(const cosmos& cosm) {
	per_actual_faction<bool> result = {};

	for_each_faction([&](const auto faction) {
		for_each_faction_spawn(cosm, faction, [&](const auto&) {
			result[faction] = true;
			return callback_result::ABORT;
		});
	});

	return result;
}

using player_character_type = controlled_character;

void reverse_perform_deletions(const deletion_queue& deletions, cosmos& cosm);

inline void remove_test_characters(cosmos& cosm) {
	deletion_queue q;

	cosm.for_each_having<invariants::sentience>(
		[&](const auto typed_handle) {
			if constexpr(std::is_same_v<entity_type_of<decltype(typed_handle)>, player_character_type>) {
				q.push_back(entity_id(typed_handle.get_id()));

				typed_handle.for_each_contained_item_recursive(
					[&](const auto& it) {
						q.push_back(entity_id(it.get_id()));
					}
				);
			}
		}
	);

	reverse_perform_deletions(q, cosm);
}

inline void remove_all_dropped_items(cosmos& cosm, const float delay_for_mags = 0.0f, bool allow_melees = false) {
	deletion_queue q;

	auto& clk = cosm.get_clock();

	cosm.for_each_having<components::item>(
		[&](const auto typed_handle) {
			if (is_like_thrown_explosive(typed_handle)) {
				return;
			}

			if (allow_melees) {
				if (typed_handle.template has<components::melee>()) {
					return;
				}
			}

			if (is_like_thrown_melee(typed_handle)) {
				return;
			}

			if (typed_handle.get_current_slot().dead()) {
				if (delay_for_mags > 0.0f) {
					if (typed_handle.template get<invariants::item>().categories_for_slot_compatibility.test(item_category::MAGAZINE)) {
						const bool dont_delete_yet = clk.lasts(
							delay_for_mags,
							typed_handle.when_last_transferred()
						);

						if (dont_delete_yet) {
							return;
						}
					}
				}

				q.push_back(entity_id(typed_handle.get_id()));

				typed_handle.for_each_contained_item_recursive(
					[&](const auto& it) {
						q.push_back(entity_id(it.get_id()));
					}
				);
			}
		}
	);

	reverse_perform_deletions(q, cosm);
}

inline auto find_faction_character_flavour(const cosmos& cosm, const faction_type faction) {
	using E = player_character_type;
	using flavour_id_type = typed_entity_flavour_id<E>;
	using flavour_type = entity_flavour<E>;

	flavour_id_type result;

	cosm.for_each_id_and_flavour<E>(
		[&](const flavour_id_type& id, const flavour_type& flavour) {
			const auto& attitude = flavour.get<components::attitude>();

			if (attitude.official_faction == faction) {
				result = id;
			}
		}
	);

	return result;
}

template <class F>
auto for_each_faction_spawn(const cosmos& cosm, const faction_type faction, F&& callback) {
	cosm.for_each_having<invariants::point_marker>(
		[&](const auto typed_handle) -> callback_result {
			const auto& marker = typed_handle.template get<invariants::point_marker>();

			if (faction == faction_type::FFA) {
				if (marker.type == point_marker_type::FFA_SPAWN) {
					return continue_or_callback_result(std::forward<F>(callback), typed_handle);
				}

				return callback_result::CONTINUE;
			}

			if (marker.type == point_marker_type::TEAM_SPAWN) {
				if (typed_handle.get_official_faction() == faction) {
					return continue_or_callback_result(std::forward<F>(callback), typed_handle);
				}
			}

			return callback_result::CONTINUE;
		}
	);
}

inline auto get_num_faction_spawns(const cosmos& cosm, const faction_type faction) {
	std::size_t total = 0;
	for_each_faction_spawn(cosm, faction, [&](auto) { ++total; } );
	return total;
}

inline auto find_faction_spawn(const cosmos& cosm, const faction_type faction, unsigned spawn_index) {
	entity_id result;

	for_each_faction_spawn(cosm, faction, [&](const auto typed_handle) {
		if (spawn_index > 0) {
			--spawn_index;
			return callback_result::CONTINUE;
		}

		result = typed_handle.get_id();
		return callback_result::ABORT;
	});

	return cosm[result];
}
