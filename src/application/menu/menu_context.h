#pragma once
#include "augs/gui/dereferenced_location.h"
#include "augs/gui/basic_gui_context.h"
#include "augs/templates/maybe_const.h"
#include "augs/audio/audio_settings.h"
#include "application/menu/menu_element_location.h"
#include "application/menu/menu_root.h"
#include "application/main_menu_button_type.h"
#include "application/ingame_menu_button_type.h"

template <bool is_const, class Enum>
class menu_context : public augs::gui::basic_context<menu_element_location<Enum>, is_const, menu_context<is_const, Enum>> {
public:
	using root_type = menu_root<Enum>;
	using menu_root_ref = maybe_const_ref_t<is_const, root_type>;

	menu_context(
		const augs::audio_volume_settings& audio_volume,
		rect_world_ref world,
		tree_ref tree,
		menu_root_ref root
	) : 
		base(world, tree), 
		audio_volume(audio_volume), 
		root(root)
	{}

	const augs::audio_volume_settings& audio_volume;
	menu_root_ref root;

	template <class other_context>
	operator other_context() const {
		return { audio_volume, world, tree, root };
	}
	
	auto get_root_id() const {
		return menu_root_in_context<Enum>();
	}

	auto& get_root() const {
		return root;
	}
};

using main_menu_context = menu_context<false, main_menu_button_type> ;
using const_main_menu_context = menu_context<true, main_menu_button_type>;

using ingame_menu_context = menu_context<false, ingame_menu_button_type>;
using const_ingame_menu_context = menu_context<true, ingame_menu_button_type>;