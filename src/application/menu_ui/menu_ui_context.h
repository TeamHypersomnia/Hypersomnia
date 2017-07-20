#pragma once
#include "augs/gui/dereferenced_location.h"
#include "augs/gui/basic_gui_context.h"
#include "augs/templates/maybe_const.h"
#include "augs/audio/audio_settings.h"
#include "application/menu_ui/menu_ui_element_location.h"
#include "application/menu_ui/menu_ui_root.h"
#include "application/main_menu_button_type.h"
#include "application/ingame_menu_button_type.h"

template <bool is_const, class Enum>
class menu_ui_context : public augs::gui::basic_context<menu_ui_element_location<Enum>, is_const, menu_ui_context<is_const, Enum>> {
public:
	//using base::base;
	//typedef augs::gui::basic_context<menu_ui_element_location, is_const, basic_menu_ui_context<is_const>> base;

	using root_type = menu_ui_root<Enum>;
	using menu_ui_root_ref = maybe_const_ref_t<is_const, root_type>;

//	typedef typename base::rect_world_ref rect_world_ref;
//	typedef typename base::tree_ref tree_ref;

	menu_ui_context(
		const augs::audio_volume_settings& audio_volume,
		rect_world_ref world,
		tree_ref tree,
		menu_ui_root_ref root
	) : 
		base(world, tree), 
		audio_volume(audio_volume), 
		root(root)
	{}

	const augs::audio_volume_settings& audio_volume;
	menu_ui_root_ref root;

	template <class other_context>
	operator other_context() const {
		return { audio_volume, world, tree, root };
	}
	
	auto get_root_id() const {
		return menu_ui_root_in_context<Enum>();
	}

	auto& get_root() const {
		return root;
	}
};

using main_menu_ui_context = menu_ui_context<false, main_menu_button_type> ;
using const_main_menu_ui_context = menu_ui_context<true, main_menu_button_type>;

using ingame_menu_ui_context = menu_ui_context<false, ingame_menu_button_type>;
using const_ingame_menu_ui_context = menu_ui_context<true, ingame_menu_button_type>;