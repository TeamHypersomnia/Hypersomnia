#pragma once
#include "game/organization/all_entity_types.h"
#include "game/components/attitude_component.h"
#include "game/common_state/entity_flavours.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/for_each_entity.h"
#include "game/modes/mode_common.h"

template <class F>
void for_each_faction(F callback) {
	callback(faction_type::METROPOLIS);
	callback(faction_type::ATLANTIS);
	callback(faction_type::RESISTANCE);
}

inline auto calc_spawnable_factions(const cosmos& cosm) {
	per_faction_t<bool> result = {};

	for_each_faction([&](const auto faction) {
		for_each_faction_spawn(cosm, faction, [&](const auto&) {
			result[faction] = true;
			return callback_result::ABORT;
		});
	});

	return result;
}

using player_character_type = controlled_character;

inline auto remove_test_characters(cosmos& cosm) {
	cosm.for_each_having<invariants::sentience>(
		[&](const auto& typed_handle){
			if constexpr(std::is_same_v<entity_type_of<decltype(typed_handle)>, player_character_type>) {
				cosmic::delete_entity(typed_handle);
			}
		}
	);
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

			if (marker.type == point_marker_type::TEAM_SPAWN) {
				if (marker.meta.associated_faction == faction) {
					return callback(typed_handle);
				}
			}

			return callback_result::CONTINUE;
		}
	);
}

inline auto get_num_faction_spawns(const cosmos& cosm, const faction_type faction) {
	std::size_t total = 0;
	for_each_faction_spawn(cosm, faction, [&](auto) { ++total; return callback_result::CONTINUE; } );
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
