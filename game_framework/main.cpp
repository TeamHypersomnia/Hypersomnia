#pragma once
#include "stdafx.h"
#include <gtest\gtest.h>
#include "augmentations.h"
#include "window_framework/window.h"

#include "entity_system/world.h"

#include "systems/physics_system.h"
#include "systems/steering_system.h"
#include "systems/movement_system.h"
#include "systems/visibility_system.h"
#include "systems/pathfinding_system.h"
#include "systems/animation_system.h"
#include "systems/camera_system.h"
#include "systems/render_system.h"
#include "systems/input_system.h"
#include "systems/gun_system.h"
#include "systems/crosshair_system.h"
#include "systems/lookat_system.h"
#include "systems/chase_system.h"
#include "systems/damage_system.h"
#include "systems/destroy_system.h"
#include "systems/particle_group_system.h"
#include "systems/particle_emitter_system.h"
#include "systems/script_system.h"
#include "systems/behaviour_tree_system.h"

#include "messages/destroy_message.h" 
#include "messages/collision_message.h"
#include "messages/intent_message.h"
#include "messages/animate_message.h"
#include "messages/particle_burst_message.h"
#include "messages/damage_message.h"
#include "messages/steering_message.h"
#include "messages/shot_message.h"

#include "game/body_helper.h"
#include "game/texture_helper.h"

#include "resources/render_info.h"
#include "resources/animate_info.h"
#include "resources/scriptable_info.h"
#include "graphics/shader.h"

using namespace augs;
using namespace entity_system;
using namespace messages;

