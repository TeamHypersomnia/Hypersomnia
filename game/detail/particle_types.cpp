#include "particle_types.h"

void general_particle::integrate(const float dt) {
	vel += acc * dt;
	pos += vel * dt;
	rotation += rotation_speed * dt;

	vel.damp(linear_damping * dt);
	augs::damp(rotation_speed, angular_damping * dt);

	lifetime_ms += dt * 1000.f;
}