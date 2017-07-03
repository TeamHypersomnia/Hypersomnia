#pragma once
#include "augs/gui/dereferenced_location.h"
#include "augs/gui/basic_gui_context.h"
#include "augs/templates/maybe_const.h"
#include "application/menu_ui/menu_ui_element_location.h"

typedef augs::gui::rect_tree<menu_ui_element_location> menu_ui_rect_tree;

class menu_ui_root;

template <bool is_const>
class basic_menu_ui_context : public augs::gui::basic_context<menu_ui_element_location, is_const, basic_menu_ui_context<is_const>> {
public:
	typedef augs::gui::basic_context<menu_ui_element_location, is_const, basic_menu_ui_context<is_const>> base;

	typedef maybe_const_ref_t<is_const, menu_ui_root> menu_ui_root_ref;

	typedef typename base::rect_world_ref rect_world_ref;
	typedef typename base::tree_ref tree_ref;

	basic_menu_ui_context(
		rect_world_ref world,
		tree_ref tree,
		menu_ui_root_ref root
	) : base(world, tree), root(root)
	{}

	menu_ui_root_ref root;

	template<class other_context>
	operator other_context() const {
		return other_context(world, tree, root);
	}

	menu_ui_root_ref get_root_of_menu_ui() const {
		return root;
	}
};

typedef basic_menu_ui_context<false> menu_ui_context;
typedef basic_menu_ui_context<true> const_menu_ui_context;
