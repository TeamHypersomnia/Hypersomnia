#pragma once
#include "../../augmentations.h"

#include "../../window_framework/window.h"
#include "../../config/config.h"

#include "systems/physics_system.h"
#include "systems/camera_system.h"
#include "systems/render_system.h"
#include "systems/input_system.h"

#include "game\body_helper.h"

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

	input_system input(gl, quit_flag);
	physics_system physics;
	render_system render(gl);
	camera_system camera(render);

	texture_baker::image img;
	texture_baker::texture tex;
	texture_baker::atlas atl;

	img.from_file(L"C:\\VVV\\head.png");
	tex.set(&img);
	atl.textures.push_back(&tex);
	atl.pack();
	atl.create_image(4, true);
	atl.build();
	atl.bind();

	my_world.add_system(&input);
	my_world.add_system(&physics);
	my_world.add_system(&render);
	my_world.add_system(&camera);

	entity& world_camera = my_world.create_entity();
	entity& gui_camera = my_world.create_entity();

	entity& rect = my_world.create_entity();
	entity& rect1 = my_world.create_entity();
	entity& rect2 = my_world.create_entity();
	entity& ground = my_world.create_entity();
	entity& crosshair = my_world.create_entity();

	sprite small_sprite(&tex);
	sprite my_sprite(&tex);
	sprite bigger_sprite(&tex);
	bigger_sprite.size.w = 400;
	small_sprite.size.w = 100;
	small_sprite.size.h = 100;

	rect.add(components::render(0, &my_sprite));
	rect.add(components::transform(vec2<float>(500, -50)));
	topdown::create_physics_component(rect, physics.b2world);
	rect1.add(components::render(0, &my_sprite));
	rect1.add(components::transform(vec2<float>(470, 50)));
	topdown::create_physics_component(rect1, physics.b2world);
	rect2.add(components::render(0, &small_sprite));
	rect2.add(components::transform(vec2<float>(400, 0), 45 * 0.01745329251994329576923690768489));
	topdown::create_physics_component(rect2, physics.b2world);

	ground.add(components::render(0, &bigger_sprite));
	ground.add(components::transform(vec2<float>(400, 400)));
	topdown::create_physics_component(ground, physics.b2world, b2_staticBody);
    ground.get<components::physics>().body->GetFixtureList()->SetRestitution(1.0f);

	crosshair.add(components::render(0, &small_sprite, components::render::GUI));
	crosshair.add(components::transform(vec2<float>(200, 10)));
	crosshair.add(components::input());
	
	rects::xywh screen_rect(0, 0, gl.get_window_rect().w, gl.get_window_rect().h);

	world_camera.add(components::camera(screen_rect, screen_rect, 0, components::render::WORLD));
	world_camera.add(components::transform());

	gui_camera.add(components::camera(screen_rect, screen_rect, 0, components::render::GUI));
	gui_camera.add(components::transform());

	while (!quit_flag)
		my_world.run();
	
	augmentations::deinit();
	return 0;
}
