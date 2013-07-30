#pragma once
#include "../../../entity_system/entity_system.h"
#include "../components/input_component.h"

using namespace augmentations;
using namespace entity_system;

namespace augmentations {
	namespace window {
		class glwindow;
	}
}

struct input_system : public processing_system_templated<components::input> {
	bool& quit_flag;
	
	void process_entities(world&) override;
	
	window::glwindow& input_window;
	input_system(window::glwindow&, bool& quit_flag);
};