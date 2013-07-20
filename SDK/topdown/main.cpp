#pragma once
#include "../../augmentations.h"

#include "../../window_framework/window.h"
#include "../../config/config.h"
#include "systems/render_system.h"
#include "systems/input_system.h"

using namespace augmentations;
using namespace entity_system;

int main() {
	augmentations::init();

	config::input_file cfg("window_config.txt");
	 
	window::glwindow gl;
	gl.create(cfg, rects::wh(100, 100));
	gl.set_show(gl.SHOW);

	
	bool quit_flag = false;
	world my_world;
	my_world.add_system(new input_system(gl, quit_flag));
	my_world.add_system(new render_system(gl));

	entity& rect = my_world.create_entity();
	rect.add(components::render(0));
	rect.add(components::transform());
	rect.add(components::input());
	
	rect.get<components::transform>().pos = rects::pointf(rects::wh(gl.get_window_rect()))*0.5;
	rect.get<components::transform>().size = rects::wh(50, 60);
	rect.get<components::transform>().rotation = 45.0;

	entity& crosshair = my_world.create_entity();
	crosshair.add(components::render(0));
	crosshair.add(components::transform());
	crosshair.add(components::input());

	crosshair.get<components::transform>().size = rects::wh(20, 20);


	while(!quit_flag) {
		rect.get<components::transform>().rotation += 0.001;
		my_world.run();
	}
	
	augmentations::deinit();
	return 0;
}
