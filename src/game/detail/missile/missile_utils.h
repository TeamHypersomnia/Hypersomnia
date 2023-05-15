#pragma once

void play_collision_sound(
	const real32 strength,
	const transformr location,
	const const_entity_handle sub,
	const const_entity_handle col,
	const logic_step step
);

template <class R, class F>
static void spawn_bullet_remnants(
	allocate_new_entity_access access,
	const logic_step step,
	R& rng,
	F flavours,
	const vec2& collision_normal,
	const vec2& impact_dir,
	const vec2& impact_point
) {
	auto& cosm = step.get_cosmos();

	shuffle_range(flavours, rng);

	auto total_spawned = std::min(static_cast<int>(flavours.size()), 1);

	auto how_many_along_normal = rng.randval(total_spawned - 1, total_spawned);

	for (int i = 0; i < total_spawned; ++i) {
		const auto& r_id = flavours[i];
		const auto speed = rng.randval(1000.f, 4800.f);

		vec2 vel;

		if (how_many_along_normal) {
			const auto sgn = rng.randval(0, 1);

			auto amount_rotated = rng.randval(70.f, 87.f);

			if (sgn == 1) {
				amount_rotated = -amount_rotated;
			}

			vel = vec2(collision_normal).rotate(amount_rotated) * speed;

			--how_many_along_normal;
		}
		else {
			vel = -1 * vec2(impact_dir).rotate(rng.randval(-40.f, 40.f)) * speed;
		}

		cosmic::create_entity(
			access,
			cosm,
			r_id,
			[&, vel](const auto typed_remnant, auto&&...) {
				auto spawn_offset = vec2(vel).normalize() * rng.randval(55.f, 60.f);
				const auto rot = rng.randval(0.f, 360.f);
				
				typed_remnant.set_logic_transform(transformr(impact_point + spawn_offset, rot));
			},
			[vel, &rng, &step](const auto typed_remnant) {
				typed_remnant.template get<components::rigid_body>().set_velocity(vel);
				typed_remnant.template get<components::rigid_body>().set_angular_velocity(rng.randval(1060.f, 4000.f));

				const auto& effect = typed_remnant.template get<invariants::remnant>().trace_particles;

				effect.start(
					step,
					particle_effect_start_input::orbit_local(typed_remnant, { vec2::zero, 180 } ),
					always_predictable_v
				);
			}
		);
	}
}


template <class T>
static void make_velocity_face_body_orientation(
	const T& typed_missile
) {
	const auto body = typed_missile.template get<components::rigid_body>();
	const auto current_vel = body.get_velocity();
	const auto tr = body.get_transform();

	const auto current_dir = tr.get_direction();

	//const auto new_tr = transformr(tr.pos, current_vel.degrees());
	const auto new_vel = current_dir * current_vel.length();

	RIC_LOG_NVPS(new_vel);
	body.set_velocity(new_vel);
}

