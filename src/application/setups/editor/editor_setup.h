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

#include "application/setups/editor/gui/editor_toolbar_gui.h"

#include "application/setups/editor/editor_filesystem.h"
#include "application/setups/editor/editor_history.h"

#include "application/setups/editor/selector/editor_entity_selector.h"
#include "application/setups/editor/mover/editor_node_mover.h"

#include "application/setups/editor/project/editor_project_paths.h"

#include "application/setups/editor/editor_view.h"
#include "augs/misc/imgui/simple_popup.h"
#include "application/setups/editor/editor_settings.h"
#include "augs/graphics/imgui_payload.h"
#include "application/setups/editor/commands/node_transform_commands.h"

#include "game/modes/arena_mode.h"

#include "game/stateless_systems/animation_system.h"
#include "view/faction_view_settings.h"

#include "application/arena/arena_handle.h"
#include "view/mode_gui/arena/arena_gui_mixin.h"
#include "application/setups/client/client_vars.h"
#include "application/setups/editor/gui/editor_recent_message.h"
#include "application/arena/arena_playtesting_context.h"

#include "application/main/miniature_generator.h"
#include "application/network/network_common.h"
#include "application/arena/scene_entity_to_node_map.h"

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
	editor_toolbar_gui toolbar = std::string("Toolbar");

	bool playtest_immersive = true;
	bool sounds_preview = true;

	bool request_toggle_sounds_preview = false;
	// END GEN INTROSPECTOR
};

struct editor_icon_info;
struct editor_icon_info_in;

struct packaged_official_content;
struct intercosm;
struct editor_resource_pools;
struct editor_official_resource_map;

template <bool C>
using editor_arena_handle = online_arena_handle<C>;

class editor_setup : public default_setup_settings, public arena_gui_mixin<editor_setup> {
	using arena_gui_base = arena_gui_mixin<editor_setup>;

	editor_settings settings;
	editor_autosave_settings last_autosave_settings;

	augs::timer autosave_timer;

	const packaged_official_content& official;

	intercosm scene;

	/* Arena handle essentials */
	all_rulesets_variant ruleset;
	all_modes_variant current_mode_state;
	cosmos_solvable_significant clean_round_state;

	entropy_accumulator total_collected;
	augs::fixed_delta_timer timer = { 5, augs::lag_spike_handling_type::DISCARD };
	mode_player_id local_player_id;

	bool playtesting = false;

	scene_entity_to_node_map scene_entity_to_node;

	editor_project project;
	editor_gui gui;
	editor_view view;

	editor_entity_selector selector;
	editor_node_mover mover;

	current_selections_type entity_selector_state;
	std::vector<entity_id> cached_selected_comparison;
	std::vector<entity_id> cached_selected_comparison_after;

	std::optional<simple_popup> autosave_popup;
	std::optional<simple_popup> invalid_filenames_popup;
	std::optional<simple_popup> redirect_or_missing_popup;

	std::optional<miniature_generator_state> miniature_generator;

	editor_forbidden_paths_result last_invalid_paths;

	editor_history history;
	editor_filesystem files;
	editor_filesystem_node official_files_root;

	const editor_project_paths paths;

	editor_recent_message recent_message;

	client_vars simulated_client;
	faction_view_settings faction_view;

	bool rebuild_ad_hoc_atlas = true;
	ad_hoc_atlas_subjects last_ad_hoc_subjects;

	double global_time_seconds = 0.0;

	bool dirty_after_loading_autosave = false;
	bool dirty_after_redirecting_paths = false;
	bool should_recount_internal_resource_references = true;

	std::optional<custom_imgui_result> imgui_return_once;

	void create_official_filesystems();

	void on_window_activate();

	void rescan_physical_filesystem();
	editor_paths_changed_report rebuild_pathed_resources();
	void report_changed_paths(const editor_paths_changed_report&);
	void autosave_if_redirected(
		const editor_paths_changed_report&,
		const bool during_activate,
		const bool undoing_to_first_revision
	);

	void rescan_missing_pathed_resources(std::vector<augs::path_type>* out_report = nullptr);
	void on_resource_references_changed();

