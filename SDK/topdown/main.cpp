#pragma once
#include <random>

#include "../../augmentations.h"

#include "../../window_framework/window.h"
#include "../../config/config.h"

#include "entity_system/world.h"

#include "systems/physics_system.h"
#include "systems/movement_system.h"
#include "systems/animation_system.h"
#include "systems/camera_system.h"
#include "systems/render_system.h"
#include "systems/input_system.h"
#include "systems/gun_system.h"
#include "systems/crosshair_system.h"
#include "systems/lookat_system.h"
#include "systems/chase_system.h"
#include "systems/damage_system.h"
#include "systems/health_system.h"
#include "systems/destroy_system.h"
#include "systems/particle_group_system.h"
#include "systems/particle_emitter_system.h"
#include "systems/script_system.h"

//#undef BOOST_DISABLE_THREADS
//#define BOOST_DISABLE_THREADS

#include "messages/destroy_message.h"
#include "messages/collision_message.h"
#include "messages/moved_message.h"
#include "messages/intent_message.h"
#include "messages/animate_message.h"
#include "messages/particle_burst_message.h"
#include "messages/damage_message.h"

#include "game/body_helper.h"
#include "game/sprite_helper.h"

#include "renderable.h"
#include "animation.h"
#include "script.h"

using namespace augmentations;
using namespace entity_system;
using namespace messages;

