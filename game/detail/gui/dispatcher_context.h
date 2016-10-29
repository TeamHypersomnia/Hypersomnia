#pragma once
#include "augs/templates.h"
#include 

class viewing_step;
class fixed_step;

namespace component {
	struct gui_element;
}

template <bool is_const>
class basic_dispatcher_context {

public:
	typedef std::conditional_t<is_const, viewing_step, fixed_step>& step_ref;
	typedef maybe_const_ref_t<is_const, gui_element> gui_element_ref;
	typedef maybe_const_ref_t<is_const, game_gui_rect_world> game_gui_rect_world_ref;

	basic_dispatcher_context(step_ref step, basic_entity_handle<is_const> handle, gui_element_ref elem, gui_element_tree& tree) :
		step(step),
		handle(handle),
		//composite_for_iteration(parent),
		elem(elem),
		tree(tree)
	{}

	step_ref step;
	basic_entity_handle<is_const> handle;
	gui_element_ref elem;
	gui_element_tree& tree;

	basic_entity_handle<is_const> get_gui_element_entity() const {
		return handle;
	}
	//parent_of_inventory_controls_rect& composite_for_iteration;

	step_ref get_step() const {
		return step;
	}

	gui_element_ref get_gui_element_component() const {
		return elem;
	}

	game_gui_rect_world_ref get_rect_world() const {
		return elem.rect_world;
	}

	gui_tree_entry& get_tree_entry(const gui_element_location& id) {
		if (gui_tree.find(id) == gui_tree.end()) {
			gui_tree.emplace(id, gui_tree_entry(operator()(id, [](const auto& resolved_ref) {
				return static_cast<const augs::gui::rect_node_data&>(resolved_ref);
			})));
		}

		return gui_tree.at(id);
	}

	const gui_tree_entry& get_tree_entry(const gui_element_location& id) const {
		return gui_tree.at(id);
	}

	bool alive(const gui_element_location& id) const {
		return id.is_set() && id.call([this](const auto& resolved) {
			return resolved.alive(*this);
		});
	}

	bool dead(const gui_element_location& id) const {
		return !alive(id);
	}

	operator basic_dispatcher_context<true>() const {
		return{ step, handle, elem, tree };
	}

	template <class L>
	decltype(auto) operator()(const gui_element_location& id, L generic_call) const {
		return id.call([&](const auto& resolved_location) {
			return resolved_location.get_object_at_location_and_call(*this, generic_call);
		});
	}

	template <class Casted>
	struct pointer_caster {
		template <class Candidate>
		maybe_const_ptr_t<is_const, Casted> operator()(maybe_const_ref_t<is_const, Candidate> object) {
			if (std::is_same<Casted, Candidate>::value || std::is_base_of<Casted, Candidate>::value) {
				return reinterpret_cast<Casted*>(&object);
			}

			return nullptr;
		}
	};

	template <class T>
	location_and_pointer<T> _dynamic_cast(const gui_element_location& id) const {
		if (dead(id)) {
			return location_and_pointer<T>();
		}

		return id.call([&](const auto& resolved_location) const {
			return{ resolved_location.get_object_at_location_and_call(*this, pointer_caster<T>()), resolved_location };
		});
	}
};

typedef basic_dispatcher_context<false> dispatcher_context;
typedef basic_dispatcher_context<true> const_dispatcher_context;