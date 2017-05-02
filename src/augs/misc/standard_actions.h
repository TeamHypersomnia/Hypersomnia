#pragma once
#include <functional>

#include "augs/math/vec2.h"

#include "augs/misc/action.h"
#include "augs/misc/delta.h"
#include "augs/misc/randomization.h"
#include "augs/misc/action_list.h"

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
			current = static_cast<T>(augs::interp(initial, to, elapsed_ms/duration_ms));
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
		float duration_ms;
		float elapsed_ms;
		float variation_multiplier;
		unsigned rng_seed;
		size_t current_interval = 0;

		std::vector<float> intervals;

		T from_container;
		T& target_container;

		populate_with_delays(T& target, const T from, const float duration_ms, const float variation_multiplier = 0.1f, const unsigned rng_seed = 0) :
			target_container(target), from_container(from), duration_ms(duration_ms), rng_seed(rng_seed), elapsed_ms(0.f), variation_multiplier(variation_multiplier) {
		}

		void on_enter() final {
			intervals = randomization(rng_seed).make_random_intervals(from_container.size(), duration_ms, variation_multiplier);
		}

		void on_update(const delta dt) final {
			elapsed_ms += dt.in_milliseconds();

			while (intervals[current_interval] < elapsed_ms && current_interval < intervals.size()) {
				target_container.insert(target_container.end(), from_container[current_interval]);

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