	void remove_last_saved_json();
	void restore_last_saved_json();

	void force_autosave();
	void autosave_now_if_needed();
	bool autosave_needed() const;
	void save();
	void save_project_file_as(const augs::path_type& path);
	bool has_unsaved_changes() const;
	bool everything_completely_saved() const;

	void load_gui_state();
	void save_gui_state();
	void save_last_project_location();

	friend create_layer_command;
	friend delete_layers_command;

	template <class T>
	friend struct create_resource_command;

	friend delete_resources_command;

	template <class T>
	friend struct create_node_command;

	friend reorder_layers_command;

	friend move_nodes_command;
	friend resize_nodes_command;
	friend flip_nodes_command;

	friend delete_nodes_command;
	friend clone_nodes_command;

	friend edit_project_settings_command;

	friend editor_node_mover;

	cosmos& get_cosmos() {
		return scene.world;
	}

	void clamp_units();

	template <class H, class S>
	static decltype(auto) get_arena_handle_impl(S& self) {
		return H {
			self.current_mode_state,
			self.scene,
			self.scene.world,
			self.ruleset,
			self.clean_round_state
		};
	}

public:
	static constexpr auto loading_strategy = viewables_loading_type::LOAD_ALL;
	static constexpr bool handles_window_input = true;
	static constexpr bool has_additional_highlights = true;

	editor_setup(
		const editor_settings& settings,
		const packaged_official_content& official,
		const augs::path_type& project_path
	);
	
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

	std::optional<parent_layer_info> find_parent_layer(editor_node_id id) const;
	std::optional<parent_layer_info> convert_to_parent_layer_info(editor_layer_id id) const;

	std::size_t find_layer_index(editor_layer_id id) const;

	void sort_inspected();

	const std::vector<editor_layer_id>& get_layers() const {
		return project.layers.order;
	}

	editor_layer* find_layer(const editor_layer_id& id);
	const editor_layer* find_layer(const editor_layer_id& id) const;

	editor_layer* find_layer(const std::string& name);
	const editor_layer* find_layer(const std::string& name) const;
	void create_new_layer(const std::string& name_pattern = "Layer");

	std::string get_free_layer_name() const;
	std::string get_free_layer_name_for(const std::string& name_pattern) const;

	const auto& get_project() const { return project; }

	template <class T>
	decltype(auto) find_resource(const editor_typed_resource_id<T>& id);

	template <class T>
	decltype(auto) find_resource(const editor_typed_resource_id<T>& id) const;

	template <class F>
	decltype(auto) on_resource(const editor_resource_id& id, F&& callback);

	template <class F>
	decltype(auto) on_resource(const editor_resource_id& id, F&& callback) const;

	template <class F>
	decltype(auto) on_internal_resource(const editor_resource_id& id, F&& callback);

	template <class F>
	decltype(auto) on_internal_resource(const editor_resource_id& id, F&& callback) const;

	template <class T>
	decltype(auto) find_node(const editor_typed_node_id<T>& id);

	template <class T>
	decltype(auto) find_node(const editor_typed_node_id<T>& id) const;

	template <class F>
	decltype(auto) on_node(const editor_node_id& id, F&& callback);

	template <class F>
	decltype(auto) on_node(const editor_node_id& id, F&& callback) const;

	template <class R, class F>
	void for_each_resource(F&& callback, bool official) const;

	template <class N, class F>
	void for_each_node(F&& callback) const;

	template <class F>
	void for_each_dashed_line(F&&) const;

	template <class T, class F>
	void for_each_inspected(F&& callback) const {
		gui.inspector.for_each_inspected<T>(std::forward<F>(callback));
	}

	template <class F>
	void for_each_inspected_entity(F&& callback) const;

	std::unordered_set<entity_id> get_all_inspected_entities() const;
	void get_all_selected_by_selector(std::vector<entity_id>& into) const;
	void inspect_from_selector_state();

	void finish_rectangular_selection();
	void select_all_entities();

