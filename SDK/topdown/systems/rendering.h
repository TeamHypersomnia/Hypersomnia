#pragma once
#include "../components.h"
#include "../../../window_framework/window.h"

using namespace augmentations;
using namespace entity_system;

class render_system : public processing_system_templated<components::transform, components::render> {
	struct quad {
		struct vertex {
			int x, y;
			unsigned char r, g, b, a;
			float u, v;
		};
		
		vertex vertices[4];
	};

	std::vector<quad> quads;
public:
	void process_entities() override;
	
	window::glwindow& output_window;

	render_system(window::glwindow&);

	struct bucket {
		std::vector<entity*> targets;
	};

	void add(entity*) override;
	void remove(entity*) override;

	std::vector<bucket> layers;
};