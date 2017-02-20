#pragma once
#include "augs/gui/dereferenced_location.h"
#include "augs/gui/basic_gui_context.h"
#include "augs/templates/maybe_const.h"
#include "game_gui_element_location.h"
#include "game/transcendental/step.h"
#include "game/transcendental/entity_handle.h"
#include "augs/graphics/vertex.h"

#include "application/config_lua_table.h"
#include "augs/misc/input_context.h"

class root_of_inventory_gui;
struct character_gui;
struct aabb_highlighter;
class gui_element_system;

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
	 
	typedef maybe_const_ref_t<is_const, gui_element_system> gui_element_system_ref;
	typedef maybe_const_ref_t<is_const, character_gui> character_gui_ref;
	typedef maybe_const_ref_t<is_const, root_of_inventory_gui> root_of_inventory_gui_ref;

	typedef typename base::rect_world_ref rect_world_ref;
	typedef typename base::tree_ref tree_ref;

	basic_game_gui_context(
		gui_element_system_ref sys,
		rect_world_ref rect_world,
		character_gui_ref char_gui,
		const_entity_handle handle,
		tree_ref tree, 
		root_of_inventory_gui_ref root
	) : 
		base(rect_world, tree),
		sys(sys),
		handle(handle),
		char_gui(char_gui),
		root(root)
	{}

	gui_element_system_ref sys;
	const_entity_handle handle;
	character_gui_ref char_gui;
	root_of_inventory_gui_ref root;

	template<class other_context>
	operator other_context() const {
		return other_context(
			get_gui_element_system(),
			get_rect_world(), 
			get_character_gui(), 
			handle,
			tree,
			root
		);
	}

	root_of_inventory_gui_ref get_root_of_inventory_gui() const {
		return root;
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

	gui_element_system_ref get_gui_element_system() const {
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
	typedef typename base::gui_element_system_ref gui_element_system_ref;

	viewing_game_gui_context(
		gui_element_system_ref sys,
		rect_world_ref rect_world,
		character_gui_ref char_gui,
		const_entity_handle handle,
		tree_ref tree,
		root_of_inventory_gui_ref root,
		const config_lua_table::hotbar_settings& hotbar_settings,
		camera_cone camera,
		const aabb_highlighter& highlighter,
		const interpolation_system& interp,
		const float interpolation_ratio,
		const input_context& input_information,
		augs::vertex_triangle_buffer& output
	) :
		base(sys, rect_world, char_gui, handle, tree, root),
		hotbar_settings(hotbar_settings),
		camera(camera),
		output(output),
		interp(interp),
		interpolation_ratio(interpolation_ratio),
		highlighter(highlighter),
		input_information(input_information)
	{}

	const config_lua_table::hotbar_settings& hotbar_settings;
	camera_cone camera;
	augs::vertex_triangle_buffer& output;
	const aabb_highlighter& highlighter;
	const interpolation_system& interp;
	const input_context& input_information;
	const float interpolation_ratio;

	augs::vertex_triangle_buffer& get_output_buffer() const {
		return output;
	}

	const aabb_highlighter& get_aabb_highlighter() const {
		return highlighter;
	}

	const interpolation_system& get_interpolation_system() const {
		return interp;
	}

	camera_cone get_camera_cone() const {
		return camera;
	}
};