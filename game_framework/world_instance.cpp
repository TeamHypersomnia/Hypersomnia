#include "stdafx.h"
#include "world_instance.h"

#include "resources/scriptable_info.h"
using namespace messages;

world_instance::~world_instance() {
	/* delete all entities before the systems get destroyed */
	my_world.delete_all_entities();
}

world_instance::world_instance() : input(*global_window), render(*global_window) {
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

	physics.substepping_routine = [this](world& owner){
		scripts.substep(owner);
		steering.substep(owner);
		movement.substep(owner);
		destroy.consume_events(owner);
		owner.flush_message_queues();
	};
}

void world_instance::default_loop() {
	my_world.validate_delayed_messages();

	input.process_entities(my_world);
	camera.consume_events(my_world);

	movement.process_entities(my_world);

	camera.process_entities(my_world);

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

	camera.process_rendering(my_world);

	my_world.flush_message_queues();
	//std::cout << physics.ray_casts_per_frame << std::endl;

	physics.ray_casts_per_frame = 0;
	//lua_gc(scripts.lua_state, LUA_GCCOLLECT, 0);

	if (world_reloading_script) {
		my_world.delete_all_entities();
		world_reloading_script->call();
		world_reloading_script = nullptr;
		lua_gc(*global_lua_state, LUA_GCCOLLECT, 0);
	}

	//auto& scripts_reloaded = resources::script::script_reloader.get_script_files_to_reload();
	//
	//for (auto& script_to_reload : scripts_reloaded) {
	//	if (script_to_reload->reload_scene_when_modified) {
	//		my_world.delete_all_entities();
	//		break;
	//	}
	//}
	//
	//for (auto& script_to_reload : scripts_reloaded) {
	//	script_to_reload->call();
	//}
	//
	//if (!scripts_reloaded.empty()) {
	//	std::cout << std::endl;
	//	lua_gc(*global_lua_state, LUA_GCCOLLECT, 0);
	//}
}

luabind::scope world_instance::bind() {
	return 
		luabind::class_<world_instance>("world_instance")
		.def(luabind::constructor<>())
		.def("default_loop", &world_instance::default_loop)

		.def_readwrite("world", &world_instance::my_world)
		.def_readwrite("input_system", &world_instance::input)
		.def_readwrite("steering_system", &world_instance::steering)
		.def_readwrite("movement_system", &world_instance::movement)
		.def_readwrite("animation_system", &world_instance::animations)
		.def_readwrite("crosshair_system", &world_instance::crosshairs)
		.def_readwrite("lookat_system", &world_instance::lookat)
		.def_readwrite("physics_system", &world_instance::physics)
		.def_readwrite("visibility_system", &world_instance::visibility)
		.def_readwrite("pathfinding_system", &world_instance::pathfinding)
		.def_readwrite("gun_system", &world_instance::guns)
		.def_readwrite("particle_group_system", &world_instance::particles)
		.def_readwrite("particle_emitter_system", &world_instance::emitters)
		.def_readwrite("render_system", &world_instance::render)
		.def_readwrite("camera_system", &world_instance::camera)
		.def_readwrite("chase_system", &world_instance::chase)
		.def_readwrite("damage_system", &world_instance::damage)
		.def_readwrite("destroy_system", &world_instance::destroy)
		.def_readwrite("behaviour_tree_system", &world_instance::behaviours)
		.def_readwrite("script_system", &world_instance::scripts)
		 ;
}