#pragma once

template <class E>
bool has_hurting_velocity(const E& self) {
	if (const auto melee_def = self.template find<invariants::melee>()) {
		if (const auto body = self.template find<components::rigid_body>()) {
			const auto& throw_def = melee_def->throw_def;

			const auto vel = body.get_velocity();
			const auto min_vel = throw_def.min_velocity_to_hurt;
			const auto min_vel_sq = min_vel * min_vel;

			return vel.length_sq() >= min_vel_sq;
		}
	}

	return false;
}

template <class E>
bool is_like_thrown_melee(const E& self) {
	if (const auto melee_def = self.template find<invariants::melee>()) {
		(void)melee_def;

		if (const auto sender = self.template find<components::sender>()) {
			return sender->is_set();
		}
	}

	return false;
}


