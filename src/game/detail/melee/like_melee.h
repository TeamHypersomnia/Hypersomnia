#pragma once
#include "game/enums/melee_fighter_state.h"

template <class E>
bool has_hurting_velocity(const E& self) {
	if (const auto melee_def = self.template find<invariants::melee>()) {
		if (const auto body = self.template find<components::rigid_body>()) {
			const auto& throw_def = melee_def->throw_def;

			const auto vel = body.get_velocity();
			const auto min_speed = throw_def.min_speed_to_hurt;
			const auto min_speed_sq = min_speed * min_speed;

			return vel.length_sq() >= min_speed_sq;
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

template <class E>
bool is_like_melee_in_action(const E& self) {
	if (const auto melee_def = self.template find<invariants::melee>()) {
		(void)melee_def;

		if (const auto slot = self.get_current_slot()) {
			if (const auto fighter = slot.get_container().template find<components::melee_fighter>()) {
				return fighter->state == melee_fighter_state::IN_ACTION;
			}
		}
	}

	return false;
}


