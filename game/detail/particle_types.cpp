#include "particle_types.h"

void general_particle::integrate(const float dt) {
	vel += acc * dt;
	pos += vel * dt;
	rotation += rotation_speed * dt;

	vel.damp(linear_damping * dt);
	augs::damp(rotation_speed, angular_damping * dt);

	lifetime_ms += dt * 1000.f;
}

void general_particle::draw(components::sprite::drawing_input basic_input) const {
	float size_mult = 1.f;

	if (shrink_when_ms_remaining > 0.f) {
		const auto alivity_multiplier = std::min(1.f, (max_lifetime_ms - lifetime_ms) / shrink_when_ms_remaining);

		size_mult *= sqrt(alivity_multiplier);

		//const auto desired_alpha = static_cast<rgba_channel>(alivity_multiplier * static_cast<float>(temp_alpha));
		//
		//if (fade_on_disappearance) {
		//	if (alpha_levels > 0) {
		//		face.color.a = desired_alpha == 0 ? 0 : ((255 / alpha_levels) * (1 + (desired_alpha / (255 / alpha_levels))));
		//	}
		//	else {
		//		face.color.a = desired_alpha;
		//	}
		//}
	}

	if (unshrinking_time_ms > 0.f) {
		size_mult *= std::min(1.f, (lifetime_ms / unshrinking_time_ms)*(lifetime_ms / unshrinking_time_ms));
	}

	auto f = face;
	f.size_multiplier.x *= size_mult;
	f.size_multiplier.y *= size_mult;

	basic_input.renderable_transform = { pos, rotation };
	face.draw(basic_input);
}