#define kajdljsdklasjdd
#ifdef kajdljsdklasjdd
int main() {
	augmentations::init();

	config::input_file cfg("window_config.txt");

	window::glwindow gl;
	gl.create(cfg, rects::wh(100, 100));
	gl.set_show(gl.SHOW);
	window::cursor(false);

	bool quit_flag = false;

	world my_world;

	input_system input(gl, quit_flag);
	movement_system movement;
	animation_system animations;
	crosshair_system crosshairs;
	lookat_system lookat;
	physics_system physics;
	gun_system guns;
	particle_group_system particles;
	particle_emitter_system emitters;
	render_system render(gl);
	camera_system camera(render);
	chase_system chase;
	damage_system damage;
	health_system health;
	destroy_system destroy;
	script_system scripts;

	topdown::current_b2world = &physics.b2world;

	my_world.add_system(&input);
	my_world.add_system(&movement);
	my_world.add_system(&physics);
	my_world.add_system(&crosshairs);
	my_world.add_system(&lookat);
	my_world.add_system(&guns);
	my_world.add_system(&damage);
	my_world.add_system(&health);
	my_world.add_system(&emitters);
	my_world.add_system(&particles);
	my_world.add_system(&chase);
	my_world.add_system(&animations);
	my_world.add_system(&render);
	my_world.add_system(&destroy);
	my_world.add_system(&camera);

	scripts.global("world", my_world);
	scripts.global("window", gl);

	script::lua_state = scripts.lua_state;
	script::script_reloader.report_errors = &std::cout;
	script::script_reloader.add_directory(L"scripts", true);
	script init_script;
	init_script.associate_filename("scripts\\init.lua");
	init_script.call();

	while (!quit_flag) {
		my_world.run();

		/* flushing message queues */
		my_world.get_message_queue<message>().clear();
		my_world.get_message_queue<moved_message>().clear();
		my_world.get_message_queue<intent_message>().clear();
		my_world.get_message_queue<damage_message>().clear();
		my_world.get_message_queue<destroy_message>().clear();
		my_world.get_message_queue<animate_message>().clear();
		my_world.get_message_queue<collision_message>().clear();
		my_world.get_message_queue<particle_burst_message>().clear();
		
		auto scripts_reloaded = script::script_reloader.get_modified_script_files();

		for (auto& script_to_reload : scripts_reloaded) {
			if (script_to_reload->reload_scene_when_modified) {
				my_world.delete_all_entities();
				init_script.call();
				break;
			}
		}
	}

	augmentations::deinit();
	return 0;
}
#else
int main() {
	augmentations::init();

	config::input_file cfg("window_config.txt");
	 
	window::glwindow gl;
	gl.create(cfg, rects::wh(100, 100));
	gl.set_show(gl.SHOW);
	window::cursor(false);

#define IMAGES 52
	texture_baker::image img[IMAGES];
	texture_baker::texture tex[IMAGES];
	texture_baker::atlas atl;

	img[0].from_file(L"Release\\resources\\pollock.jpg");
	img[1].from_file(L"Release\\resources\\enemy.png");
	img[2].from_file(L"Release\\resources\\crosshair.png");
	img[3].from_file(L"Release\\resources\\rifle.png");

	img[4].from_file(L"Release\\resources\\legs_1.png");
	img[5].from_file(L"Release\\resources\\legs_2.png");
	img[6].from_file(L"Release\\resources\\legs_3.png");
	img[7].from_file(L"Release\\resources\\legs_4.png");
	img[8].from_file(L"Release\\resources\\legs_5.png");
	img[9].from_file(L"Release\\resources\\legs_6.png");
	img[10].from_file(L"Release\\resources\\legs_7.png");
	img[11].from_file(L"Release\\resources\\legs_8.png");
	img[12].from_file(L"Release\\resources\\legs_9.png");
	img[13].from_file(L"Release\\resources\\legs_10.png");

	img[14].from_file(L"Release\\resources\\player_1.png");
	img[15].from_file(L"Release\\resources\\player_2.png");
	img[16].from_file(L"Release\\resources\\player_3.png");
	img[17].from_file(L"Release\\resources\\player_4.png");
	img[18].from_file(L"Release\\resources\\player_5.png");
	img[19].from_file(L"Release\\resources\\player_6.png");
	img[20].from_file(L"Release\\resources\\player_7.png");
	img[21].from_file(L"Release\\resources\\player_8.png");
	img[22].from_file(L"Release\\resources\\player_9.png");
	img[23].from_file(L"Release\\resources\\player_10.png");
	
	img[24].from_file(L"Release\\resources\\crate.jpg");

	img[25].from_file(L"Release\\resources\\player_shotgun_1.png");
	img[26].from_file(L"Release\\resources\\player_shotgun_2.png");
	img[27].from_file(L"Release\\resources\\player_shotgun_3.png");
	img[28].from_file(L"Release\\resources\\player_shotgun_4.png");
	img[29].from_file(L"Release\\resources\\player_shotgun_5.png");

	img[30].from_file(L"Release\\resources\\bullet.png");

	img[31].from_file(L"Release\\resources\\player_shotgun_shot_1.png");
	img[32].from_file(L"Release\\resources\\player_shotgun_shot_2.png");
	img[33].from_file(L"Release\\resources\\player_shotgun_shot_3.png");
	img[34].from_file(L"Release\\resources\\player_shotgun_shot_4.png");
	img[35].from_file(L"Release\\resources\\player_shotgun_shot_5.png");

	img[36].from_file(L"Release\\resources\\piece_1.png");
	img[37].from_file(L"Release\\resources\\piece_2.png");
	img[38].from_file(L"Release\\resources\\piece_3.png");
	img[39].from_file(L"Release\\resources\\piece_4.png");
	img[40].from_file(L"Release\\resources\\piece_5.png");
	img[41].from_file(L"Release\\resources\\piece_6.png");
	img[42].from_file(L"Release\\resources\\piece_7.png");
	img[43].from_file(L"Release\\resources\\piece_8.png");
	
	img[44].from_file(L"Release\\resources\\smoke_particle.png");

	img[45].from_file(L"Release\\resources\\blood_1.png");
	img[46].from_file(L"Release\\resources\\blood_2.png");
	img[47].from_file(L"Release\\resources\\blood_3.png");
	img[48].from_file(L"Release\\resources\\blood_4.png");
	img[49].from_file(L"Release\\resources\\blood_5.png");

	img[50].from_file(L"Release\\resources\\dead_front.png");
	img[51].from_file(L"Release\\resources\\dead_back.png");

	for (int i = 0; i < IMAGES; ++i) {
		tex[i].set(img + i);
		atl.textures.push_back(tex + i);
	}

	atl.default_build();

	sprite small_sprite(tex + 24);
	sprite my_sprite(tex + 24);
	sprite bigger_sprite(tex + 24);
	sprite player_sprite(tex + 1);
	sprite crosshair_sprite(tex + 2);
	sprite rifle_sprite(tex + 3);
	sprite bg_sprite(tex + 0);
	sprite bullet_sprite(tex + 30);

	sprite legs_sprites [] =   { tex + 4, tex + 5, tex + 6, tex + 7, tex + 8, tex + 9, tex + 10, tex + 11, tex + 12, tex + 13 };
	sprite player_sprites [] = { tex + 14, tex + 15, tex + 16, tex + 17, tex + 18, tex + 19, tex + 20, tex + 21, tex + 22, tex + 23 };
	sprite player_shotgun_sprites [] = { tex + 25, tex + 26, tex + 27, tex + 28, tex + 29 };
	sprite player_shotgun_shot_sprites [] = { tex + 31, tex + 32, tex + 33, tex + 34, tex + 35 };
	for (auto& it : legs_sprites)
		it.size *= 2.f;

	sprite wood_pieces [] = { tex + 36, tex + 37, tex + 38, tex + 39, tex + 40, tex + 41, tex + 42, tex + 43 };
	sprite smoke_sprite(tex + 44);

	sprite blood_sprites [] = { tex + 45, tex + 46, tex + 47, tex + 48, tex + 49 };

	sprite dead_front_sprite(tex + 50);
	sprite dead_back_sprite(tex + 51);

	animation legs_animation;
	animation player_animation;
	animation player_shotgun_animation;
	animation player_shotgun_shot_animation;

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

	player_shotgun_animation.frames.push_back(animation::frame(player_shotgun_sprites + 0, 5.f));
	player_shotgun_animation.frames.push_back(animation::frame(player_shotgun_sprites + 1, 5.f));
	player_shotgun_animation.frames.push_back(animation::frame(player_shotgun_sprites + 2, 5.f));
	player_shotgun_animation.frames.push_back(animation::frame(player_shotgun_sprites + 3, 5.f));
	player_shotgun_animation.frames.push_back(animation::frame(player_shotgun_sprites + 4, 5.f));
	player_shotgun_animation.loop_mode = animation::loop_type::INVERSE;

	player_shotgun_shot_animation.frames.push_back(animation::frame(player_shotgun_shot_sprites + 0, 15.f));
	player_shotgun_shot_animation.frames.push_back(animation::frame(player_shotgun_shot_sprites + 1, 15.f));
	player_shotgun_shot_animation.frames.push_back(animation::frame(player_shotgun_shot_sprites + 2, 15.f));
	player_shotgun_shot_animation.frames.push_back(animation::frame(player_shotgun_shot_sprites + 3, 15.f));
	player_shotgun_shot_animation.frames.push_back(animation::frame(player_shotgun_shot_sprites + 4, 15.f));
	player_shotgun_shot_animation.frames.push_back(animation::frame(player_shotgun_shot_sprites + 3, 15.f));
	player_shotgun_shot_animation.frames.push_back(animation::frame(player_shotgun_shot_sprites + 2, 15.f));
	player_shotgun_shot_animation.frames.push_back(animation::frame(player_shotgun_shot_sprites + 1, 15.f));
	player_shotgun_shot_animation.frames.push_back(animation::frame(player_shotgun_shot_sprites + 0, 15.f));
	player_shotgun_shot_animation.loop_mode = animation::loop_type::NONE;

	bullet_sprite.size /= 1.25;
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
	gun_system guns(physics);
	particle_group_system particles;
	particle_emitter_system emitters;
	render_system render(gl);
	camera_system camera(render);
	chase_system chase;
	damage_system damage;
	health_system health(physics);
	destroy_system destroy;
	script_system scripts;

	//luabind::globals(scripts.lua_state)["world"] = &my_world;

	//script my_script;
	//my_script.associate_filename("script.txt");
	//auto luaerrors = my_script.compile(scripts.lua_state);
	//auto luaerrors_call = my_script.call(scripts.lua_state);

	input_system::context main_context;
	main_context.raw_id_to_intent[window::event::mouse::raw_motion] = intent_message::intent::AIM;
	main_context.raw_id_to_intent[window::event::mouse::ldown] = intent_message::intent::SHOOT;
	main_context.raw_id_to_intent[window::event::mouse::ldoubleclick] = intent_message::intent::SHOOT;
	main_context.raw_id_to_intent[window::event::mouse::ltripleclick] = intent_message::intent::SHOOT;
	main_context.raw_id_to_intent[window::event::mouse::rdown] = intent_message::intent::SWITCH_LOOK;
	main_context.raw_id_to_intent[window::event::mouse::rdoubleclick] = intent_message::intent::SWITCH_LOOK;
	main_context.raw_id_to_intent[window::event::keys::W] = intent_message::intent::MOVE_FORWARD;
	main_context.raw_id_to_intent[window::event::keys::S] = intent_message::intent::MOVE_BACKWARD;
	main_context.raw_id_to_intent[window::event::keys::A] = intent_message::intent::MOVE_LEFT;
	main_context.raw_id_to_intent[window::event::keys::D] = intent_message::intent::MOVE_RIGHT;
	input.active_contexts.push_back(&main_context);

	physics.b2world.SetGravity(vec2<>(0.f, 0.f));

	my_world.add_system(&input);
	my_world.add_system(&movement);
	my_world.add_system(&physics);
	my_world.add_system(&crosshairs);
	my_world.add_system(&lookat);
	my_world.add_system(&guns);
	my_world.add_system(&damage);
	my_world.add_system(&health);
	my_world.add_system(&emitters);
	my_world.add_system(&particles);
	my_world.add_system(&chase);
	my_world.add_system(&animations);
	my_world.add_system(&render);
	my_world.add_system(&destroy);
	my_world.add_system(&camera);

	enum LAYERS {
		GUI_OBJECTS,
		EFFECTS,
		OBJECTS,
		PLAYERS,
		BULLETS,
		LEGS,
		ON_GROUND,
		GROUND
	};
	
	b2Filter filter_objects, filter_characters, filter_bullets, filter_corpses;
	
	{
		enum {
			CHARACTERS = 1 << 0,
			OBJECTS = 1 << 1,
			BULLETS = 1 << 2,
			CORPSES = 1 << 3
		};

		filter_characters.categoryBits = CHARACTERS;
		filter_characters.maskBits = OBJECTS | BULLETS | CHARACTERS;

		filter_objects.categoryBits = OBJECTS;
		filter_objects.maskBits = OBJECTS | BULLETS | CHARACTERS | CORPSES;

		filter_bullets.categoryBits = BULLETS;
		filter_bullets.maskBits = OBJECTS | CHARACTERS;

		filter_corpses.categoryBits = CORPSES;
		filter_corpses.maskBits = OBJECTS;
	}

	//if (luaL_dostring(L,
	//	" local my_entity = world:create_entity() \n"
	//	" local my_entity2 = world:create_entity() \n"
	//	"world:delete_entity(world:create_entity(), my_entity2)\n"
	//	)) {
	//		std::ofstream logfilelua("luakurwa.txt");
	//		logfilelua << lua_tostring(L, -1);
	//}

	entity& world_camera = my_world.create_entity();
	entity& bg = my_world.create_entity();
	entity& rect = my_world.create_entity();
	entity& rect1 = my_world.create_entity();
	entity& rect2 = my_world.create_entity();
	entity& ground = my_world.create_entity();
	entity& crosshair = my_world.create_entity();

	components::particle_emitter::particle templates[9], barrel_explosion_template, barrel_smoke_template, blood_shower_templates[5];

	barrel_explosion_template.angular_damping = 0.f;
	barrel_explosion_template.linear_damping = 55000.f;
	barrel_explosion_template.should_disappear = true;
	barrel_explosion_template.face = bullet_sprite;
	barrel_explosion_template.face.size *= 4.f;
	barrel_explosion_template.lifetime_ms = 0.f;

	barrel_smoke_template.angular_damping = 0.f;
	barrel_smoke_template.linear_damping = 100.f;
	barrel_smoke_template.should_disappear = true;
	barrel_smoke_template.face = smoke_sprite;
	barrel_smoke_template.face.size /= 5.f;
	barrel_smoke_template.face.color.a = 3;
	barrel_smoke_template.lifetime_ms = 0.f;
	barrel_smoke_template.acc.y = -300.f;

	for (int i = 0; i < 5; ++i) {
		blood_shower_templates[i].angular_damping = 1000.f;
		blood_shower_templates[i].linear_damping = 80500.f;
		blood_shower_templates[i].should_disappear = false;
		blood_shower_templates[i].face = blood_sprites[i];
		blood_shower_templates[i].face.color.a = 255;
		//blood_shower_templates[i].face.size *= vec2<>(1.2f, 0.9f);
	}

	/* big pieces */
	for (int i = 0; i < 4; ++i) {
		templates[i].angular_damping = 1200.f * 0.01745329251994329576923690768489f;
		templates[i].linear_damping = 10000.f;
		templates[i].should_disappear = false;
		templates[i].face = wood_pieces[i];
	}
	/* small pieces */
	for (int i = 4; i < 8; ++i) {
		templates[i].angular_damping = 1200.f * 0.01745329251994329576923690768489f;
		templates[i].linear_damping = 7000.f;
		templates[i].should_disappear = false;
		templates[i].face = wood_pieces[i];
	}

	templates[8].angular_damping = 1200.f * 0.01745329251994329576923690768489f;
	templates[8].linear_damping = 7000.f;
	templates[8].should_disappear = true;
	templates[8].face = wood_pieces[7];
	templates[8].max_lifetime_ms = 200.f;
	templates[8].lifetime_ms = 0.f;

	components::particle_emitter::subscribtion wood_effects, player_effects;
	auto& wood_emissions = wood_effects[messages::particle_burst_message::burst_type::BULLET_IMPACT];
	auto& shot_emissions = player_effects[messages::particle_burst_message::burst_type::WEAPON_SHOT];
	auto& blood_emissions = player_effects[messages::particle_burst_message::burst_type::BULLET_IMPACT];
	
	components::particle_emitter::emission wood_parts_big, wood_parts_small, barrel_explosion, barrel_smoke[2], blood_shower, blood_pool, blood_droplets, wood_dust;

	wood_parts_big.spread_radians = 45.f * 0.01745329251994329576923690768489f;
	wood_parts_big.particles_per_burst.first = 1;
	wood_parts_big.particles_per_burst.second = 1;
	wood_parts_big.type = components::particle_emitter::emission::type::BURST;
	wood_parts_big.velocity.first = 2000.f;
	wood_parts_big.velocity.second = 3000.f;
	wood_parts_big.angular_velocity.first = 100.f * 0.01745329251994329576923690768489f;
	wood_parts_big.angular_velocity.second = 220.f * 0.01745329251994329576923690768489f;
	wood_parts_big.particle_templates = std::vector<components::particle_emitter::particle>(templates + 0, templates + 4);
	wood_parts_big.size_multiplier.first = 0.1f;
	wood_parts_big.size_multiplier.second = 1.0f;
	wood_parts_big.initial_rotation_variation = 180.f * 0.01745329251994329576923690768489f;
	wood_parts_big.angular_offset = 0.f;
	wood_parts_big.particle_group_layer = ON_GROUND;

	wood_parts_small.spread_radians = 45.f * 0.01745329251994329576923690768489f;
	wood_parts_small.particles_per_burst.first = 2;
	wood_parts_small.particles_per_burst.second = 4;
	wood_parts_small.type = components::particle_emitter::emission::type::BURST;
	wood_parts_small.velocity.first = 200.f;
	wood_parts_small.velocity.second = 3000.f;
	wood_parts_small.particle_templates = std::vector<components::particle_emitter::particle>(templates + 4, templates + 8);
	wood_parts_small.size_multiplier.first = 0.1f;
	wood_parts_small.size_multiplier.second = 1.0f;
	wood_parts_small.angular_velocity.first = 100.f * 0.01745329251994329576923690768489f;
	wood_parts_small.angular_velocity.second = 220.f * 0.01745329251994329576923690768489f;
	wood_parts_small.initial_rotation_variation = 180.f * 0.01745329251994329576923690768489f;
	wood_parts_small.angular_offset = 0.f;
	wood_parts_small.particle_group_layer = ON_GROUND;

	wood_dust.spread_radians = 45.f * 0.01745329251994329576923690768489f;
	wood_dust.particles_per_burst.first = 5;
	wood_dust.particles_per_burst.second = 55;
	wood_dust.type = components::particle_emitter::emission::type::BURST;
	wood_dust.velocity.first = 200.f;
	wood_dust.velocity.second = 4000.f;
	wood_dust.particle_templates.push_back(templates[8]);
	wood_dust.size_multiplier.first = 0.1f;
	wood_dust.size_multiplier.second = 0.5f;
	wood_dust.angular_velocity.first = 540.f * 0.01745329251994329576923690768489f;
	wood_dust.angular_velocity.second = 1220.f * 0.01745329251994329576923690768489f;
	wood_dust.particle_lifetime_ms.first = 0.f;
	wood_dust.particle_lifetime_ms.second = 350.f;
	wood_dust.initial_rotation_variation = 180.f * 0.01745329251994329576923690768489f;
	wood_dust.angular_offset = 0.f;
	wood_dust.particle_group_layer = ON_GROUND;

	barrel_explosion.spread_radians = 15.5f * 0.01745329251994329576923690768489f;
	barrel_explosion.particles_per_burst.first = 10;
	barrel_explosion.particles_per_burst.second = 50;
	barrel_explosion.type = components::particle_emitter::emission::type::BURST;
	barrel_explosion.velocity.first = 1000.f;
	barrel_explosion.velocity.second = 10000.f;
	barrel_explosion.angular_velocity.first = 0.f;
	barrel_explosion.angular_velocity.second = 0.f;
	barrel_explosion.particle_lifetime_ms.first = 20.f;
	barrel_explosion.particle_lifetime_ms.second = 40.f;
	barrel_explosion.particle_templates.push_back(barrel_explosion_template);
	barrel_explosion.size_multiplier.first = 0.8f;
	barrel_explosion.size_multiplier.second = 1.2f;
	barrel_explosion.initial_rotation_variation = 0.f;
	barrel_explosion.particle_group_layer = EFFECTS;
	barrel_explosion.angular_offset = 0.f;

	barrel_smoke[0].spread_radians = 25.5f * 0.01745329251994329576923690768489f;
	barrel_smoke[0].particles_per_sec.first = 50.f;
	barrel_smoke[0].particles_per_sec.second = 90.f;
	barrel_smoke[0].stream_duration_ms.first = 100.f;
	barrel_smoke[0].stream_duration_ms.second = 600.f;
	barrel_smoke[0].type = components::particle_emitter::emission::type::STREAM;
	barrel_smoke[0].velocity.first = 50.f;
	barrel_smoke[0].velocity.second = 100.f;
	barrel_smoke[0].angular_velocity.first = 0.f;
	barrel_smoke[0].angular_velocity.second = 0.f * 0.01745329251994329576923690768489f;
	barrel_smoke[0].particle_lifetime_ms.first = 10.f;
	barrel_smoke[0].particle_lifetime_ms.second = 3000.f;
	barrel_smoke[0].particle_templates.push_back(barrel_smoke_template);
	barrel_smoke[0].size_multiplier.first = 0.7f;
	barrel_smoke[0].size_multiplier.second = 5.5f;
	barrel_smoke[0].initial_rotation_variation = 180.f * 0.01745329251994329576923690768489f;
	barrel_smoke[0].particle_group_layer = EFFECTS;
	barrel_smoke[0].angular_offset = 0.f;
	barrel_smoke[0].randomize_acceleration = false;

	barrel_smoke[1].spread_radians = 14.5f * 0.01745329251994329576923690768489f;
	barrel_smoke[1].particles_per_sec.first = 50.f;
	barrel_smoke[1].particles_per_sec.second = 90.f;
	barrel_smoke[1].stream_duration_ms.first = 100.f;
	barrel_smoke[1].stream_duration_ms.second = 6000.f;
	barrel_smoke[1].type = components::particle_emitter::emission::type::STREAM;
	barrel_smoke[1].velocity.first = 100.f;
	barrel_smoke[1].velocity.second = 200.f;
	barrel_smoke[1].particle_lifetime_ms.first = 10.f;
	barrel_smoke[1].particle_lifetime_ms.second = 3000.f;
	barrel_smoke[1].angular_velocity.first = 0.f;
	barrel_smoke[1].angular_velocity.second = 0.f * 0.01745329251994329576923690768489f;
	barrel_smoke[1].particle_templates.push_back(barrel_smoke_template);
	barrel_smoke[1].size_multiplier.first = 1.0f;
	barrel_smoke[1].size_multiplier.second = 1.3f;
	barrel_smoke[1].initial_rotation_variation = 180.f * 0.01745329251994329576923690768489f;
	barrel_smoke[1].particle_group_layer = EFFECTS;
	barrel_smoke[1].angular_offset = 0.f;
	barrel_smoke[1].randomize_acceleration = false;

	blood_shower.spread_radians = 120.5f * 0.01745329251994329576923690768489f;
	//blood_shower.particles_per_sec.first = 10.f;
	//blood_shower.particles_per_sec.second = 200.f;
	blood_shower.particles_per_burst.first = 10;
	blood_shower.particles_per_burst.second = 20;
	//blood_shower.stream_duration_ms.first = 100.f;
	//blood_shower.stream_duration_ms.second = 1000.f;
	blood_shower.type = components::particle_emitter::emission::type::BURST;
	blood_shower.velocity.first = 1.f;
	blood_shower.velocity.second = 4000.f;
	blood_shower.angular_velocity.first = 0.f;
	blood_shower.angular_velocity.second = 1500.f * 0.01745329251994329576923690768489f;
	blood_shower.particle_templates = std::vector<components::particle_emitter::particle>(blood_shower_templates, blood_shower_templates + 5);
	blood_shower.size_multiplier.first = 0.2f;
	blood_shower.size_multiplier.second = 0.35f;
	blood_shower.initial_rotation_variation = 180.f * 0.01745329251994329576923690768489f;
	blood_shower.particle_group_layer = ON_GROUND;
	blood_shower.angular_offset = 0.f;

	blood_droplets.spread_radians = 90.5f * 0.01745329251994329576923690768489f;
	blood_droplets.particles_per_burst.first = 10;
	blood_droplets.particles_per_burst.second = 500;
	blood_droplets.type = components::particle_emitter::emission::type::BURST;
	blood_droplets.velocity.first = 1.f;
	blood_droplets.velocity.second = 5000.f;
	blood_droplets.angular_velocity.first = 0.f;
	blood_droplets.angular_velocity.second = 1500.f * 0.01745329251994329576923690768489f;
	blood_droplets.particle_templates = std::vector<components::particle_emitter::particle>(blood_shower_templates, blood_shower_templates + 5);

	for (auto& it : blood_droplets.particle_templates) {
		it.should_disappear = true;
	}

	blood_droplets.size_multiplier.first = 0.2f;
	blood_droplets.size_multiplier.second = 0.35f;
	blood_droplets.initial_rotation_variation = 180.f * 0.01745329251994329576923690768489f;
	blood_droplets.particle_group_layer = ON_GROUND;
	blood_droplets.angular_offset = 0.f;
	blood_droplets.particle_lifetime_ms.first = 200.f;
	blood_droplets.particle_lifetime_ms.second = 500.f;

	blood_pool.spread_radians = 180.5f * 0.01745329251994329576923690768489f;
	blood_pool.particles_per_sec.first = 20.f;
	blood_pool.particles_per_sec.second = 100.f;
	blood_pool.stream_duration_ms.first = 300.f;
	blood_pool.stream_duration_ms.second = 1000.f;
	blood_pool.type = components::particle_emitter::emission::type::STREAM;
	blood_pool.velocity.first = 1.f;
	blood_pool.velocity.second = 6.f;
	blood_pool.angular_velocity.first = 0.f;
	blood_pool.angular_velocity.second = 10.f * 0.01745329251994329576923690768489f;
	blood_pool.particle_templates = std::vector<components::particle_emitter::particle>(blood_shower_templates, blood_shower_templates + 5);
	
	for (auto& it : blood_pool.particle_templates) {
		it.linear_damping = 2.f;
	}
	
	blood_pool.size_multiplier.first = 0.3f;
	blood_pool.size_multiplier.second = 1.0f;
	blood_pool.initial_rotation_variation = 180.f * 0.01745329251994329576923690768489f;
	blood_pool.particle_group_layer = ON_GROUND;
	blood_pool.angular_offset = 0.f;

	wood_emissions.push_back(wood_parts_big);
	wood_emissions.push_back(wood_parts_small);
	wood_emissions.push_back(barrel_smoke[0]);
	wood_emissions.push_back(barrel_smoke[1]);
	wood_emissions.push_back(wood_dust);
	shot_emissions.push_back(barrel_explosion);
	shot_emissions.push_back(barrel_smoke[0]);
	shot_emissions.push_back(barrel_smoke[1]);
	blood_emissions.push_back(blood_shower);
	blood_emissions.push_back(blood_pool);
	blood_emissions.push_back(blood_droplets);

	bg.add(components::render(GROUND, &bg_sprite));
	bg.add(components::transform());

	/* creating flyweights */
	components::animate::subscribtion 
		player_shotgun_animate_subscribtion, 
		player_animate_subscribtion, 
		legs_animate_subscribtion;

	player_shotgun_animate_subscribtion[animate_message::animation::MOVE] = &player_shotgun_animation;
	player_shotgun_animate_subscribtion[animate_message::animation::SHOT] = &player_shotgun_shot_animation;
	player_animate_subscribtion[animate_message::animation::MOVE] = &player_animation;
	legs_animate_subscribtion[animate_message::animation::MOVE] = &legs_animation;

	/* creating instances of animation */
	components::animate 
		player_shotgun_animate(&player_shotgun_animate_subscribtion),
		player_animate(&player_animate_subscribtion), 
		legs_animate(&legs_animate_subscribtion);

	components::gun::gun_info double_barrel, assault_rifle;
	bullet_sprite.size.y /= 3.f;
	bullet_sprite.size.x *= 2.f;

	assault_rifle.bullets_once = 1;
	assault_rifle.bullet_max_damage = 100.f;
	assault_rifle.bullet_distance_offset = 120.f;
	assault_rifle.bullet_min_damage = 80.f;
	assault_rifle.bullet_speed = 10000.f;
	assault_rifle.bullet_sprite = &bullet_sprite;
	assault_rifle.is_automatic = true;
	assault_rifle.max_rounds = 30;
	assault_rifle.shooting_interval_ms = 70.f;
	assault_rifle.spread_radians = 1.f * 0.01745329251994329576923690768489f;
	assault_rifle.velocity_variation = 1500.f;
	assault_rifle.shake_radius = 9.5f;
	assault_rifle.shake_spread_radians = 45.f * 0.01745329251994329576923690768489f;
	assault_rifle.bullet_collision_filter = filter_bullets;
	assault_rifle.bullet_layer = BULLETS;
	assault_rifle.max_bullet_distance = 5000.f;

	double_barrel.bullets_once = 15;
	double_barrel.bullet_min_damage = 10.f;
	double_barrel.bullet_max_damage = 12.f;
	double_barrel.bullet_distance_offset = 120.f;
	double_barrel.bullet_speed = 5000.f;
	double_barrel.bullet_sprite = &bullet_sprite;
	double_barrel.is_automatic = true;
	double_barrel.max_rounds = 2;
	double_barrel.shooting_interval_ms = 500.f;
	double_barrel.spread_radians = 5.f * 0.01745329251994329576923690768489f;
	double_barrel.velocity_variation = 1500.f;
	double_barrel.shake_radius = 0.5f;
	double_barrel.shake_spread_radians = 45.f * 0.01745329251994329576923690768489f;
	double_barrel.bullet_collision_filter = filter_bullets;
	double_barrel.bullet_layer = BULLETS;
	double_barrel.max_bullet_distance = 5000.f;

	components::health::health_info npc_health_info;
	npc_health_info.dead_lifetime_ms = 1000.f;
	npc_health_info.should_disappear = true;
	npc_health_info.death_render = components::render(ON_GROUND, &dead_front_sprite);
	npc_health_info.max_hp = 100.f;
	npc_health_info.corpse_collision_filter = filter_corpses;

	auto spawn_npc = [&](components::animate& animate_component){
		entity& physical = my_world.create_entity();
		entity& legs = my_world.create_entity();

		components::movement player_movement(vec2<>(15000.f, 15000.f), 15000.f);
		player_movement.animation_receivers.push_back(components::movement::subscribtion(&physical, false));
		player_movement.animation_receivers.push_back(components::movement::subscribtion(&legs, true));

		physical.add(components::render(PLAYERS, &player_sprite));
		physical.add(components::transform(vec2<>(0.f, 0.f)));
		physical.add(components::particle_emitter(&player_effects));
		physical.add(animate_component);
		physical.add(player_movement);
		physical.add(components::health(&npc_health_info, 500.f));
		
		components::children player_children;
		player_children.children_entities.push_back(&legs);
		physical.add(player_children);

		components::gun my_gun(&assault_rifle);
		my_gun.current_rounds = 1000;

		physical.add(my_gun);

		topdown::create_physics_component(physical, physics.b2world, filter_characters, b2_dynamicBody);
		physical.get<components::physics>().body->SetLinearDamping(13.0f);
		physical.get<components::physics>().body->SetAngularDamping(5.0f);
		physical.get<components::physics>().body->SetFixedRotation(true);

		legs.add(legs_animate);
		legs.add(components::render(LEGS, nullptr));
		legs.add(components::chase(&physical));
		legs.add(components::transform());
		legs.add(components::lookat(&physical, components::lookat::chase_type::VELOCITY));
		
		return std::pair<entity&, entity&>(physical, legs);
	};
	
	auto player = spawn_npc(player_shotgun_animate);

	components::input player_input;
	player_input.intents.add(intent_message::intent::MOVE_FORWARD);
	player_input.intents.add(intent_message::intent::MOVE_BACKWARD);
	player_input.intents.add(intent_message::intent::MOVE_LEFT);
	player_input.intents.add(intent_message::intent::MOVE_RIGHT);
	player_input.intents.add(intent_message::intent::SHOOT);
	
	player.first.add(player_input);
	player.first.add(components::lookat(&crosshair));

	player.first.get<components::physics>().body->SetTransform(vec2<>(-500.f*PIXELS_TO_METERSf, 0.f), 0.f);
	player.first.get<components::gun>().target_camera_to_shake = &world_camera;
	
	for (int i = 0; i < 30; ++i) {
		auto npc = spawn_npc(player_animate);
		npc.first.get<components::physics>().body->SetLinearDamping(4.0f);
		npc.first.get<components::physics>().body->SetFixedRotation(false);
		npc.first.get<components::physics>().body->GetFixtureList()->SetDensity(0.1f);
		npc.first.get<components::physics>().body->ResetMassData();
	}

	rect.add(components::render(OBJECTS, &my_sprite));
	rect.add(components::transform(vec2<>(500.f, -50.f)));
	rect.add(components::particle_emitter(&wood_effects));
	topdown::create_physics_component(rect, physics.b2world, filter_objects);
	rect1.add(components::render(OBJECTS, &my_sprite));
	rect1.add(components::transform(vec2<>(470.f, 50.f)));
	rect1.add(components::particle_emitter(&wood_effects));
	topdown::create_physics_component(rect1, physics.b2world, filter_objects);
	rect2.add(components::render(OBJECTS, &small_sprite));
	rect2.add(components::particle_emitter(&wood_effects));
	rect2.add(components::transform(vec2<>(400.f, 0.f), 45.f * 0.01745329251994329576923690768489f));
	topdown::create_physics_component(rect2, physics.b2world, filter_objects);

	ground.add(components::render(OBJECTS, &bigger_sprite));
	ground.add(components::transform(vec2<>(400.f, 400.f), 75.f * 0.01745329251994329576923690768489f));
	ground.add(components::particle_emitter(&wood_effects));
	topdown::create_physics_component(ground, physics.b2world, filter_objects, b2_dynamicBody);
    ground.get<components::physics>().body->GetFixtureList()->SetFriction(0.0f);

	rect.get<components::physics>().body->SetLinearDamping(5.f);
	rect1.get<components::physics>().body->SetLinearDamping(5.f);
	rect2.get<components::physics>().body->SetLinearDamping(5.f);
	ground.get<components::physics>().body->SetLinearDamping(5.f);
	rect.get<components::physics>().body->SetAngularDamping(5.f);
	rect1.get<components::physics>().body->SetAngularDamping(5.f);
	rect2.get<components::physics>().body->SetAngularDamping(5.f);
	ground.get<components::physics>().body->SetAngularDamping(5.f);

	crosshair.add(components::render(GUI_OBJECTS, &crosshair_sprite));
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
	world_camera.get<components::camera>().orbit_mode = components::camera::ANGLED;
	world_camera.get<components::camera>().max_look_expand = vec2<>(vec2<int>(gl.get_screen_rect())) * 0.5f;
	world_camera.get<components::camera>().enable_smoothing = true;
	world_camera.add(components::input());
	world_camera.get<components::input>().intents.add(intent_message::intent::SWITCH_LOOK);
	
	while (!quit_flag) {
		my_world.run();

		/* flushing message queues */
		my_world.get_message_queue<message>().clear();
		my_world.get_message_queue<moved_message>().clear();
		my_world.get_message_queue<intent_message>().clear();
		my_world.get_message_queue<damage_message>().clear();
		my_world.get_message_queue<destroy_message>().clear();
		my_world.get_message_queue<animate_message>().clear();
		my_world.get_message_queue<collision_message>().clear();
		my_world.get_message_queue<particle_burst_message>().clear();
	}
	
	augmentations::deinit();
	return 0;
}

#endif