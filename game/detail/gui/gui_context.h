#pragma once
#include "augs/gui/dereferenced_location.h"
#include "augs/gui/basic_gui_context.h"
#include "augs/templates/maybe_const.h"
#include "game_gui_element_location.h"
#include "game/transcendental/step.h"

class root_of_inventory_gui;

namespace component {
	struct gui_element;
}

class gui_tree_entry;

typedef augs::gui::rect_tree<game_gui_element_location> game_gui_element_tree;

template <class step_type>
class basic_gui_context : public augs::gui::basic_context<game_gui_element_location, entity_handle_type_for_step<step_type>::is_const_value, basic_gui_context<step_type>> {
public:
	typedef entity_handle_type_for_step<step_type> entity_handle_type;
	typedef augs::gui::basic_context<game_gui_element_location, is_const, basic_gui_context<step_type>> base;
	 
	typedef maybe_const_ref_t<is_const, components::gui_element> gui_element_ref;
	typedef maybe_const_ref_t<is_const, root_of_inventory_gui> root_of_inventory_gui_ref;

	basic_gui_context(
		step_type step, 
		entity_handle_type handle, 
		tree_ref tree, 
		root_of_inventory_gui_ref root) : base(handle.get<components::gui_element>().rect_world, tree),
		step(step),
		handle(handle),
		elem(handle.get<components::gui_element>()),
		root(root)
	{}

	step_type step;
	entity_handle_type handle;
	gui_element_ref elem;
	root_of_inventory_gui_ref root;

	template<class other_context>
	operator other_context() const {
		return other_context(step, handle, tree, root);
	}

	root_of_inventory_gui_ref get_root_of_inventory_gui() const {
		return root;
	}

	basic_entity_handle<is_const> get_gui_element_entity() const {
		return handle;
	}

	step_type get_step() const {
		return step;
	}

	gui_element_ref get_gui_element_component() const {
		return elem;
	}
};

typedef basic_gui_context<logic_step> logic_gui_context;
typedef basic_gui_context<const_cosmic_step> const_gui_context;
typedef basic_gui_context<viewing_step> viewing_gui_context;
