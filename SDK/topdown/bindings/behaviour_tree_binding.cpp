#pragma once
#include "stdafx.h"
#include "bindings.h"
#include "../components/behaviour_tree_component.h"

namespace bindings {
	luabind::scope _behaviour_tree_component() {
		return
			
			//luabind::class_<behaviour_tree::behaviour>("behaviour_interface")
			//.def(luabind::constructor<>())
			//,
			luabind::class_<behaviour_tree::decorator>("behaviour_decorator")
			.def(luabind::constructor<>())
			.def_readwrite("next_decorator", &behaviour_tree::decorator::next_decorator)
			,

			luabind::class_<behaviour_tree::timer_decorator, behaviour_tree::decorator>("behaviour_timer_decorator")
			.def(luabind::constructor<>())
			.def_readwrite("maximum_running_time_ms", &behaviour_tree::timer_decorator::maximum_running_time_ms)
			,

			luabind::class_<behaviour_tree::composite>("behaviour_node")
			.def(luabind::constructor<>())
			.def_readwrite("default_return", &behaviour_tree::composite::default_return)
			.def_readwrite("node_type", &behaviour_tree::composite::node_type)
			.def_readwrite("on_enter", &behaviour_tree::composite::enter_callback)
			.def_readwrite("on_exit", &behaviour_tree::composite::exit_callback)
			.def_readwrite("on_update", &behaviour_tree::composite::update_callback)
			.def_readwrite("name", &behaviour_tree::composite::name)
			.def_readwrite("decorator_chain", &behaviour_tree::composite::decorator_chain)
			.def_readwrite("concurrent_return", &behaviour_tree::composite::concurrent_return)
			.def_readwrite("skip_to_running_child", &behaviour_tree::composite::skip_to_running_child)
			.def("add_child", &behaviour_tree::composite::add_child)
			.enum_("constants")[
				luabind::value("SUCCESS", behaviour_tree::composite::SUCCESS),
				luabind::value("RUNNING", behaviour_tree::composite::RUNNING),
				luabind::value("FAILURE", behaviour_tree::composite::FAILURE),
				luabind::value("SEQUENCER", behaviour_tree::composite::SEQUENCER),
				luabind::value("SELECTOR", behaviour_tree::composite::SELECTOR),
				luabind::value("CONCURRENT", behaviour_tree::composite::CONCURRENT)
			]
			,
			
			//luabind::class_<behaviour_tree::tree>("behaviour_tree_allocator")
			//.def(luabind::constructor<>())
			//.def("create_flattened_tree", &behaviour_tree::tree::create_flattened_tree)
			//.def("retrieve_behaviour", &behaviour_tree::tree::retrieve_behaviour)
			//,

			luabind::class_<behaviour_tree>("behaviour_tree_component")
			.def(luabind::constructor<>())
			.def("add_tree", &behaviour_tree::add_tree);
			;
	}
}