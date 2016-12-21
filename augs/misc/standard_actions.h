#pragma once
#include "action.h"
#include "augs/math/vec2.h"
#include "augs/misc/delta.h"

namespace augs {
	template <class T>
	class tween_value_action : public action {
	public:
		T initial;
		T to;
		
		float duration_ms;
		float elapsed_ms;

		T& current;

		tween_value_action(T& val, const T to, const float duration_ms) 
			: current(val), initial(to), to(to), duration_ms(duration_ms), elapsed_ms(0.f) {
		}
		
		void on_enter() final {
			initial = current;
		}

		void on_update(const delta dt) final {
			current = augs::interp(initial, to, elapsed_ms/duration_ms);
			elapsed_ms += dt.in_milliseconds();
		}

		bool is_complete() const final {
			return elapsed_ms >= duration_ms;
		}
	};

	template <class T>
	class set_value_action : public action {
	public:
		T to;
		T& current;

		set_value_action(T& val, const T to)
			: current(val), to(to) {
		}

		void on_enter() final {}

		void on_update(const delta dt) final {
			current = to;
		}

		bool is_complete() const final {
			return true;
		}
	};

	class delay_action : public action {
	public:
		float duration_ms;
		float elapsed_ms;

		delay_action(const float duration_ms);
		
		void on_enter() final {}
		void on_update(const delta dt) final;
		bool is_complete() const final;
	};
}