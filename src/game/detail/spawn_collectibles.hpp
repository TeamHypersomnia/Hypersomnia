#pragma once
#include "game/cosmos/just_create_entity.h"
#include "game/cosmos/entity_flavour_id.h"
#include "game/components/rigid_body_component.h"
#include "augs/misc/randomization.h"

/*
 * Queue-based version of spawn_coins that uses queue_just_create_entity
 * to safely spawn coins without causing entity pool reallocation issues.
 */
template <class FlavoursContainer>
inline void spawn_coins_queued(
	const money_type total_value,
	const vec2 position,
	const logic_step step,
	const FlavoursContainer& flavours
) {
	auto& cosm = step.get_cosmos();

	auto val_of = [&](auto f) {
		if (!f.is_set()) {
			return 0;
		}

		return cosm.on_flavour(f, [&](auto& typed_flavour) {
			return typed_flavour.template get<invariants::touch_collectible>().money_value;
		});
	};

	if (flavours.empty()) {
		return;
	}

	std::size_t denom_i = 0;
	money_type denom_val = val_of(flavours[denom_i]);

	for (money_type spawned = 0; spawned < total_value;) {
		while (!flavours[denom_i].is_set() || spawned + denom_val > total_value) {
			if (denom_i + 1 < flavours.size()) {
				++denom_i;
				denom_val = val_of(flavours[denom_i]);
			}
			else {
				break;
			}
		}

		/* Get the flavour_id to queue */
		const auto flavour_id = entity_flavour_id(flavours[denom_i]);
		const auto spawn_pos = position;

		queue_just_create_entity(
			step,
			flavour_id,
			[spawn_pos](entity_handle coin_entity, logic_step) {
				if (coin_entity.dead()) {
					return;
				}

				auto& cosm_inner = coin_entity.get_cosmos();
				auto rng = cosm_inner.get_nontemporal_rng_for(coin_entity);

				const auto spawn_offset = rng.randval(0.0f, 15.0f);
				const auto random_dir = rng.template random_point_on_unit_circle<real32>();

				auto coin_transform = transformr(spawn_pos, 0);
				coin_transform.pos += random_dir * spawn_offset;

				coin_entity.set_logic_transform(coin_transform);

				if (auto rigid_body = coin_entity.find<components::rigid_body>()) {
					rigid_body.set_velocity(random_dir * rng.randval(50.0f, 400.0f));
				}
			}
		);

		spawned += denom_val;
	}
}
