#pragma once
#include "../../augmentations.h"

#include "../../window_framework/window.h"
#include "../../config/config.h"

#include "systems/physics_system.h"
#include "systems/movement_system.h"
#include "systems/animation_system.h"
#include "systems/camera_system.h"
#include "systems/render_system.h"
#include "systems/input_system.h"
#include "systems/crosshair_system.h"
#include "systems/lookat_system.h"
#include "systems/chase_system.h"

#include "messages/collision_message.h"
#include "messages/moved_message.h"
#include "messages/intent_message.h"
#include "messages/animate_message.h"

#include "game/body_helper.h"

#include "renderable.h"
#include "animation.h"

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

#define IMAGES 25
	texture_baker::image img[IMAGES];
	texture_baker::texture tex[IMAGES];
	texture_baker::atlas atl;

	img[0].from_file(L"C:\\vvv2\\pollock.jpg");
	img[1].from_file(L"C:\\vvv2\\enemy.png");
	img[2].from_file(L"C:\\vvv2\\crosshair.png");
	img[3].from_file(L"C:\\vvv2\\rifle.png");

	img[4].from_file(L"C:\\vvv2\\legs_1.png");
	img[5].from_file(L"C:\\vvv2\\legs_2.png");
	img[6].from_file(L"C:\\vvv2\\legs_3.png");
	img[7].from_file(L"C:\\vvv2\\legs_4.png");
	img[8].from_file(L"C:\\vvv2\\legs_5.png");
	img[9].from_file(L"C:\\vvv2\\legs_6.png");
	img[10].from_file(L"C:\\vvv2\\legs_7.png");
	img[11].from_file(L"C:\\vvv2\\legs_8.png");
	img[12].from_file(L"C:\\vvv2\\legs_9.png");
	img[13].from_file(L"C:\\vvv2\\legs_10.png");

	img[14].from_file(L"C:\\vvv2\\player_1.png");
	img[15].from_file(L"C:\\vvv2\\player_2.png");
	img[16].from_file(L"C:\\vvv2\\player_3.png");
	img[17].from_file(L"C:\\vvv2\\player_4.png");
	img[18].from_file(L"C:\\vvv2\\player_5.png");
	img[19].from_file(L"C:\\vvv2\\player_6.png");
	img[20].from_file(L"C:\\vvv2\\player_7.png");
	img[21].from_file(L"C:\\vvv2\\player_8.png");
	img[22].from_file(L"C:\\vvv2\\player_9.png");
	img[23].from_file(L"C:\\vvv2\\player_10.png");
	
	img[24].from_file(L"C:\\vvv2\\crate.jpg");

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
	atl.nearest();

	sprite small_sprite(tex + 24);
	sprite my_sprite(tex + 24);
	sprite bigger_sprite(tex + 24);
	sprite player_sprite(tex + 1);
	sprite crosshair_sprite(tex + 2);
	sprite rifle_sprite(tex + 3);
	sprite bg_sprite(tex + 0);

	sprite legs_sprites [] =   { tex + 4, tex + 5, tex + 6, tex + 7, tex + 8, tex + 9, tex + 10, tex + 11, tex + 12, tex + 13 };
	sprite player_sprites [] = { tex + 14, tex + 15, tex + 16, tex + 17, tex + 18, tex + 19, tex + 20, tex + 21, tex + 22, tex + 23 };
	for (auto& it : legs_sprites)
		it.size *= 2.f;

	animation legs_animation;
	animation player_animation;

	legs_animation.frames.push_back(animation::frame(nullptr, 2.f));
	legs_animation.frames.push_back(animation::frame(legs_sprites + 4, 2.f));
	legs_animation.frames.push_back(animation::frame(legs_sprites + 3, 2.f));
	legs_animation.frames.push_back(animation::frame(legs_sprites + 2, 2.f));
	legs_animation.frames.push_back(animation::frame(legs_sprites + 1, 2.f));
	legs_animation.frames.push_back(animation::frame(legs_sprites + 0, 2.f));
	legs_animation.frames.push_back(animation::frame(legs_sprites + 1, 2.f));
	legs_animation.frames.push_back(animation::frame(legs_sprites + 2, 2.f));
	legs_animation.frames.push_back(animation::frame(legs_sprites + 3, 2.f));
	legs_animation.frames.push_back(animation::frame(legs_sprites + 4, 2.f));
	legs_animation.frames.push_back(animation::frame(nullptr, 2.f));
	legs_animation.frames.push_back(animation::frame(legs_sprites + 5, 2.f));
	legs_animation.frames.push_back(animation::frame(legs_sprites + 6, 2.f));
	legs_animation.frames.push_back(animation::frame(legs_sprites + 7, 2.f));
	legs_animation.frames.push_back(animation::frame(legs_sprites + 8, 2.f));
	legs_animation.frames.push_back(animation::frame(legs_sprites + 9, 2.f));
	legs_animation.frames.push_back(animation::frame(legs_sprites + 8, 2.f));
	legs_animation.frames.push_back(animation::frame(legs_sprites + 7, 2.f));
	legs_animation.frames.push_back(animation::frame(legs_sprites + 6, 2.f));
	legs_animation.frames.push_back(animation::frame(legs_sprites + 5, 2.f));
	legs_animation.loop_mode = animation::loop_type::REPEAT;

	player_animation.frames.push_back(animation::frame(player_sprites + 0, 2.f));
	player_animation.frames.push_back(animation::frame(player_sprites + 1, 2.f));
	player_animation.frames.push_back(animation::frame(player_sprites + 2, 2.f));
	player_animation.frames.push_back(animation::frame(player_sprites + 3, 2.f));
	player_animation.frames.push_back(animation::frame(player_sprites + 4, 2.f));
	player_animation.frames.push_back(animation::frame(player_sprites + 3, 2.f));
	player_animation.frames.push_back(animation::frame(player_sprites + 2, 2.f));
	player_animation.frames.push_back(animation::frame(player_sprites + 1, 2.f));
	player_animation.frames.push_back(animation::frame(player_sprites + 0, 2.f));
	player_animation.frames.push_back(animation::frame(player_sprites + 5, 2.f));
	player_animation.frames.push_back(animation::frame(player_sprites + 6, 2.f));
	player_animation.frames.push_back(animation::frame(player_sprites + 7, 2.f));
	player_animation.frames.push_back(animation::frame(player_sprites + 8, 2.f));
	player_animation.frames.push_back(animation::frame(player_sprites + 9, 2.f));
	player_animation.frames.push_back(animation::frame(player_sprites + 8, 2.f));
	player_animation.frames.push_back(animation::frame(player_sprites + 7, 2.f));
	player_animation.frames.push_back(animation::frame(player_sprites + 6, 2.f));
	player_animation.frames.push_back(animation::frame(player_sprites + 5, 2.f));
	player_animation.loop_mode = animation::loop_type::REPEAT;

	bg_sprite.size *= 2;
	
	rifle_sprite.size /= 4;

	player_sprite.size = vec2<>(60.f, 70.f);

	bigger_sprite.size.y = 800;
	small_sprite.size = vec2<int>(100, 100);
	my_sprite.size = vec2<int>(200, 120);
	crosshair_sprite.size = vec2<int>(70, 70);

	bool quit_flag = false;

	world my_world;

	input_system input(gl, quit_flag);
	movement_system movement;
	animation_system animations;
	crosshair_system crosshairs;
	lookat_system lookat;
	physics_system physics;
	render_system render(gl);
	camera_system camera(render);
	chase_system chase;

	input_system::context main_context;
	main_context.raw_id_to_intent[window::event::mouse::raw_motion] = intent_message::intent::AIM;
	main_context.raw_id_to_intent[window::event::mouse::ldown] = intent_message::intent::SHOOT;
	main_context.raw_id_to_intent[window::event::keys::W] = intent_message::intent::MOVE_FORWARD;
	main_context.raw_id_to_intent[window::event::keys::S] = intent_message::intent::MOVE_BACKWARD;
	main_context.raw_id_to_intent[window::event::keys::A] = intent_message::intent::MOVE_LEFT;
	main_context.raw_id_to_intent[window::event::keys::D] = intent_message::intent::MOVE_RIGHT;
	input.active_contexts.push_back(&main_context);

	physics.b2world.SetGravity(vec2<>(0.f, 0.f));

	my_world.add_system(&input);
	my_world.add_system(&movement);
	my_world.add_system(&animations);
	my_world.add_system(&physics);
	my_world.add_system(&crosshairs);
	my_world.add_system(&lookat);
	my_world.add_system(&chase);
	my_world.add_system(&render);
	my_world.add_system(&camera);

	entity& world_camera = my_world.create_entity();
	entity& bg = my_world.create_entity();
	entity& rect = my_world.create_entity();
	entity& rect1 = my_world.create_entity();
	entity& rect2 = my_world.create_entity();
	entity& ground = my_world.create_entity();
	entity& crosshair = my_world.create_entity();

	bg.add(components::render(3, &bg_sprite));
	bg.add(components::transform());

	components::animate player_animate;
	player_animate.available_animations.add(components::animate::response(animate_message::animation::MOVE, &player_animation));
	player_animate.current_animation = &player_animation;

	components::animate legs_animate;
	legs_animate.available_animations.add(components::animate::response(animate_message::animation::MOVE, &legs_animation));
	legs_animate.current_animation = &legs_animation;

	auto spawn_npc = [&](){
		entity& physical = my_world.create_entity();
		entity& legs = my_world.create_entity();

		components::movement player_movement(vec2<>(15000.f, 15000.f), 15000.f);
		player_movement.animation_receivers.push_back(components::movement::subscribtion(&physical, false));
		player_movement.animation_receivers.push_back(components::movement::subscribtion(&legs, true));

		physical.add(components::render(0, &player_sprite));
		physical.add(components::transform(vec2<>(0.f, 0.f)));
		physical.add(player_animate);
		physical.add(player_movement);

		topdown::create_physics_component(physical, physics.b2world, b2_dynamicBody);
		physical.get<components::physics>().body->SetLinearDamping(13.0f);
		physical.get<components::physics>().body->SetAngularDamping(5.0f);
		physical.get<components::physics>().body->SetFixedRotation(true);

		legs.add(legs_animate);
		legs.add(components::render(1, nullptr));
		legs.add(components::chase(&physical));
		legs.add(components::transform());
		legs.add(components::lookat(&physical, components::lookat::chase_type::VELOCITY));
		
		return std::pair<entity&, entity&>(physical, legs);
	};
	
	auto player = spawn_npc();

	components::input player_input;
	player_input.intents.add(intent_message::intent::MOVE_FORWARD);
	player_input.intents.add(intent_message::intent::MOVE_BACKWARD);
	player_input.intents.add(intent_message::intent::MOVE_LEFT);
	player_input.intents.add(intent_message::intent::MOVE_RIGHT);
	
	player.first.add(player_input);
	player.first.add(components::lookat(&crosshair));

	player.first.get<components::physics>().body->SetTransform(vec2<>(-500.f*PIXELS_TO_METERSf, 0.f), 0.f);
	
	for (int i = 0; i < 100; ++i) {
		auto npc = spawn_npc();
		npc.first.get<components::physics>().body->SetLinearDamping(4.0f);
		npc.first.get<components::physics>().body->SetFixedRotation(false);
		//npc.first.get<components::physics>().body->GetFixtureList()->SetDensity(0.1f);
		npc.first.get<components::physics>().body->ResetMassData();
	    //if(i&2)npc.first.add(player_input);
	}

	rect.add(components::render(0, &my_sprite));
	rect.add(components::transform(vec2<>(500.f, -50.f)));
	topdown::create_physics_component(rect, physics.b2world);
	rect1.add(components::render(0, &my_sprite));
	rect1.add(components::transform(vec2<>(470.f, 50.f)));
	topdown::create_physics_component(rect1, physics.b2world);
	rect2.add(components::render(0, &small_sprite));
	rect2.add(components::transform(vec2<>(400.f, 0.f), 45.f * 0.01745329251994329576923690768489f));
	topdown::create_physics_component(rect2, physics.b2world);

	ground.add(components::render(0, &bigger_sprite));
	ground.add(components::transform(vec2<>(400.f, 400.f), 75.f * 0.01745329251994329576923690768489f));
	topdown::create_physics_component(ground, physics.b2world, b2_dynamicBody);
    ground.get<components::physics>().body->GetFixtureList()->SetFriction(0.0f);

	rect.get<components::physics>().body->SetLinearDamping(5.f);
	rect1.get<components::physics>().body->SetLinearDamping(5.f);
	rect2.get<components::physics>().body->SetLinearDamping(5.f);
	ground.get<components::physics>().body->SetLinearDamping(5.f);
	rect.get<components::physics>().body->SetAngularDamping(5.f);
	rect1.get<components::physics>().body->SetAngularDamping(5.f);
	rect2.get<components::physics>().body->SetAngularDamping(5.f);
	ground.get<components::physics>().body->SetAngularDamping(5.f);

	crosshair.add(components::render(0, &crosshair_sprite));
	crosshair.add(components::transform(vec2<>(vec2<int>(gl.get_window_rect().w, gl.get_window_rect().h)) / 2.f));
	crosshair.add(components::crosshair(2.5f));
	crosshair.add(components::chase(&player.first, true));
	crosshair.add(components::input());
	crosshair.get<components::input>().intents.add(intent_message::intent::AIM);
	
	world_camera.add(components::transform());
	world_camera.add(components::chase(&player.first, false, vec2<>(vec2<int>(gl.get_screen_rect()))*-0.5f));
	world_camera.add(components::camera(gl.get_screen_rect(), gl.get_screen_rect(), 0, components::render::WORLD, 0.5, 20.0));
	world_camera.get<components::camera>().crosshair = &crosshair;
	world_camera.get<components::camera>().player = &player.first;
	world_camera.get<components::camera>().orbit_mode = components::camera::LOOK;
	world_camera.get<components::camera>().max_look_expand = vec2<>(vec2<int>(gl.get_screen_rect())) * 0.5f;
	world_camera.get<components::camera>().enable_smoothing = true;

	while (!quit_flag) {
		my_world.run();

		/* flushing message queues */
		my_world.get_message_queue<message>().clear();
		my_world.get_message_queue<animate_message>().clear();
		my_world.get_message_queue<intent_message>().clear();
		my_world.get_message_queue<moved_message>().clear();
		my_world.get_message_queue<collision_message>().clear();
	}
	
	augmentations::deinit();
	return 0;
}
