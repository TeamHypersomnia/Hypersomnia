#pragma once
#include "value_animator.h"

namespace augmentations {
	namespace util {
		animator::method::method(float left_x, float right_x, const std::function<float (float)>& increasing_func) 
			: increasing_func(increasing_func), left_x(left_x), diff_x(right_x - left_x), left_y(increasing_func(left_x)), diff_y(increasing_func(right_x) - left_y) {}

		animator::method 
			animator::LINEAR(0.f, 1.f,								[](float x){return x;}), 
			animator::QUADRATIC(-1.f, 0.f,							[](float x){return -(x*x);}), 
			animator::SINUSOIDAL(-3.14159265f/2., 3.14159265f/2.,	[](float x){return sin(x);}), 
			animator::HYPERBOLIC(-2.f, -0.5f,						[](float x){return -1.f/x;}), 
			animator::LOGARITHMIC(0.05f, 5.f,						[](float x){return log(x);}),
			animator::EXPONENTIAL(-3.f, 1.f,						[](float x){return -exp(-x);}); 

		animator::animator(const std::function<void (float)>& callback, float init_val, float desired_val, float miliseconds, method how) 
			: callback(callback), init_val(init_val), diff(desired_val - init_val), miliseconds(miliseconds), how(how) {
		}

		void animator::start() {
			tm.microseconds();
		}

		bool animator::animate() {
			float ms = float(tm.get_miliseconds());
			bool finished = ms >= miliseconds;
			callback(finished ? init_val + diff : init_val + diff * (how.increasing_func(how.left_x + ms / miliseconds * how.diff_x) - how.left_y) / how.diff_y);
			return finished;
		}

		animation::animation(int loops) : loops(loops), current(0) {}

		void animation::start() {
			current = current_loop = 0;
			if(!animators.empty()) animators[0]->start();
		}

		bool animation::animate() {
			if(loops > -1 && current_loop >= loops) return true;
			if(current < animators.size()) {
				if(animators[current]->animate()) {
					++current;
					if(current >= animators.size()) {
						current = 0;
						++current_loop;
					}
					animators[current]->start();
					if(loops > -1 && current_loop >= loops) return true;
				}
				return false;
			}
			else return true;
		}
	}
}

