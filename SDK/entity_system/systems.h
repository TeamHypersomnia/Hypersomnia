#pragma once
#include "../../entity_system/entity_system.h"
#include "../../window_framework/window.h"
#include "components.h"	
#include "../../utility/delta_accumulator.h"
#include <array>

using namespace augmentations;
using namespace entity_system;

struct render_system : public processing_system_templated<transform_component, render_component> {
	void process_entities() override;
	
	window::glwindow& output_window;

	render_system(window::glwindow&);
};

struct movement_system : public processing_system_templated<transform_component, velocity_component> {
	util::delta_accumulator accumulator;
	movement_system();

	void process_entities() override;
};

struct input_system : public processing_system_templated<velocity_component, input_component> {
	bool& quit_flag;
	
	enum intents {
		GO_DOWN, GO_LEFT, GO_RIGHT, GO_UP
	};

	std::array<bool, 4> states;
	
	void process_entities() override;
	
	window::glwindow& input_window;
	input_system(window::glwindow&, bool& quit_flag);
};
