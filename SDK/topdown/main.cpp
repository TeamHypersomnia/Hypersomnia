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

	//physics.world.SetGravity(vec2<>(0,0));
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
	entity& ground = my_world.create_entity();
	entity& crosshair = my_world.create_entity();

	sprite small_sprite(&tex);
	sprite my_sprite(&tex);
	sprite bigger_sprite(&tex);
	bigger_sprite.size.w = 400;
	small_sprite.size /= 2;

	rect.add(components::render(0, &my_sprite));
	rect.add(components::transform(vec2<float>(30, 10)));
	topdown::create_physics_component(rect, physics.b2world);

	ground.add(components::render(0, &bigger_sprite));
	ground.add(components::transform(vec2<float>(50, 100)));
	topdown::create_physics_component(ground, physics.b2world, b2_staticBody);

	crosshair.add(components::render(0, &small_sprite, components::render::GUI));
	crosshair.add(components::transform(vec2<float>(200, 10)));
	crosshair.add(components::input());
	
	rects::xywh screen_rect(0, 0, gl.get_window_rect().w, gl.get_window_rect().h);

	world_camera.add(components::camera(screen_rect, screen_rect, 0, components::render::WORLD));
	world_camera.add(components::transform());

	gui_camera.add(components::camera(screen_rect, screen_rect, 0, components::render::GUI));
	gui_camera.add(components::transform());

	/*b2BodyDef def;
	b2Body* body[2]; 
	def.type = b2_dynamicBody;
	def.angle = 0;
	def.userData = (void*)&rect;
	body[0] = physics.b2world.CreateBody(&def);
	def.type = b2_staticBody;
	def.userData = (void*)&ground;
	body[1] = physics.b2world.CreateBody(&def);
	
	body[0]->SetTransform(vec2<float>(2, 1), 1.f);
	body[1]->SetTransform(vec2<float>(2, 5), 0.f);
	
	b2PolygonShape shape[2];
	shape[0].SetAsBox(139 / 100.0, 172 / 100.0);
	shape[1].SetAsBox(139 / 100.0, 172 / 100.0);

	b2FixtureDef fixdef;
	fixdef.density = 10.0;
	fixdef.shape = &shape[0];

	body[0]->CreateFixture(&fixdef);
	fixdef.shape = &shape[1];
	body[1]->CreateFixture(&fixdef);*/
	
	//rect.get<components::physics>().body = body[0];
	//ground.get<components::transform>().current.pos = body[1]->GetTransform().p;
	//ground.get<components::physics>().body = body[1];
	
	//body[0]->ResetMassData();
		//body[0]->ApplyLinearImpulse(vec2<>(0, -10000), body[0]->GetLocalCenter());
		//body[0]->SetLinearDamping(0.1);
	//	body[0]->SetAngularDamping(0.1);
//	body[0]->ApplyAngularImpulse(10000);

	while (!quit_flag) {
		my_world.run();
	}
	
	augmentations::deinit();
	return 0;
}
