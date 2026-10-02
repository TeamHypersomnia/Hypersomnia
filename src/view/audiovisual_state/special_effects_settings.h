#pragma once

/*
	Tunes the pixel art explosion particles (see exploding_ring_input),
	given for an explosion the size of a force grenade's - they scale with the explosion's radius.

	particle_jitter_radius displaces the particles radially to make the ring's edges ragged,
	particle_jitter_degrees randomizes their rotation off the ring's tangent.
	particle_random_acceleration accelerates each particle in a random direction.
	The _min/_max pairs give the range each particle randomizes its value from.
	particle_lifetime_mult is relative to the exploding ring's duration.
	particle_size_mult scales all particles up regardless of the explosion's radius.

	The particles' color goes from hot (the brightened inner ring's color) to the ring's color
	until particle_hot_fraction of the ring's duration, then from particle_cool_from_fraction
	cools down to the ring's color darkened to particle_cool_brightness (or to the explosion's own end color)
	over particle_cool_duration_fraction of the ring's duration.
	Each transition goes in particle_color_steps discrete steps (0 = smoothly).
	Explosions with a palette go through all of its colors over particle_palette_duration_fraction of the ring's duration instead.

	particle_ease_out is the time constant of the particles' exponential slowdown, relative to the ring's duration
	(0 = constant speed). Their paths stay the same, only front-loaded: they reach the ring's end at the same time.
	particle_speed_stretch lengthens the particles at full speed (0.6 = 60% longer).

	particle_ember_fraction of the particles become embers: smaller, slower, lingering particle_ember_lifetime_mult times longer.
	Only in the red to orange explosions.

	thin_ring_thickness is the thickness of the thin rings accompanying the explosion's rings.
*/

struct explosion_particles_settings {
	// GEN INTROSPECTOR struct explosion_particles_settings
	float particle_jitter_radius = 5.f;
	float particle_jitter_degrees = 0.f;
	float particle_tangential_speed_min = 300.f;
	float particle_tangential_speed_max = 4250.f;
	float particle_random_acceleration_min = 0.f;
	float particle_random_acceleration_max = 1000.f;
	float particle_size_mult = 1.5f;
	float particle_lifetime_mult_min = 0.1f;
	float particle_lifetime_mult_max = 1.4f;
	float particle_hot_fraction = 0.15f;
	float particle_cool_from_fraction = 1.f;
	float particle_cool_brightness = 0.65f;
	float particle_cool_duration_fraction = 0.15f;
	float particle_palette_duration_fraction = 0.7f;
	int particle_color_steps = 255;
	float particle_ease_out = 0.35f;
	float particle_speed_stretch = 0.f;
	float particle_ember_fraction = 0.03f;
	float particle_ember_lifetime_mult = 3.f;
	float thin_ring_thickness = 6.f;
	// END GEN INTROSPECTOR

	bool operator==(const explosion_particles_settings& b) const = default;

	static auto fire_defaults() {
		auto result = explosion_particles_settings();

		result.particle_jitter_radius = 0.f;
		result.particle_tangential_speed_min = 800.f;
		result.particle_tangential_speed_max = 2250.f;
		result.particle_size_mult = 3.4f;
		result.particle_palette_duration_fraction = 2.2f;
		result.particle_ember_fraction = 0.02f;
		result.particle_ember_lifetime_mult = 1.8f;
		result.thin_ring_thickness = 39.f;

		return result;
	}
};

/*
	fire_particles are for the explosions with fire_particles set (force grenades, skull rockets, bombs),
	standard_particles for all the others.
*/

struct explosions_settings {
	// GEN INTROSPECTOR struct explosions_settings
	float sparkle_amount = 1.f;
	float thunder_amount = 1.f;
	float smoke_amount = 0.7f;
	int max_particles_per_explosion = 2000;
	explosion_particles_settings standard_particles;
	explosion_particles_settings fire_particles = explosion_particles_settings::fire_defaults();
	// END GEN INTROSPECTOR
	
	bool operator==(const explosions_settings& b) const = default;
};

struct special_effects_settings {
	// GEN INTROSPECTOR struct special_effects_settings
	explosions_settings explosions;
	float muzzle_flash_intensity = 1.f;
	float particle_stream_amount = 1.f;
	float particle_burst_amount = 1.f;
	// END GEN INTROSPECTOR

	bool operator==(const special_effects_settings& b) const = default;
};
