#include "standard_actions.h"

namespace augs {
	delay_action::delay_action(const float duration_ms)
		: duration_ms(duration_ms), elapsed_ms(0.f) {
	}

	void delay_action::on_update(const delta dt) {
		elapsed_ms += dt.in_milliseconds();
	}

	bool delay_action::is_complete() const {
		return elapsed_ms >= duration_ms;
	}

	list_action::list_action(action_list&& list) : list(std::move(list)) {

	}

	void list_action::on_update(const delta dt) {
		list.update(dt);
	}

	bool list_action::is_complete() const {
		return list.is_complete();
	}
};