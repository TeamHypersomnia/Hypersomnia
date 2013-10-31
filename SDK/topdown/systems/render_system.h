#pragma once
#include "window_framework/window.h"

#include "entity_system/processing_system.h"

#include "../components/transform_component.h"
#include "../components/render_component.h"

#include "../resources/vertex.h"
#include "../resources/fbo.h"

using namespace augmentations;
using namespace entity_system;

class render_system : public processing_system_templated<components::transform, components::render> {
	resources::buffer triangles;

	friend class camera_system;
public:
	fbo scene_fbo, postprocess_fbo;

	float visibility_expansion;
	float max_visibility_expansion_distance;
	int draw_visibility;

	window::glwindow& output_window;
	render_system(window::glwindow& output_window);

	void process_entities(world&) override;

	void draw(rects::xywh visible_area, components::transform::state, unsigned mask);
	void render(rects::xywh visible_area);
};