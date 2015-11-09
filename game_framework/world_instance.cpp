#include "world_instance.h"

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
#include "systems/behaviour_tree_system.h"

#include "messages/destroy_message.h" 
#include "messages/collision_message.h"
#include "messages/intent_message.h"
#include "messages/animate_message.h"
#include "messages/particle_burst_message.h"
#include "messages/damage_message.h"
#include "messages/shot_message.h"

using namespace messages;

augs::window::glwindow* world_instance::global_window = nullptr;

world_instance::~world_instance() {
	/* delete all entities before the systems get destroyed */
	my_world.delete_all_entities();
}

world_instance::world_instance() {
	my_world.register_component<components::animate>();
	my_world.register_component<components::behaviour_tree>();
	my_world.register_component<components::camera>();
	my_world.register_component<components::chase>();
	my_world.register_component<components::children>();
	my_world.register_component<components::crosshair>();
	my_world.register_component<components::damage>();
	my_world.register_component<components::gun>();
	my_world.register_component<components::input>();
	my_world.register_component<components::lookat>();
	my_world.register_component<components::movement>();
	my_world.register_component<components::particle_emitter>();
	my_world.register_component<components::particle_group>();
	my_world.register_component<components::pathfinding>();
	my_world.register_component<components::physics>();
	my_world.register_component<components::render>();
	my_world.register_component<components::steering>();
	my_world.register_component<components::transform>();
	my_world.register_component<components::visibility>();


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