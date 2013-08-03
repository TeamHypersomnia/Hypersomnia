#pragma once
#include <Box2D\Collision\b2DynamicTree.h>
#include "window_framework/window.h"

#include "entity_system/processing_system.h"

#include "../components/transform_component.h"
#include "../components/render_component.h"

#include "../renderable.h"

using namespace augmentations;
using namespace entity_system;

class render_system : public processing_system_templated<components::transform, components::render> {
	buffer triangles;

	friend class camera_system;

	std::unordered_map<unsigned, std::vector<entity*>> entities_by_mask;
public:

	window::glwindow& output_window;
	render_system(window::glwindow& output_window);

	void add(entity*) override;
	void remove(entity*) override;

	void process_entities(world&) override;

	void draw(rects::xywh visible_area, components::transform, unsigned mask);
	void render();
};