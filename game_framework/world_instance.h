#include "entity_system/world.h"

#include "utilities/lua_state_wrapper.h"
#include "window_framework/window.h"
#include <luabind/luabind.hpp>

struct world_instance {
	static augs::window::glwindow* global_window;
	/* all systems */
	augs::entity_system::world my_world;
	augs::entity_system::world& get_world() { return my_world; }

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