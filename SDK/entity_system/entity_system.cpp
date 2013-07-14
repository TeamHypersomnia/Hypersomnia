#pragma once
#include "../../augmentations.h"

#include "../../window_framework/window.h"
#include "../../config/config.h"
#include "systems.h"
#include "components.h"

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
	my_world.add_system(new movement_system);
	my_world.add_system(new render_system(gl));

	entity& player = my_world.create_entity();

	player.add(input_component());
	player.add(velocity_component());
	player.add(render_component(0, 255, 0, 0, 255));
	player.add(transform_component(rects::pointf(30, 30), rects::wh(50, 50)));

	while(!quit_flag) my_world.run();
	
	my_world.delete_entity(player);

	augmentations::deinit();
	return 0;
}