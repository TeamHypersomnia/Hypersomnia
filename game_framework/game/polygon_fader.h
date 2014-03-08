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

		void generate_triangles(components::transform::state camera_transform, resources::buffer* output_buffer, rects::xywh visible_area) {
			resources::renderable::draw_input my_draw_input;
			my_draw_input.camera_transform = camera_transform;
			my_draw_input.output = output_buffer;
			my_draw_input.visible_area = rects::ltrb(visible_area);

			for (auto& t : traces) 
				t.poly.draw(my_draw_input);
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