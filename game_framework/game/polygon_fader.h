#pragma once
#include "../resources/render_info.h"
#include "misc/timer.h"
#include "misc/value_animator.h"

namespace helpers {
	struct polygon_fader {
		augs::misc::timer fade_timer;

		struct trace {
			augs::misc::animator animator;
			resources::polygon poly;
		};

		std::vector<trace> traces;
		int max_traces;

		polygon_fader() : max_traces(100) {}

		void add_trace(resources::polygon poly, augs::misc::animator animator) {
			trace new_trace;
			new_trace.poly = poly;
			new_trace.animator = animator;
			traces.push_back(new_trace); 
		}

		void generate_triangles(resources::renderable::draw_input& camera_draw_input) {
			for (auto& t : traces) {
				camera_draw_input.transform = components::transform::state();
				t.poly.draw(camera_draw_input);
			}
		}

		void loop() {
			traces.erase(std::remove_if(std::begin(traces), std::end(traces), [](trace& t){
				if (t.animator.has_finished()) return true;

				for (auto& v : t.poly.model) {
					v.color.a = t.animator.get_animated();
				}

				return false;
			}), std::end(traces));

			if (max_traces > 0 && traces.size() > max_traces) {
				traces.erase(traces.begin(), traces.begin() + (traces.size() - max_traces));
			}
		}

		size_t get_num_traces() {
			return traces.size();
		}
	};
}