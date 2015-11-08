#include "world_instance.h"
#include "game_framework/components/all_components.h"

using namespace messages;

augs::window::glwindow* world_instance::global_window = nullptr;

world_instance::~world_instance() {
	/* delete all entities before the systems get destroyed */
	my_world.delete_all_entities();
}

world_instance::world_instance() {
	ALL_COMPONENTS(REGISTER_COMPONENT_TYPE, my_world);

	my_world.register_system<input_system>(std::ref(*global_window));
	my_world.register_system<steering_system>();
	my_world.register_system<movement_system>();
	my_world.register_system<animation_system>();
	my_world.register_system<crosshair_system>();
	my_world.register_system<lookat_system>();
	my_world.register_system<physics_system>();
	my_world.register_system<visibility_system>();
	my_world.register_system<pathfinding_system>();
	my_world.register_system<gun_system>();
	my_world.register_system<particle_group_system>();
	my_world.register_system<particle_emitter_system>();
	my_world.register_system<render_system>(std::ref(*global_window));
	my_world.register_system<camera_system>();
	my_world.register_system<chase_system>();
	my_world.register_system<damage_system>();
	my_world.register_system<destroy_system>();
	my_world.register_system<behaviour_tree_system>();

	my_world.register_message_queue<intent_message>();
	my_world.register_message_queue<damage_message>();
	my_world.register_message_queue<destroy_message>();
	my_world.register_message_queue<animate_message>();
	my_world.register_message_queue<collision_message>();
	my_world.register_message_queue<particle_burst_message>();
	my_world.register_message_queue<shot_message>();
}

void world_instance::default_loop() {

}

luabind::scope world_instance::bind() {
	return 
		luabind::class_<world_instance>("world_instance")
		.def(luabind::constructor<>())
		.def("default_loop", &world_instance::default_loop)

		.property("world", &world_instance::get_world)
		.property("input_system", &world_instance::get<input_system>)
		.property("steering_system", &world_instance::get<steering_system>)
		.property("movement_system", &world_instance::get<movement_system>)
		.property("animation_system", &world_instance::get<animation_system>)
		.property("crosshair_system", &world_instance::get<crosshair_system>)
		.property("lookat_system", &world_instance::get<lookat_system>)
		.property("physics_system", &world_instance::get<physics_system>)
		.property("visibility_system", &world_instance::get<visibility_system>)
		.property("pathfinding_system", &world_instance::get<pathfinding_system>)
		.property("gun_system", &world_instance::get<gun_system>)
		.property("particle_group_system", &world_instance::get<particle_group_system>)
		.property("particle_emitter_system", &world_instance::get<particle_emitter_system>)
		.property("render_system", &world_instance::get<render_system>)
		.property("camera_system", &world_instance::get<camera_system>)
		.property("chase_system", &world_instance::get<chase_system>)
		.property("damage_system", &world_instance::get<damage_system>)
		.property("destroy_system", &world_instance::get<destroy_system>)
		.property("behaviour_tree_system", &world_instance::get<behaviour_tree_system>)
		 ;
}