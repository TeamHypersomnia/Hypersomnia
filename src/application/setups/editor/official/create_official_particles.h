#pragma once

template <class A, class B>
auto float_range(const A a, const B b) {
	return augs::bound<float>(static_cast<float>(a), static_cast<float>(b));
}

void create_particles(editor_resource_pools& pools) {
	auto create_particles = [&](const official_particles id) -> auto& {
		return create_official(id, pools).editable;
	};

	auto default_bounds = [](particles_emission& em) {
		em.swings_per_sec_bound = { { 0.15f, 0.25f },{ 0.30f, 0.50f } };
		em.swing_spread_bound = { { 0.5f, 1.0f },{ 5.0f, 6.0f } };
	};

	{
		// TODO this is just a test, will need proper defs
		auto& effect = create_particles(official_particles::GLASS_DAMAGE);

		{
			particles_emission em;
			em.rotation_speed = float_range(0, 0);

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(13.f, 13.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(20.f, 20.f);

			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;
			em.randomize_acceleration = false;

			effect.emissions.push_back(em);
		}
	}

}
