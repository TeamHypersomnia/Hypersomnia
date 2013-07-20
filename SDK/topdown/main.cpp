#pragma once
#include "../../augmentations.h"

#include "../../window_framework/window.h"
#include "../../config/config.h"
#include "systems/rendering.h"

using namespace augmentations;
using namespace entity_system;

int main() {
	augmentations::init();

	config::input_file cfg("window_config.txt");
	 
	window::glwindow gl;
	gl.create(cfg, rects::wh(100, 100));
	gl.set_show(gl.SHOW);

	
	bool quit_flag = false;
	glm::normalize(glm::vec3());
	world my_world;
	my_world.add_system(new render_system(gl));
	
	while(!quit_flag) my_world.run();
	
	augmentations::deinit();
	return 0;
}
