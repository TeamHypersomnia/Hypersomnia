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

class root_of_inventory_gui;
struct character_gui;

typedef augs::gui::rect_tree<game_gui_element_location> game_gui_rect_tree;

template <bool is_const>
class basic_game_gui_context 
	: public augs::gui::basic_context<
		game_gui_element_location, 
		is_const, 
		basic_game_gui_context<is_const>
	> {
public:
	typedef augs::gui::basic_context<game_gui_element_location, is_const, basic_game_gui_context<is_const>> base;
	 
	typedef maybe_const_ref_t<is_const, game_gui_system> game_gui_system_ref;
	typedef maybe_const_ref_t<is_const, character_gui> character_gui_ref;
	typedef maybe_const_ref_t<is_const, root_of_inventory_gui> root_of_inventory_gui_ref;

	typedef typename base::rect_world_ref rect_world_ref;
	typedef typename base::tree_ref tree_ref;

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
		return root_of_inventory_gui_in_context();
	}

	auto& get_root() const {
		return sys.root;
	}

	const_entity_handle get_gui_element_entity() const {
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

typedef basic_game_gui_context<false> game_gui_context;
typedef basic_game_gui_context<true> const_game_gui_context;

class viewing_game_gui_context : public const_game_gui_context {
public:
	typedef const_game_gui_context base;

	typedef typename base::rect_world_ref rect_world_ref;
	typedef typename base::tree_ref tree_ref;
	typedef typename base::character_gui_ref character_gui_ref;
	typedef typename base::root_of_inventory_gui_ref root_of_inventory_gui_ref;
	typedef typename base::game_gui_system_ref game_gui_system_ref;

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