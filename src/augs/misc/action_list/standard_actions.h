#pragma once
#include <functional>

#include "augs/math/vec2.h"

#include "augs/misc/action_list/action.h"
#include "augs/misc/timing/delta.h"
#include "augs/misc/randomization.h"
#include "augs/misc/action_list/action_list.h"

namespace augs {
	template <class T>
	class tween_value_action : public action {
	public:
		T& current;

		T initial;
		T to;
		
		float duration_ms;
		float elapsed_ms;

		tween_value_action(T& val, const T to, const float duration_ms) 
			: current(val), initial(to), to(to), duration_ms(duration_ms), elapsed_ms(0.f) {
		}
		
		void on_enter() final {
			initial = current;
		}

		void on_update(const delta dt) final {
			current = static_cast<T>(augs::interp(initial, to, elapsed_ms/duration_ms));
			elapsed_ms += dt.in_milliseconds();

			if (is_complete()) {
				current = to;
			}
		}

		bool is_complete() const final {
			return elapsed_ms >= duration_ms;
		}
	};

	template <class T>
	class set_value_action : public action {
	public:
		T& current;
		T to;

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

	class list_action : public action {
	public:
		action_list list;

		list_action(action_list&&);

		void on_enter() final {}
		void on_update(const delta dt) final;

		bool is_complete() const final;
	};

	template <class T>
	class populate_with_delays : public action {
	public:
		T& target_container;
		T from_container;

		float duration_ms;
		float variation_multiplier;
		rng_seed_type rng_seed;

		float elapsed_ms = 0.f;
		size_t current_interval = 0;

		std::vector<float> intervals;

		populate_with_delays(T& target, const T from, const float duration_ms, const float variation_multiplier = 0.1f, const rng_seed_type rng_seed = 0) :
		target_container(target), from_container(from), duration_ms(duration_ms), variation_multiplier(variation_multiplier), rng_seed(rng_seed) {
		}

		void on_enter() final {
			intervals = randomization(rng_seed).make_random_intervals(from_container.size(), duration_ms, variation_multiplier);
		}

		void on_update(const delta dt) final {
			elapsed_ms += dt.in_milliseconds();

			while (current_interval < intervals.size() && intervals[current_interval] < elapsed_ms) {
				target_container.insert(target_container.end(), from_container.at(current_interval));

				++current_interval;
			}
		}

		bool is_complete() const final {
			return elapsed_ms >= duration_ms;
		}
	};

	class callback_action : public action {
		std::function<void()> callback;

		callback_action(std::function<void()> callback) : callback(callback) {}

		void on_enter() final {}

		void on_update(const delta dt) final {
			callback();
		}

		bool is_complete() const final {
			return true;
		}
	};
}