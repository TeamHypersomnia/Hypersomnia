#pragma once
#include "augs/templates/maybe_const.h"
#include "augs/misc/basic_input_context.h"
#include "augs/graphics/vertex.h"
#include "augs/gui/dereferenced_location.h"
#include "augs/gui/basic_gui_context.h"

#include "game/enums/input_context_enums.h"
#include "game/transcendental/entity_handle.h"
#include "game/detail/camera_cone.h"
#include "game/detail/gui/game_gui_element_location.h"
#include "game/detail/gui/character_gui_drawing_input.h"

#include "application/config_lua_table.h"

class game_gui_root;
struct character_gui;

using game_gui_rect_tree = augs::gui::rect_tree<game_gui_element_location>;

template <bool is_const>
class basic_game_gui_context 
	: public augs::gui::basic_context<
		game_gui_element_location, 
		is_const, 
		basic_game_gui_context<is_const>
	> {
public:
	using base = augs::gui::basic_context<game_gui_element_location, is_const, basic_game_gui_context<is_const>>;
	using game_gui_system_ref = maybe_const_ref_t<is_const, game_gui_system>;
	using character_gui_ref = maybe_const_ref_t<is_const, character_gui>;
	using game_gui_root_ref = maybe_const_ref_t<is_const, game_gui_root>;
	using rect_world_ref = typename base::rect_world_ref;
	using tree_ref = typename base::tree_ref;

	basic_game_gui_context(
		game_gui_system_ref sys,
		rect_world_ref rect_world,
		character_gui_ref char_gui,
		const_entity_handle handle,
		tree_ref tree
	) : 
		base(rect_world, tree),
		sys(sys),
		handle(handle),
		char_gui(char_gui)
	{}

	game_gui_system_ref sys;
	const_entity_handle handle;
	character_gui_ref char_gui;

	template<class other_context>
	operator other_context() const {
		return other_context(
			get_game_gui_system(),
			get_rect_world(), 
			get_character_gui(), 
			handle,
			tree
		);
	}

	auto get_root_id() const {
		return game_gui_root_in_context();
	}

	auto& get_root() const {
		return sys.root;
	}

	const_entity_handle get_subject_entity() const {
		return handle;
	}

	const cosmos& get_cosmos() const {
		return handle.get_cosmos();
	}

	character_gui_ref get_character_gui() const {
		return char_gui;
	}

	game_gui_system_ref get_game_gui_system() const {
		return sys;
	}
};

using game_gui_context = basic_game_gui_context<false>;
using const_game_gui_context = basic_game_gui_context<true>;

class viewing_game_gui_context : public const_game_gui_context {
public:
	using base = const_game_gui_context;

	using rect_world_ref = typename base::rect_world_ref;
	using tree_ref = typename base::tree_ref;
	using character_gui_ref = typename base::character_gui_ref;
	using game_gui_root_ref = typename base::game_gui_root_ref;
	using game_gui_system_ref = typename base::game_gui_system_ref;

	viewing_game_gui_context(
		rect_world_ref rect_world,
		character_gui_ref char_gui,
		const_entity_handle handle,
		tree_ref tree,
		const character_gui_drawing_input in
	) :
		base(in.gui, rect_world, char_gui, handle, tree),
		in(in)
	{}

	const character_gui_drawing_input in;

	augs::vertex_triangle_buffer& get_output_buffer() const {
		return in.output;
	}

	const aabb_highlighter& get_aabb_highlighter() const {
		return in.world_hover_highlighter;
	}

	const interpolation_system& get_interpolation_system() const {
		return in.interpolation;
	}

	camera_cone get_camera_cone() const {
		return in.camera;
	}

	auto get_hotbar_settings() const {
		return in.hotbar;
	}

	auto get_input_information() const {
		return in.input_information;
	}
};