#pragma once
#include <functional>
#include <vector>

#include "augs/misc/timer.h"

namespace augs {
	class value_animator {
		timer tm;
		std::function<void(float)> callback;
		float init_val, diff, milliseconds;
	public:
		struct method {
			std::function<float(float)> increasing_func;
			float left_x, diff_x, left_y, diff_y;

			method(float left_x, float right_x, const std::function<float(float)>& increasing_func);

			static float linear(float);
			static float quadratic(float);
			static float sinusoidal(float);
			static float hyperbolic(float);
			static float logarithmic(float);
			static float exponential(float);
		} how;
		static method LINEAR, QUADRATIC, SINUSOIDAL, HYPERBOLIC, LOGARITHMIC, EXPONENTIAL;
		value_animator(float init_val, float desired_val, float milliseconds, method how, const std::function<void(float) >& callback = nullptr);

		value_animator(float init_val = 0.f, float desired_val = 0.f, float milliseconds = 0.f);

		void set_linear() { how = LINEAR; }
		void set_quadratic() { how = QUADRATIC; }
		void set_sinusoidal() { how = SINUSOIDAL; }
		void set_hyperbolic() { how = HYPERBOLIC; }
		void set_logarithmic() { how = LOGARITHMIC; }
		void set_exponential() { how = EXPONENTIAL; }

		float get_animated();

		void start();
		bool animate();
		bool animate(float& out_val);

		bool has_finished() const {
			return tm.get<std::chrono::milliseconds>() >= milliseconds;
		}

		void reset(float init_val, float desired_val);
		void reset(float init_val, float desired_val, float milliseconds);
	};

	struct value_animation {
		std::vector<value_animator> animators;

		int loops;
		int current_loop = 0;
		unsigned current = 0u;
		value_animation(int loops = -1);

		void start();
		bool animate(float& out_val);
	};
}

