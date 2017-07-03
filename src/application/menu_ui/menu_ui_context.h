#pragma once
#include "augs/gui/dereferenced_location.h"
#include "augs/gui/basic_gui_context.h"
#include "augs/templates/maybe_const.h"
#include "application/ui/app_ui_element_location.h"

typedef augs::gui::rect_tree<app_ui_element_location> app_ui_rect_tree;

class app_ui_root;

template <bool is_const>
class basic_app_ui_context : public augs::gui::basic_context<app_ui_element_location, is_const, basic_app_ui_context<is_const>> {
public:
	typedef augs::gui::basic_context<app_ui_element_location, is_const, basic_app_ui_context<is_const>> base;

	typedef maybe_const_ref_t<is_const, app_ui_root> app_ui_root_ref;

	typedef typename base::rect_world_ref rect_world_ref;
	typedef typename base::tree_ref tree_ref;

	basic_app_ui_context(
		rect_world_ref world,
		tree_ref tree,
		app_ui_root_ref root
	) : base(world, tree), root(root)
	{}

	app_ui_root_ref root;

	template<class other_context>
	operator other_context() const {
		return other_context(world, tree, root);
	}

	app_ui_root_ref get_root_of_app_ui() const {
		return root;
	}
};

typedef basic_app_ui_context<false> app_ui_context;
typedef basic_app_ui_context<true> const_app_ui_context;
