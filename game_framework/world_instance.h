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
#include "systems/behaviour_tree_system.h"

#include "messages/destroy_message.h" 
#include "messages/collision_message.h"
#include "messages/intent_message.h"
#include "messages/animate_message.h"
#include "messages/particle_burst_message.h"
#include "messages/damage_message.h"
#include "messages/shot_message.h"

#include "utilities/lua_state_wrapper.h"
#include <luabind/luabind.hpp>

struct world_instance {
	static augs::window::glwindow* global_window;
	/* all systems */
	world my_world;
	world& get_world() { return my_world; }

	world_instance();
	~world_instance();

	void default_loop();

	world_instance& operator=(const world_instance&) {
		assert(0);
		return *this;
	}

	template<typename T>
	T* get() {
		return &my_world.get_system<T>();
	}

	static luabind::scope bind();
};