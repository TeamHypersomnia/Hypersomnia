#pragma once
#include "value_animator.h"

namespace augs {
	namespace misc {
		animator::method::method(float left_x, float right_x, const std::function<float (float)>& increasing_func) 
			: increasing_func(increasing_func), left_x(left_x), diff_x(right_x - left_x), left_y(increasing_func(left_x)), diff_y(increasing_func(right_x) - left_y) {}

		animator::method 
			animator::LINEAR(0.f, 1.f,								[](float x){return x;}), 
			animator::QUADRATIC(-1.f, 0.f,							[](float x){return -(x*x);}), 
			animator::SINUSOIDAL(-3.14159265f/2.f, 3.14159265f/2.f,	[](float x){return sin(x);}), 
			animator::HYPERBOLIC(-2.f, -0.5f,						[](float x){return -1.f/x;}), 
			animator::LOGARITHMIC(0.05f, 5.f,						[](float x){return log(x);}),
			animator::EXPONENTIAL(-3.f, 1.f,						[](float x){return -exp(-x);}); 

		animator::animator(float init_val, float desired_val, float milliseconds, method how, const std::function<void (float) >& callback)
			: callback(callback), init_val(init_val), diff(desired_val - init_val), milliseconds(milliseconds), how(how) {
		}

		void animator::start() {
			tm.extract<std::chrono::microseconds>();
		}

		bool animator::animate() {
			float result;
			bool finished = animate(result);
			if(callback) 
				callback(result);
			return finished;
		}
		
		bool animator::animate(float& out_val) {
			float ms = float(tm.get<std::chrono::milliseconds>());
			bool finished = ms >= milliseconds;
			out_val = finished ? init_val + diff : init_val + diff * (how.increasing_func(how.left_x + ms / milliseconds * how.diff_x) - how.left_y) / how.diff_y;
			return finished;
		}

		void animator::reset(float init, float desired) {
			init_val = init;
			diff = desired - init_val;
			start();
		}

		void animator::reset(float init, float desired, float new_milliseconds) {
			milliseconds = new_milliseconds;
			reset(init, desired);
		}

		animation::animation(int loops) : loops(loops), current(0) {}

		void animation::start() {
			current = current_loop = 0;
			if(!animators.empty()) animators[0].start();
		}

		bool animation::animate(float& out_val) {
			if(loops > -1 && current_loop >= loops) return true;
			if(current < animators.size()) {
				if (animators[current].animate(out_val)) {
					++current;
					if(current >= animators.size()) {
						current = 0;
						++current_loop;
					}
					animators[current].start();
					if(loops > -1 && current_loop >= loops) return true;
				}
				return false;
			}
			else return true;
		}
	}
}

