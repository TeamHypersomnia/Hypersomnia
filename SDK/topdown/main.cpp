#pragma once
#include "../../augmentations.h"

#include "../../window_framework/window.h"
#include "../../config/config.h"

#include "systems/physics_system.h"
#include "systems/movement_system.h"
#include "systems/camera_system.h"
#include "systems/render_system.h"
#include "systems/input_system.h"
#include "systems/crosshair_system.h"
#include "systems/lookat_system.h"
#include "systems/chase_system.h"

#include "messages/collision_message.h"
#include "messages/moved_message.h"
#include "messages/intent_message.h"

#include "game/body_helper.h"

using namespace augmentations;
using namespace entity_system;
using namespace messages;

int main() {
	augmentations::init();

	config::input_file cfg("window_config.txt");
	 
	window::glwindow gl;
	gl.create(cfg, rects::wh(100, 100));
	gl.set_show(gl.SHOW);
	window::cursor(false);

#define IMAGES 4

	texture_baker::image img[IMAGES];
	texture_baker::texture tex[IMAGES];
	texture_baker::atlas atl;

	img[0].from_file(L"C:\\VVV\\rect.png");
	img[1].from_file(L"C:\\VVV\\rect.png");
	img[2].from_file(L"C:\\VVV\\rect.png");
	img[3].from_file(L"C:\\VVV\\rect.png");

	for (int i = 0; i < IMAGES; ++i) {
		tex[i].set(img + i);
		atl.textures.push_back(tex + i);
	}

	atl.pack();
	atl.create_image(4, true);
	atl.build();
	atl.bind();

	/* destroy the raw image as it is already uploaded to GPU */
	atl.img.destroy();
	atl.linear();

	sprite small_sprite(tex + 0);
	sprite my_sprite(tex + 0);
	sprite bigger_sprite(tex + 0);
	sprite player_sprite(tex + 1);
	sprite crosshair_sprite(tex + 2);
	sprite rifle_sprite(tex + 3);

	player_sprite.size = vec2<float>(100,100);

	bigger_sprite.size.w = 400;
	small_sprite.size.w = 100;
	small_sprite.size.h = 100;
	my_sprite.size.w = 200;
	my_sprite.size.h = 120;

	bool quit_flag = false;

	world my_world;

	input_system input(gl, quit_flag);
	movement_system movement;
	crosshair_system crosshairs;
	lookat_system lookat;
	physics_system physics;
	render_system render(gl);
	camera_system camera(render);
	chase_system chase;

	input_system::context main_context;
	main_context.raw_id_to_intent[window::event::mouse::motion] = intent_message::intent::AIM;
	main_context.raw_id_to_intent[window::event::mouse::ldown] = intent_message::intent::SHOOT;
	main_context.raw_id_to_intent[window::event::keys::W] = intent_message::intent::MOVE_FORWARD;
	main_context.raw_id_to_intent[window::event::keys::S] = intent_message::intent::MOVE_BACKWARD;
	main_context.raw_id_to_intent[window::event::keys::A] = intent_message::intent::MOVE_LEFT;
	main_context.raw_id_to_intent[window::event::keys::D] = intent_message::intent::MOVE_RIGHT;
	input.active_contexts.push_back(&main_context);

	physics.b2world.SetGravity(vec2<>(0.f, 0.f));

	my_world.add_system(&input);
	my_world.add_system(&movement);
	my_world.add_system(&crosshairs);
	my_world.add_system(&lookat);
	my_world.add_system(&chase);
	my_world.add_system(&physics);
	my_world.add_system(&render);
	my_world.add_system(&camera);

	entity& world_camera = my_world.create_entity();
	entity& gui_camera = my_world.create_entity();

	entity& player_visual = my_world.create_entity();
	entity& player_physical = my_world.create_entity();
	entity& rect = my_world.create_entity();
	entity& rect1 = my_world.create_entity();
	entity& rect2 = my_world.create_entity();
	entity& ground = my_world.create_entity();
	entity& crosshair = my_world.create_entity();

	components::input player_input;
	player_input.intents.add(intent_message::intent::MOVE_FORWARD);
	player_input.intents.add(intent_message::intent::MOVE_BACKWARD);
	player_input.intents.add(intent_message::intent::MOVE_LEFT);
	player_input.intents.add(intent_message::intent::MOVE_RIGHT);

	player_visual.add(components::render(0, &player_sprite));
	player_visual.add(components::transform(vec2<float>(gl.get_screen_rect()) / 2));
	player_visual.add(components::lookat(&crosshair));
	player_visual.add(components::chase(&player_physical));

	player_physical.add(components::render(0, &player_sprite));
	player_physical.add(components::transform(vec2<float>(gl.get_screen_rect()) / 2));
	player_physical.add(components::movement(vec2<float>(50000.f, 50000.f), 500.f));
	player_physical.add(player_input);
	topdown::create_physics_component(player_physical, physics.b2world, b2_dynamicBody);
	player_physical.get<components::physics>().body->SetLinearDamping(20.0f);
	player_physical.get<components::physics>().body->SetAngularDamping(20.0f);
	player_physical.get<components::physics>().body->SetFixedRotation(true);

	rect.add(components::render(0, &my_sprite));
	rect.add(components::transform(vec2<float>(500, -50)));
	topdown::create_physics_component(rect, physics.b2world);
	rect1.add(components::render(0, &my_sprite));
	rect1.add(components::transform(vec2<float>(470, 50)));
	topdown::create_physics_component(rect1, physics.b2world);
	rect2.add(components::render(0, &small_sprite));
	rect2.add(components::transform(vec2<float>(400, 0), 45 * 0.01745329251994329576923690768489));
	topdown::create_physics_component(rect2, physics.b2world);
	rect .get<components::physics>().body->ApplyLinearImpulse(vec2<float>(0.f, 2000.f*PIXELS_TO_METERS),  rect.get<components::physics>().body->GetWorldCenter());
	rect1.get<components::physics>().body->ApplyLinearImpulse(vec2<float>(0.f, 2000.f*PIXELS_TO_METERS), rect1.get<components::physics>().body->GetWorldCenter());
	rect2.get<components::physics>().body->ApplyLinearImpulse(vec2<float>(0.f, 2000.f*PIXELS_TO_METERS), rect2.get<components::physics>().body->GetWorldCenter());

	ground.add(components::render(0, &bigger_sprite));
	ground.add(components::transform(vec2<float>(400, 400), 75 * 0.01745329251994329576923690768489));
	topdown::create_physics_component(ground, physics.b2world, b2_staticBody);
    ground.get<components::physics>().body->GetFixtureList()->SetFriction(0.0f);

	crosshair.add(components::render(0, &crosshair_sprite, components::render::GUI));
	crosshair.add(components::transform(vec2<float>(gl.get_window_rect().w, gl.get_window_rect().h) / 2));
	crosshair.add(components::crosshair(gl.get_screen_rect(), 1.f));
	crosshair.add(components::input());
	crosshair.get<components::input>().intents.add(intent_message::intent::AIM);
	
	world_camera.add(components::camera(gl.get_screen_rect(), gl.get_screen_rect(), 0, components::render::WORLD));
	world_camera.add(components::transform());

	gui_camera.add(components::camera(gl.get_screen_rect(), gl.get_screen_rect(), 0, components::render::GUI));
	gui_camera.add(components::transform());

	player_physical.remove<components::render>();
	while (!quit_flag) {
		my_world.run();

		/* flushing message queues */
		my_world.get_message_queue<message>().clear();
		my_world.get_message_queue<intent_message>().clear();
		my_world.get_message_queue<moved_message>().clear();
		my_world.get_message_queue<collision_message>().clear();
	}
	
	augmentations::deinit();
	return 0;
}
