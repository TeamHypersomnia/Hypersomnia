#pragma once
#include "stdafx.h"
#include "bindings.h"
#include "../components/behaviour_tree_component.h"

namespace bindings {
	luabind::scope _behaviour_tree_component() {
		return
			luabind::class_<behaviour_tree::behaviour>("behaviour_node"),
			//luabind::class_<behaviour_tree::selector, behaviour_tree::behaviour>("selector"),
			//luabind::class_<behaviour_tree::sequencer, behaviour_tree::behaviour>("sequencer"),

			luabind::class_<behaviour_tree>("behaviour_tree_component")
			//.def(luabind::constructor<>())
			//.def_readwrite("root", &behaviour_tree::root);
			;
	}
}