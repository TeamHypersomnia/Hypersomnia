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

		void on_update(const delta) final {
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

	struct populate_with_delays_impl {
		float interval_ms;
		float variation_multiplier;
		rng_seed_type rng_seed;

	private:
		float elapsed_ms = 0.f;
		size_t current_interval = 0;
		std::vector<float> intervals;
	public:

		populate_with_delays_impl(const float interval_ms = 150.f, const float variation_multiplier = 0.4f, const rng_seed_type rng_seed = 0) :
		interval_ms(interval_ms), variation_multiplier(variation_multiplier), rng_seed(rng_seed) {
		}


		template <class T>
		void on_enter(const T& from_container) {
			const auto duration_ms = from_container.size() * interval_ms;
			intervals = randomization(rng_seed).make_random_intervals(from_container.size(), duration_ms, variation_multiplier);
		}

		template <class T>
		void on_update(T& target_container, const T& from_container, const delta dt) {
			elapsed_ms += dt.in_milliseconds();

			while (current_interval < intervals.size() && intervals[current_interval] < elapsed_ms) {
				target_container.insert(target_container.end(), from_container.at(current_interval));

				++current_interval;
			}
		}

		bool is_complete() const {
			return current_interval >= intervals.size();
		}
	};

	template <class T>
	class populate_with_delays : public action, public populate_with_delays_impl {
		using base = populate_with_delays_impl;

	public:
		T& target_container;
		T from_container;

		populate_with_delays(T& target, const T from, const float interval_ms, const float variation_multiplier = 0.1f, const rng_seed_type rng_seed = 0
		) :
			populate_with_delays_impl(interval_ms, variation_multiplier, rng_seed),
			target_container(target), 
			from_container(from) 
		{}

		void on_enter() final {
			base::on_enter(from_container);
		}

		void on_update(const delta dt) final {
			base::on_update(target_container, from_container, dt);
		}

		bool is_complete() const final {
			return base::is_complete();
		}
	};

	class callback_action : public action {
		std::function<void()> callback;

		callback_action(std::function<void()> callback) : callback(callback) {}

		void on_enter() final {}

		void on_update(const delta) final {
			callback();
		}

		bool is_complete() const final {
			return true;
		}
	};
}