resources::script* world_reloading_script = nullptr;
#include <float.h>
//unsigned int fp_control_state = _controlfp(_EM_INVALID, _MCW_EM);
int main() {    

	//_clearfp();
	//_controlfp(_controlfp(0, 0) & ~(_EM_INVALID | _EM_ZERODIVIDE | _EM_OVERFLOW),
	//	_MCW_EM);
	augs::init();	
	script_system scripts;
	lua_gc(scripts.lua_state, LUA_GCCOLLECT, 0);
	resources::script::script_reloader.report_errors = &std::cout;
	resources::script::lua_state = scripts.lua_state;
	resources::script::dofile("config.lua");

	window::glwindow gl;
	gl.create(scripts.lua_state, rects::wh(100, 100));
	gl.set_show(gl.SHOW);
	gl.vsync(0);
	window::cursor(false); 
	 
	input_system input(gl);
	steering_system steering;
	movement_system movement;
	animation_system animations;
	crosshair_system crosshairs;
	lookat_system lookat;
	physics_system physics;
	visibility_system visibility;
	pathfinding_system pathfinding;
	gun_system guns;
	particle_group_system particles;
	particle_emitter_system emitters;
	render_system render(gl);
	camera_system camera;
	chase_system chase;
	damage_system damage;
	destroy_system destroy;
	behaviour_tree_system behaviours;

	world my_world;

	my_world.register_system(&input);
	my_world.register_system(&steering);
	my_world.register_system(&movement);
	my_world.register_system(&animations);
	my_world.register_system(&crosshairs);
	my_world.register_system(&lookat);
	my_world.register_system(&physics);
	my_world.register_system(&visibility);
	my_world.register_system(&pathfinding);
	my_world.register_system(&guns);
	my_world.register_system(&particles);
	my_world.register_system(&emitters);
	my_world.register_system(&render);
	my_world.register_system(&camera);
	my_world.register_system(&chase);
	my_world.register_system(&damage);
	my_world.register_system(&destroy);
	my_world.register_system(&behaviours);
	my_world.register_system(&scripts);
	 
	my_world.register_message_queue<intent_message>();
	my_world.register_message_queue<damage_message>();
	my_world.register_message_queue<destroy_message>();
	my_world.register_message_queue<animate_message>();
	my_world.register_message_queue<collision_message>();
	my_world.register_message_queue<particle_burst_message>();
	my_world.register_message_queue<shot_message>();

	scripts.global("world", my_world);
	scripts.global("window", gl);
	scripts.global("input_system", input);
	scripts.global("visibility_system", visibility);
	scripts.global("pathfinding_system", pathfinding);
	scripts.global("render_system", render);
	scripts.global("physics_system", physics);
	scripts.global("script_reloader", resources::script::script_reloader);

	components::physics my_comp, my_comp2;
	my_comp.original_model.push_back(augs::vec2<>(2342342, 1219839));
	my_comp.original_model.push_back(augs::vec2<>(4, 3));
	my_comp.original_model.push_back(augs::vec2<>(5645, 2342));
	my_comp.original_model.push_back(augs::vec2<>(2342342, 1219839));
	my_comp.original_model.push_back(augs::vec2<>(4, 3));
	my_comp.original_model.push_back(augs::vec2<>(5645, 2342));
	my_comp.original_model.push_back(augs::vec2<>(2342342, 1219839));
	my_comp.original_model.push_back(augs::vec2<>(4, 3));
	my_comp.original_model.push_back(augs::vec2<>(5645, 2342));

	my_comp2 = my_comp;

	resources::script init_script;

	init_script.associate_filename("init.lua");
	init_script.add_reload_dependant(&init_script);
 	init_script.call();

	std::cout << std::endl;
	lua_gc(scripts.lua_state, LUA_GCCOLLECT, 0);

	int argc = 0;
	::testing::InitGoogleTest(&argc, (wchar_t**)nullptr);

	::testing::FLAGS_gtest_catch_exceptions = false;
	::testing::FLAGS_gtest_break_on_failure = false;
	auto result = RUN_ALL_TESTS();

	using namespace augs::graphics;
	
	//shader vertex_shader(GL_VERTEX_SHADER, "#version 330 \n\
	//uniform mat4 projection_matrix;					   										   \n\
	//layout(location = 0) in vec2 position;\n\
	//void main()\n\
	//{\n\
	//	vec4 output_vert;\
	//	output_vert.x = position.x;		\
	//	output_vert.y = position.y;				\
	//	output_vert.z = 0.0f;						\
	//	output_vert.w = 1.0f; \
	//	 \
	//	gl_Position = projection_matrix * output_vert;\n\
	//}\n");
	//
	//shader fragment_shader(GL_FRAGMENT_SHADER, "#version 330 \n\
	//										   											   \n\
	//	out vec4 outputColor;\n\
	//void main()\n\
	//{\n\
	//	outputColor = vec4(1.0f, 0.0f, 1.0f, 1.0f);\n\
	//}");

	//shader_program my_program;
	//my_program.attach(vertex_shader);
	//my_program.attach(fragment_shader);
	//my_program.build();
	//my_program.use();
	
	//glGetUniformLocation(my_program.id, "projection_matrix");

	float myval = std::numeric_limits<float>::quiet_NaN();
	std::cout << std::isnan(myval) << std::endl; 

	while (!input.quit_flag) {
		my_world.validate_delayed_messages();

		input.process_entities(my_world);                  
		movement.process_entities(my_world);

		physics.substepping_routine = [&steering, &movement, &damage, &destroy, &scripts](world& owner){
			scripts.substep(owner);
			steering.substep(owner);
			movement.substep(owner);
			destroy.consume_events(owner);
		};

		physics.process_entities(my_world);                 
		behaviours.process_entities(my_world);              
		lookat.process_entities(my_world);                  
		chase.process_entities(my_world);                   
		crosshairs.process_entities(my_world);              
		guns.process_entities(my_world);                   
		damage.process_entities(my_world);                  
		particles.process_entities(my_world);               
		animations.process_entities(my_world);              
		visibility.process_entities(my_world);              
		pathfinding.process_entities(my_world);             
		render.process_entities(my_world);                  
		camera.process_entities(my_world);                  
		scripts.process_entities(my_world);
		
		damage.process_events(my_world);
		destroy.consume_events(my_world);  

		scripts.process_events(my_world);

		damage.process_events(my_world);
		destroy.consume_events(my_world);

		movement.consume_events(my_world);
		animations.consume_events(my_world);
		crosshairs.consume_events(my_world);
		guns.consume_events(my_world);
		emitters.consume_events(my_world);
		camera.consume_events(my_world);

		my_world.flush_message_queues();
		//std::cout << physics.ray_casts_per_frame << std::endl;

		 
		if (world_reloading_script) {
			my_world.delete_all_entities(true);
			world_reloading_script->call();
			world_reloading_script = nullptr;
			lua_gc(scripts.lua_state, LUA_GCCOLLECT, 0);
		}
		else {
			auto& scripts_reloaded = resources::script::script_reloader.get_script_files_to_reload();

			for (auto& script_to_reload : scripts_reloaded) {
				if (script_to_reload->reload_scene_when_modified) {
					my_world.delete_all_entities(true);
					break;
				}
			}

			for (auto& script_to_reload : scripts_reloaded) {
				script_to_reload->call();
			}

			if (!scripts_reloaded.empty()) {
				std::cout << std::endl;
				lua_gc(scripts.lua_state, LUA_GCCOLLECT, 0);
			}
		}
	}

	augs::deinit();
	return 0;
}