	void center_view_at_selection();
	void center_view_at(editor_node_id);
	void reset_zoom();
	bool is_view_centered_at_selection() const;

	template <class F>
	void for_each_highlight(F&& callback) const;

	editor_node_id to_node_id(entity_id) const;

	entity_id get_hovered_entity(const necessary_images_in_atlas_map& sizes_for_icons, std::optional<vec2> at = std::nullopt) const;
	editor_node_id get_hovered_node(const necessary_images_in_atlas_map& sizes_for_icons, std::optional<vec2> at = std::nullopt) const;
	editor_node_id get_hovered_node() const;

	template <class T>
	entity_id to_entity_id(const editor_typed_node_id<T> id) const {
		if (const auto node = find_node(id)) {
			return node->scene_entity_id;
		}

		return {};
	}

	entity_id to_entity_id(editor_node_id) const;

	void scroll_once_to(inspected_variant);

	name_to_node_map_type make_name_to_node_map() const;
	std::unordered_map<std::string, editor_resource_id> make_name_to_internal_resource_map() const;

	std::string get_free_node_name_for(const std::string& new_name) const;
	std::string get_free_internal_resource_name_for(const std::string& new_name) const;

	bool exists(const editor_resource_id&) const;

	void seek_to_revision(editor_history::index_type);
	bool wants_multiple_selection() const;

	template <class T>
	const T& post_new_command(T&&);

	template <class T>
	const T& rewrite_last_command(T&&);

	template <class T, class... Args>
	auto make_command_from_selected_entities(const std::string& preffix, Args&&...) const;

	template <class T, class... Args>
	auto make_command_from_selected_nodes(const std::string& preffix, Args&&...) const;

	template <class T, class... Args>
	auto make_command_from_selected_resources(const std::string& preffix, Args&&...) const;

	template <class T, class Node, class... Args>
	auto make_command_from_selected_typed_nodes(const std::string& preffix, Args&&...) const;

	template <class T, class Resource, class... Args>
	auto make_command_from_selected_typed_resources(const std::string& preffix, bool include_resources_from_selected_nodes, Args&&...) const;

	template <class T, class... Args>
	auto make_command_from_selected_layers(const std::string& preffix, Args&&...) const;

	void clear_inspector();

	bool register_node_in_layer(editor_node_id node, editor_node_id over_node);
	bool register_node_in_layer(editor_node_id node, editor_layer_id layer, std::size_t index);

	void unregister_node_from_layer(editor_node_id);
	void unregister_node_from_layer(editor_node_id, editor_layer_id);

	void inspect(const current_selections_type&);
	void inspect(const std::vector<entity_id>&);

	void inspect(inspected_variant);
	void inspect_add_quiet(inspected_variant);
	void quiet_set_last_inspected_layer_or_node(inspected_variant);
	void quiet_set_last_inspected_any(inspected_variant);

	template <class F>
	void inspect_erase_if(F&& callback) { 
		erase_if(gui.inspector.all_inspected, callback);
		after_quietly_adding_inspected();
	}

	void after_quietly_adding_inspected();

	void inspect_only(inspected_variant);
	void inspect_only(const std::vector<inspected_variant>&);
	void inspect_only(const std::vector<editor_node_id>&);
	void inspect_project_settings(bool scroll = true);
	void inspected_to_entity_selector_state();

	template <class T>
	bool inspects_any() const {
		return gui.inspector.template inspects_any<T>();
	}

	template <class T>
	bool inspects_only() const {
		return gui.inspector.template inspects_only<T>();
	}

	bool is_inspected(inspected_variant) const;
	std::vector<inspected_variant> get_all_inspected() const;

	template <class T>
	decltype(auto) get_all_inspected() const {
		return gui.inspector.template get_all_inspected<T>();
	}

	editor_node_id get_topmost_inspected_node() const;

	std::string get_name(inspected_variant) const;
	std::string get_name(entity_id) const;

	template <class N>
	std::string get_name(editor_typed_node_id<N> id) const {
		if (const auto node = find_node(id)) {
			return node->get_display_name();
		}

		return "";
	}

