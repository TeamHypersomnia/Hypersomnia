#pragma once
#include "augs/gui/dereferenced_location.h"
#include "augs/gui/basic_gui_context.h"
#include "augs/templates/maybe_const.h"
#include "augs/audio/audio_settings.h"

#include "application/gui/menu/menu_element_location.h"
#include "application/gui/menu/menu_root.h"

#include "game/view/necessary_resources.h"

template <bool is_const, class Enum>
class menu_context : public augs::gui::basic_context<menu_element_location<Enum>, is_const, menu_context<is_const, Enum>> {
public:
	using root_type = menu_root<Enum>;
	using menu_root_ref = maybe_const_ref_t<is_const, root_type>;

	menu_root_ref root;
	const necessary_images_in_atlas& necessarys;
	const augs::baked_font& gui_font;
	const necessary_sound_buffers& sounds;
	const augs::audio_volume_settings& audio_volume;
	
	menu_context(
		const base b,
		menu_root_ref root,
		const necessary_images_in_atlas& necessarys,
		const augs::baked_font& gui_font,
		const necessary_sound_buffers& sounds,
		const augs::audio_volume_settings& audio_volume
	) : 
		base(b),
		root(root),
		necessarys(necessarys),
		gui_font(gui_font),
		sounds(sounds),
		audio_volume(audio_volume)
	{}
	
	template <class other_context>
	operator other_context() const {
		return { 
			*static_cast<const base*>(this), 
			root,
			necessarys,
			gui_font,
			sounds, 
			audio_volume 
		};
	}

	/* Boilerplate getters, pay no heed */

	auto get_root_id() const {
		return menu_root_in_context<Enum>();
	}

	auto& get_root() const {
		return root;
	}

	auto& get_necessary_images() const {
		return necessarys;
	}

	const auto& get_gui_font() const {
		return gui_font;
	}
};

template <class Enum>
class viewing_menu_context : public menu_context<true, Enum> {
public:
	using base = menu_context<true, Enum>;
	
	const augs::drawer_with_default output;

	viewing_menu_context(
		const base b,
		const augs::drawer_with_default output
	) :
		base(b),
		output(output)
	{}

	/* Boilerplate getters, pay no heed */

	auto get_output() const {
		return output;
	}
};