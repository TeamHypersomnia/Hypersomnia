#pragma once
#include "timer.h"
#include <functional>
#include <vector>

namespace augs {
	namespace misc {
		class animator {
			timer tm;
			std::function<void (float)> callback;
			float init_val, diff, milliseconds;
		public:
			struct method {
				std::function<float (float)> increasing_func;
				float left_x, diff_x, left_y, diff_y;

				method(float left_x, float right_x, const std::function<float (float)>& increasing_func); 
				
				static float linear(float);
				static float quadratic(float);
				static float sinusoidal(float);
				static float hyperbolic(float);
				static float logarithmic(float);
				static float exponential(float);
			} how;
			static method LINEAR, QUADRATIC, SINUSOIDAL, HYPERBOLIC, LOGARITHMIC, EXPONENTIAL;
			animator(float init_val, float desired_val, float milliseconds, method how, const std::function<void (float) >& callback = nullptr);
			
			void start();
			bool animate();
			bool animate(float& out_val);

			void reset(float init_val, float desired_val);
			void reset(float init_val, float desired_val, float milliseconds);
		};

		struct animation {
			std::vector<animator> animators;

			int loops, current_loop;
			unsigned current;
			animation(int loops = -1);

			void start();
			bool animate(float& out_val); 
		};
	}
}

