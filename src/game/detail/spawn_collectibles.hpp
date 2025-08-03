#pragma once
#include "game/cosmos/create_entity.hpp"

inline void spawn_coins(
	const allocate_new_entity_access access,
	const money_type total_value,
	const vec2 position,
	const logic_step step,
	std::array<constrained_entity_flavour_id<invariants::touch_collectible>, 3> flavours
) {
	auto& cosm = step.get_cosmos();

	auto val_of = [&](auto f) {
		return cosm.on_flavour(f, [&](auto& typed_flavour) {
			return typed_flavour.template get<invariants::touch_collectible>().money_value;
		});
	};

	std::reverse(flavours.begin(), flavours.end());

	std::size_t denom_i = 0;
	money_type denom_val = val_of(flavours[denom_i]);

	for (money_type spawned = 0; spawned < total_value;) {
		while (spawned + denom_val > total_value) {
			if (denom_i + 1 < flavours.size()) {
				++denom_i;
				denom_val = val_of(flavours[denom_i]);
			}
			else {
				break;
			}
		}

		cosmic::create_entity(
			access,
			cosm,

			flavours[denom_i],

			[&](const auto coin_entity, auto&&...) {
				auto rng = cosm.get_nontemporal_rng_for(coin_entity);

				const auto spawn_offset = rng.randval(0.0f, 15.0f);
				const auto random_dir = rng.template random_point_on_unit_circle<real32>();

				auto coin_transform = transformr(position, 0);
				coin_transform.pos += random_dir * spawn_offset;

				coin_entity.set_logic_transform(coin_transform);

				const auto& rigid_body = coin_entity.template get<components::rigid_body>();
				rigid_body.set_velocity(random_dir * rng.randval(50.0f, 400.0f));
			},

			[&](const auto) {}
		);

		spawned += denom_val;
	}
}

