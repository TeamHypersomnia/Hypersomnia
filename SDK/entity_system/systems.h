#pragma once
#include "../../entity_system/entity_system.h"
#include "../../window_framework/window.h"
#include "components.h"	

using namespace augmentations;
using namespace entity_system;

struct render_system : public processing_system_templated<transform_component, render_component> {
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

struct movement_system : public processing_system_templated<transform_component, velocity_component> {
	void process_entities() override;
};

struct input_system : public processing_system_templated<velocity_component, input_component> {
	bool& quit_flag;
	void process_entities() override;
	
	window::glwindow& input_window;
	input_system(window::glwindow&, bool& quit_flag);
};
