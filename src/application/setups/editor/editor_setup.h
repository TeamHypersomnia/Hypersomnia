#pragma once
#include <optional>
#include "augs/misc/timing/fixed_delta_timer.h"
#include "augs/math/camera_cone.h"

#include "game/detail/render_layer_filter.h"
#include "game/cosmos/entity_handle.h"
#include "game/modes/test_mode.h"

#include "test_scenes/test_scene_settings.h"

#include "application/intercosm.h"
#include "application/setups/default_setup_settings.h"

#include "application/debug_settings.h"
#include "application/input/entropy_accumulator.h"
#include "application/setups/setup_common.h"
#include "view/mode_gui/arena/arena_player_meta.h"

#include "view/viewables/ad_hoc_atlas_subject.h"
#include "application/setups/editor/project/editor_project.h"

#include "application/setups/editor/gui/editor_inspector_gui.h"
#include "application/setups/editor/gui/editor_layers_gui.h"
#include "application/setups/editor/gui/editor_filesystem_gui.h"
#include "application/setups/editor/gui/editor_history_gui.h"

#include "application/setups/editor/editor_filesystem.h"
#include "application/setups/editor/editor_history.h"

#include "application/setups/editor/project/editor_project_paths.h"

#include "application/setups/editor/editor_view.h"
#include "augs/misc/imgui/simple_popup.h"
#include "application/setups/editor/editor_settings.h"

struct config_lua_table;
struct draw_setup_gui_input;

template <class E>
struct editor_typed_resource_id;

template <class E>
struct editor_typed_node_id;

namespace sol {
	class state;
}

struct editor_paths_changed_report {
	std::vector<std::pair<augs::path_type, augs::path_type>> redirects;
	std::vector<augs::path_type> missing;

	bool any() const {
		return redirects.size() > 0 || missing.size() > 0;
	}
};

struct editor_gui {
	// GEN INTROSPECTOR struct editor_gui
	editor_inspector_gui inspector = std::string("Inspector");
	editor_layers_gui layers = std::string("Layers");
	editor_filesystem_gui filesystem = std::string("Resources");
	editor_history_gui history = std::string("History");
	// END GEN INTROSPECTOR
};

class editor_setup : public default_setup_settings {
	intercosm scene;
	per_entity_type_array<std::vector<editor_node_id>> scene_entity_to_node;

	editor_resource_pools official_resources;

	editor_project project;
	editor_gui gui;
	editor_view view;

	std::optional<simple_popup> ok_only_popup;

	editor_history history;
	editor_filesystem files;

	const editor_project_paths paths;
	editor_settings settings;

	bool rebuild_ad_hoc_atlas = true;
	ad_hoc_atlas_subjects last_ad_hoc_subjects;

	void on_window_activate();
	void rebuild_filesystem();
	editor_paths_changed_report rebuild_pathed_resources();

	void force_autosave_now();

	void load_gui_state();
	void save_gui_state();
	void save_last_project_location();

	template <class S, class F>
	static decltype(auto) on_resource_impl(S& self, const editor_resource_id& id, F&& callback);

	template <class S, class T>
	static decltype(auto) find_resource_impl(S& self, const editor_typed_resource_id<T>& id);

	friend create_layer_command;

	template <class T>
	friend struct create_node_command;

	friend reorder_layers_command;

public:
	static constexpr auto loading_strategy = viewables_loading_type::LOAD_ALL;
	static constexpr bool handles_window_input = true;

	editor_setup(const augs::path_type& project_path);
	
	~editor_setup();

	void open_default_windows();

	bool handle_input_before_imgui(
		handle_input_before_imgui_input
	);

	bool handle_input_before_game(
		handle_input_before_game_input
	);

	void customize_for_viewing(config_lua_table&) const;
	std::optional<ad_hoc_atlas_subjects> get_new_ad_hoc_images();

	std::optional<std::pair<editor_layer_id, std::size_t>> find_parent_layer(editor_node_id id) const;

	const std::vector<editor_layer_id>& get_layers() const {
		return project.layers.order;
	}

	editor_layer* find_layer(const editor_layer_id& id);
	const editor_layer* find_layer(const editor_layer_id& id) const;

	editor_layer* find_layer(const std::string& name);
	void create_new_layer(const std::string& name_pattern = "New layer%x");
	std::string get_free_layer_name(const std::string& name_pattern = "New layer%x");

	template <class T>
	decltype(auto) find_node(const editor_typed_node_id<T>& id);

