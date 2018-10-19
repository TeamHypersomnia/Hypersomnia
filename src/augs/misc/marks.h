#pragma once
#include <map>
#include "augs/window_framework/event.h"

namespace augs {
	enum class marks_state {
		// GEN INTROSPECTOR enum class augs::marks_state
		NONE,
		JUMPING,
		MARKING
		// END GEN INTROSPECTOR
	};

	enum class marks_result_type {
		NONE,
		ONLY_FETCH_INPUT,
		JUMPED
	};

	template <class T>
	struct marks {
		struct result {
			marks_result_type result;
			T chosen_value;
		};

		using code_point_type = decltype(event::change::character_data::code_point);

		// GEN INTROSPECTOR struct augs::marks class T
		std::map<code_point_type, T> marks;
		marks_state state = marks_state::NONE;
		T previous;
		int eat_keys = 0;
		// END GEN INTROSPECTOR

		result control(
			const event::change&,
			const T& current
		);
	};
}
