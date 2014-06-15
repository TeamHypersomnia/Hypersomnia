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
#include "messages/shot_message.h"

struct world_instance {
	static augs::window::glwindow* global_window;
	/* all systems */
	world my_world;
	input_system input;
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
	render_system render;
	camera_system camera;
	chase_system chase;
	damage_system damage;
	destroy_system destroy;
	behaviour_tree_system behaviours;
	script_system scripts;

	world_instance();
	~world_instance();

	void default_loop();

	world_instance& operator=(const world_instance&) {
		assert(0);
		return *this;
	}

	static luabind::scope bind();
};