	template <class T>
	decltype(auto) find_node(const editor_typed_node_id<T>& id) const;

	template <class T>
	decltype(auto) find_resource(const editor_typed_resource_id<T>& id);

	template <class T>
	decltype(auto) find_resource(const editor_typed_resource_id<T>& id) const;

	template <class F>
	decltype(auto) on_node(const editor_node_id& id, F&& callback);

	template <class F>
	decltype(auto) on_node(const editor_node_id& id, F&& callback) const;

	template <class F>
	decltype(auto) on_resource(const editor_resource_id& id, F&& callback);

	template <class F>
	decltype(auto) on_resource(const editor_resource_id& id, F&& callback) const;

	editor_node_id to_node_id(entity_id) const;

	entity_id get_hovered_entity() const;
	editor_node_id get_hovered_node() const;

	std::unordered_map<std::string, editor_node_id> make_name_to_node_map() const;
	std::string get_free_node_name_for(const std::string& new_name) const;

	bool exists(const editor_resource_id&) const;

	void seek_to_revision(editor_history::index_type);
	bool wants_multiple_selection() const;

	template <class T>
	decltype(auto) post_new_command(T&&);

	template <class T>
	decltype(auto) rewrite_last_command(T&&);

	void inspect(inspected_variant);
	void inspect_only(inspected_variant);

	template <class T>
	bool inspects_any() const {
		return gui.inspector.template inspects_any<T>();
	}

	bool is_inspected(inspected_variant) const;
	std::vector<inspected_variant> get_all_inspected() const;

	template <class T>
	decltype(auto) get_all_inspected() const {
		return gui.inspector.template get_all_inspected<T>();
	}

	std::string get_name(inspected_variant) const;
	editor_history::index_type get_last_command_index() const;

	editor_command_input make_command_input(); 

	void undo();
	void undo_quiet();

	void redo();

	bool handle_doubleclick_in_layers_gui = false;

	void rebuild_scene();

	augs::path_type resolve(const augs::path_type& path_in_project) const;

	camera_eye get_camera_eye() const;

	vec2 get_world_cursor_pos() const;
	vec2 get_world_cursor_pos(const camera_eye eye) const;

	/*********************************************************/
	/*************** DEFAULT SETUP BOILERPLATE ***************/
	/*********************************************************/

	auto get_audiovisual_speed() const {
		return 1.0;
	}

	const auto& get_viewed_cosmos() const {
		return scene.world;
	}

	auto get_interpolation_ratio() const {
		return 1.0;
	}

	auto get_viewed_character_id() const {
		return entity_id();
	}

	auto get_controlled_character_id() const {
		return get_viewed_character_id();
	}

	auto get_viewed_character() const {
		return get_viewed_cosmos()[get_viewed_character_id()];
	}

	const auto& get_viewable_defs() const {
		return scene.viewables;
	}

	void perform_main_menu_bar(perform_custom_imgui_input);
	custom_imgui_result perform_custom_imgui(perform_custom_imgui_input);

	void apply(const config_lua_table&);

	auto escape() {
		return setup_escape_result::IGNORE;
	}

	auto get_inv_tickrate() const {
		return get_viewed_cosmos().get_fixed_delta().in_seconds<double>();
	}

	template <class C>
	void advance(
		const setup_advance_input& in,
		const C& callbacks
	) {
		(void)in;
		(void)callbacks;
	}

	template <class T>
	void control(const T&) {}

	void accept_game_gui_events(const game_gui_entropy_type&) {}

	std::optional<camera_eye> find_current_camera_eye() const {
		return get_camera_eye();
	}

	augs::path_type get_unofficial_content_dir() const;

	auto get_render_layer_filter() const {
		return render_layer_filter::disabled();
	}

	void draw_custom_gui(const draw_setup_gui_input&);

	void ensure_handler() {}
	bool requires_cursor() const { return false; }

private:
	entropy_accumulator zero_entropy;
public:

	const entropy_accumulator& get_entropy_accumulator() const {
		return zero_entropy;
	}

	template <class F>
	void on_mode_with_input(F&&) const {}

	auto get_game_gui_subject_id() const {
		return get_viewed_character_id();
	}

	std::nullopt_t get_new_player_metas() {
		return std::nullopt;
	}

	const arena_player_metas* find_player_metas() const {
		return nullptr;
	}

	void after_all_drawcalls(game_frame_buffer&) {}
	void do_game_main_thread_synced_op(renderer_backend_result&) {}
};