	template <class N>
	std::string get_name(editor_typed_resource_id<N> id) const {
		if (const auto resource = find_resource(id)) {
			return resource->get_display_name();
		}

		return "";
	}

	std::size_t get_node_count() const;
	std::size_t get_project_pathed_resource_count() const;

	editor_history::index_type get_last_command_index() const;

	editor_command_input make_command_input(bool skip_inspector = false); 
	entity_selector_input make_selector_input() const;
	node_mover_input make_mover_input();

	std::optional<rgba> find_highlight_color_of(const entity_id id) const;
	std::optional<ltrb> find_selection_aabb() const;

	template <class F>
	std::optional<ltrb> find_aabb_of_typed_nodes(F&& for_each_typed_node) const;

	template <class F>
	std::optional<ltrb> find_aabb_of_nodes(F&& for_each_node) const;

	std::optional<ltrb> find_screen_space_rect_selection(
		const vec2i screen_size,
		const vec2i mouse_pos
	) const;

	void undo();
	void undo_quiet();

	void redo();

	bool handle_doubleclick_in_layers_gui = false;

	void rebuild_arena(const bool editor_preview = true);

	const auto& get_paths() const {
		return paths;
	}

	augs::path_type resolve_project_path(const augs::path_type& path_in_project) const;

	camera_eye get_camera_eye() const;

	vec2 get_world_cursor_pos() const;
	vec2 get_world_cursor_pos(const camera_eye eye) const;

	template <class T>
	rgba get_icon_color_for(
		const T& object
	) const;
	
	template <class T>
	editor_icon_info get_icon_for(
		const T& object, 
		editor_icon_info_in
	) const;

	void unhover();
	void clear_id_caches();
	bool confirm_modal_popup();

	const auto& get_view() const {
		return view;
	}

	void start_renaming_selection();

	bool start_moving_selection();
	void finish_moving_selection();
	void show_absolute_mover_pos_once();

	void start_rotating_selection();
	void start_resizing_selection(bool two_edges, resize_nodes_command::active_edges custom_edges = {});
	void rotate_selection_once_by(int degrees);
	void flip_selection_horizontally();
	void flip_selection_vertically();
	void reset_rotation_of_selected_nodes();
	void set_zoom(float);

	void toggle_grid();
	void toggle_snapping();
	bool is_grid_enabled() const;
	bool is_snapping_enabled() const;

	void sparser_grid();
	void denser_grid();

	node_mover_op get_current_node_transforming_op() const;

	int get_current_grid_size() const;

	void make_last_command_a_child();

	bool is_next_command_child() const;
	bool is_last_command_child() const;

	void cut_selection();
	void delete_selection();

	bool is_node_active(const editor_node_id) const;
	auto only_active_nodes() const {
		return [this](const auto id) {
			return is_node_active(id);
		};
	}

	void mirror_selection(vec2i direction, bool move_if_only_clone = true);
	void clone_selection(bool start_moving = true);

	void move_dragged_to_new_layer(editor_node_id dragged_node);
	void move_inspected_to_new_layer();

	const auto& get_history() const {
		return history;
	}

	/*********************************************************/
	/*************** DEFAULT SETUP BOILERPLATE ***************/
	/*********************************************************/

	auto get_audiovisual_speed() const {
		return 1.0;
	}

	const auto& get_viewed_cosmos() const {
		return scene.world;
	}

	double get_interpolation_ratio() const;

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

	setup_escape_result escape();

	auto get_inv_tickrate() const {
		return get_viewed_cosmos().get_fixed_delta().in_seconds<double>();
	}

	template <class C>
	void advance(
		const setup_advance_input& in,
		const C& callbacks
	) {
		global_time_seconds += in.frame_delta.in_seconds();

		timer.advance(in.frame_delta);

		auto steps = timer.extract_num_of_logic_steps(get_inv_tickrate());

		while (steps--) {
			if (is_playtesting()) {
				const auto total = total_collected.extract(
					get_viewed_character(), 
					local_player_id, 
					in.make_accumulator_input()
				);

				get_arena_handle().on_mode_with_input(
					[&](auto& mode, const auto& input) {
						mode.advance(
							input,
							total,
							callbacks,
							solve_settings()
						);
					}
				);
			}
			else {
				animation_system().dry_advance_stateful_animations(scene.world);
			}
		}

		if (is_playtesting()) {
			global_time_seconds = scene.world.get_total_seconds_passed(get_interpolation_ratio());
		}

		(void)in;
		(void)callbacks;
	}

