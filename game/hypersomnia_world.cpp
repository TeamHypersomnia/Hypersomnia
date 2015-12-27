#include "hypersomnia_world.h"

#include "game_framework/all_component_includes.h"
#include "game_framework/all_system_includes.h"
#include "game_framework/all_message_includes.h"

using namespace components;
using namespace messages;

hypersomnia_world::hypersomnia_world() {
	register_messages_components_systems();
}

void hypersomnia_world::register_messages_components_systems() {
	register_component<animate>();
	register_component<behaviour_tree>();
	register_component<camera>();
	register_component<chase>();
	register_component<children>();
	register_component<crosshair>();
	register_component<damage>();
	register_component<gun>();
	register_component<input>();
	register_component<lookat>();
	register_component<movement>();
	register_component<particle_emitter>();
	register_component<particle_group>();
	register_component<pathfinding>();
	register_component<physics>();
	register_component<render>();
	register_component<steering>();
	register_component<transform>();
	register_component<visibility>();

	register_system<input_system>();
	register_system<steering_system>();
	register_system<movement_system>();
	register_system<animation_system>();
	register_system<crosshair_system>();
	register_system<lookat_system>();
	register_system<physics_system>();
	register_system<visibility_system>();
	register_system<pathfinding_system>();
	register_system<gun_system>();
	register_system<particle_group_system>();
	register_system<particle_emitter_system>();
	register_system<render_system>();
	register_system<camera_system>();
	register_system<chase_system>();
	register_system<damage_system>();
	register_system<destroy_system>();
	register_system<behaviour_tree_system>();

	register_message_queue<intent_message>();
	register_message_queue<damage_message>();
	register_message_queue<destroy_message>();
	register_message_queue<animate_message>();
	register_message_queue<collision_message>();
	register_message_queue<particle_burst_message>();
	register_message_queue<shot_message>();
}

void hypersomnia_world::simulate() {


}
