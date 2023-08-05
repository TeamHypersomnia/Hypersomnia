#pragma once
#include "augs/gui/dereferenced_location.h"
#include "augs/gui/basic_gui_context.h"
#include "augs/templates/maybe_const.h"
#include "augs/audio/audio_settings.h"

#include "view/necessary_resources.h"

#include "application/gui/menu/menu_element_location.h"
#include "application/gui/menu/menu_root.h"

struct menu_context_dependencies {
	const necessary_images_in_atlas_map& necessarys;
	const augs::baked_font& gui_font;
	const all_necessary_sounds& sounds;
	const augs::audio_volume_settings& audio_volume;
	const bool will_quit_to_editor;
	const bool will_quit_to_projects;
	const bool is_tutorial;
};

template <bool is_const, class Enum>
class menu_context : public augs::gui::basic_context<menu_element_location<Enum>, is_const, menu_context<is_const, Enum>> {
public:
	using base = augs::gui::basic_context<menu_element_location<Enum>, is_const, menu_context<is_const, Enum>>;

	using root_type = menu_root<Enum>;
	using menu_root_ref = maybe_const_ref_t<is_const, root_type>;

	menu_root_ref root;
	const menu_context_dependencies deps;

	menu_context(
		const base b,
		menu_root_ref root,
		const menu_context_dependencies deps
	) : 
		base(b),
		root(root),
		deps(deps)
	{}
	
	template <class other_context>
	operator other_context() const {
		return { 
			*static_cast<const base*>(this), 
			root,
			deps
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
		return deps.necessarys;
	}

	const auto& get_gui_font() const {
		return deps.gui_font;
	}

	const auto& get_audio_volume() const {
		return deps.audio_volume;
	}

	const auto& get_sounds() const {
		return deps.sounds;
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