	void accept_game_gui_events(const game_gui_entropy_type&);

	std::optional<camera_eye> find_current_camera_eye() const;

	augs::path_type get_unofficial_content_dir() const;

	augs::maybe<render_layer_filter> get_render_layer_filter() const;

	void draw_custom_gui(const draw_setup_gui_input&);
	void draw_custom_gui_over_imgui(const draw_setup_gui_input&);
	void draw_recent_message(const draw_setup_gui_input&);

	void ensure_handler() {}

private:
	entropy_accumulator zero_entropy;
public:

	template <class T>
	void control(const T& t) {
		if (is_playtesting()) {
			total_collected.control(t);
		}
	}

	const entropy_accumulator& get_entropy_accumulator() const {
		if (is_playtesting()) {
			return total_collected;
		}

		return zero_entropy;
	}

	template <class F>
	void on_mode_with_input(F&&) const {}

	std::nullopt_t get_new_player_metas() {
		return std::nullopt;
	}

	const arena_player_metas* find_player_metas() const {
		return nullptr;
	}

	bool is_mover_active() const {
		return mover.is_active(history);
	}

	std::optional<parent_layer_info> find_best_layer_for_new_node() const;

	const auto& get_last_inspected_layer_or_node() const {
		return gui.inspector.get_last_inspected_layer_or_node();
	}

	const auto& get_last_inspected_any() const {
		return gui.inspector.get_last_inspected_any();
	}

	void set_inspector_tab(inspected_node_tab_type);
	void set_inspector_tab(inspected_project_tab_type);

	void start_playtesting();
	bool is_playtesting() const;

	/* Arena interface */
	bool is_gameplay_on() const { return is_playtesting(); }

	auto get_local_player_id() const {
		return local_player_id;
	}

	void stop_playtesting();

	bool can_undo() const;
	bool can_redo() const;

	bool can_resize_nodes() const;
	bool can_transform_nodes() const;

	void warp_cursor_to_center(augs::window&);
	bool can_resize_selected_nodes() const;
	bool should_warp_cursor_before_cloning() const;

	void toggle_sounds_preview();

	template <class F>
	void rebuild_prefab_nodes(editor_typed_node_id<editor_prefab_node>, F&& on_created_child, bool call_reverse = false);

	void unpack_prefab(editor_typed_node_id<editor_prefab_node>);

	const intercosm& get_built_official_content() const;
	const editor_resource_pools& get_official_resources() const;

private:
	editor_resource_pools& get_mut_official_resources();
public:

	const editor_official_resource_map& get_official_resource_map() const;

	editor_arena_handle<false> get_arena_handle();
	editor_arena_handle<true> get_arena_handle() const;

	bool is_dirty() const;

	void assign_project(const editor_project&, const bool undoing_to_first_revision);
	void on_project_assigned(const bool undoing_to_first_revision);

	arena_playtesting_context make_playtesting_context() const;

	template <class... Args>
	void set_recent_message(Args&&... args) { recent_message.set(std::forward<Args>(args)...); }

	std::string get_arena_name_with_star() const;
	std::string get_arena_name() const;

	void prepare_for_online_playtesting();
	void request_online_playtesting();

	const auto& get_scene() const {
		return scene;
	}

	void after_all_drawcalls(game_frame_buffer&);
	void do_game_main_thread_synced_op(renderer_backend_result&);

	void request_arena_screenshot(const augs::path_type& output_path, int max_size, bool reveal);
	bool is_generating_miniature() const;

	void rebuild_project_internal_resources_gui();
	void recount_internal_resource_references_if_needed();

	bool warp_cursor_once = false;
};