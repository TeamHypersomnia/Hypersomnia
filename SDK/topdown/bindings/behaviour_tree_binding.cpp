#pragma once
#include "stdafx.h"
#include "bindings.h"
#include "../components/behaviour_tree_component.h"

namespace bindings {
	luabind::scope _behaviour_tree_component() {
		return
			luabind::class_<behaviour_tree::behaviour>("behaviour_node")
			.def(luabind::constructor<>())
			.def_readwrite("default_return", &behaviour_tree::behaviour::default_return)
			.def_readwrite("node_type", &behaviour_tree::behaviour::node_type)
			.def_readwrite("on_enter", &behaviour_tree::behaviour::enter_callback)
			.def_readwrite("on_exit", &behaviour_tree::behaviour::exit_callback)
			.def_readwrite("on_update", &behaviour_tree::behaviour::update_callback)
			.def_readwrite("name", &behaviour_tree::behaviour::name)
			.def("add_child", &behaviour_tree::behaviour::add_child)
			.enum_("constants")[
				luabind::value("SUCCESS", behaviour_tree::behaviour::SUCCESS),
				luabind::value("RUNNING", behaviour_tree::behaviour::RUNNING),
				luabind::value("FAILURE", behaviour_tree::behaviour::FAILURE),
				luabind::value("SEQUENCER", behaviour_tree::behaviour::SEQUENCER),
				luabind::value("SELECTOR", behaviour_tree::behaviour::SELECTOR)
			]
			,
			
			luabind::class_<behaviour_tree::tree>("behaviour_tree_allocator")
			.def(luabind::constructor<>())
			.def("create_flattened_tree", &behaviour_tree::tree::create_flattened_tree)
			.def("retrieve_behaviour", &behaviour_tree::tree::retrieve_behaviour)
			,

			luabind::class_<behaviour_tree>("behaviour_tree_component")
			.def(luabind::constructor<>())
			.def_readwrite("starting_node", &behaviour_tree::starting_node);
			;
	}
}