#pragma once
#include <future>

#include "augs/templates/wrap_templates.h"

#include "game/assets/all_logical_assets.h"
#include "game/detail/visible_entities.h"

#include "view/necessary_image_id.h"
#include "view/necessary_resources.h"

#include "view/viewables/all_viewables_defs.h"
#include "view/viewables/viewables_loading_type.h"
#include "view/mode_gui/arena/arena_mode_gui.h"

#include "application/intercosm.h"
#include "game/cosmos/entity_handle.h"
#include "application/debug_settings.h"

#include "application/setups/setup_common.h"

#include "application/setups/editor/editor_popup.h"
#include "application/setups/editor/editor_significant.h"
#include "application/setups/editor/editor_autosave.h"
#include "application/setups/editor/editor_settings.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/editor_recent_paths.h"

#include "application/setups/editor/gui/editor_entity_selector.hpp"
#include "application/setups/editor/gui/editor_entity_mover.h"

#include "application/setups/editor/editor_command_input.h"

#include "application/setups/editor/gui/editor_history_gui.h"
#include "application/setups/editor/gui/editor_go_to_gui.h"
#include "application/setups/editor/gui/editor_fae_gui.h"
#include "application/setups/editor/gui/editor_common_state_gui.h"
#include "application/setups/editor/gui/editor_selection_groups_gui.h"
#include "application/setups/editor/gui/editor_summary_gui.h"
#include "application/setups/editor/gui/editor_layers_gui.h"
#include "application/setups/editor/gui/editor_player_gui.h"
#include "application/setups/editor/gui/editor_modes_gui.h"
#include "application/setups/editor/gui/editor_tutorial_gui.h"

#include "application/setups/editor/gui/editor_pathed_asset_gui.h"
#include "application/setups/editor/gui/editor_unpathed_asset_gui.h"

#include "application/app_intent_type.h"
#include "application/setups/editor/editor_recent_message.h"

#include "application/input/entropy_accumulator.h"

struct config_lua_table;
struct draw_setup_gui_input;

namespace augs {
	class window;
	struct introspection_access;

	namespace event {
		struct change;
		struct state;
	}
}

struct editor_destructor_input {
	sol::state& lua;
};

class images_in_atlas_map;

struct intercosm;

class editor_setup {
	friend augs::introspection_access;

	/* These two friends for handy printing of internal state */
	friend editor_tutorial_gui;
	friend editor_summary_gui;
	friend editor_coordinates_gui;

	double global_time_seconds = 0.0;

	entropy_accumulator total_collected;

	editor_significant signi;
	editor_autosave autosave;
	editor_recent_paths recent;
	editor_settings settings;
	editor_entity_selector selector;
	editor_entity_mover mover;

	editor_go_to_entity_gui go_to_entity_gui;
	editor_recent_message recent_message;

	// GEN INTROSPECTOR class editor_setup
	editor_player_gui player_gui = std::string("Player");
	editor_modes_gui modes_gui = std::string("Modes");
	editor_history_gui history_gui = std::string("History");
	editor_fae_gui fae_gui = std::string("Scene hierarchy");
	editor_selected_fae_gui selected_fae_gui = std::string("Selection hierarchy");
	editor_common_state_gui common_state_gui = std::string("Common state");
	editor_selection_groups_gui selection_groups_gui = std::string("Selection groups");
	editor_summary_gui summary_gui = std::string("Summary");
	editor_coordinates_gui coordinates_gui = std::string("Coordinates");
	editor_layers_gui layers_gui = std::string("Layers");
	editor_tutorial_gui tutorial_gui = std::string("Editor tutorial");
	imgui_tutorial_gui imgui_tutorial = std::string("ImGui tutorial");

	editor_images_gui images_gui = std::string("Images");
	editor_sounds_gui sounds_gui = std::string("Sounds");

	editor_plain_animations_gui plain_animations_gui = std::string("Animations");
	editor_particle_effects_gui particle_effects_gui = std::string("Particle effects");
	// END GEN INTROSPECTOR

	arena_gui_state arena_gui;

	std::optional<editor_popup> ok_only_popup;

	editor_destructor_input destructor_input;

	const_entity_handle get_matching_go_to_entity() const;

	void on_folder_changed();
	void override_viewed_entity(const entity_id);

	void clear_id_caches();

	template <class F>
	void catch_popup(F&& callback) {
		try {
			callback();
		}
		catch (const editor_popup& p) {
			set_popup(p);
		}
	}

	template <class F>
	void try_to_open_new_folder(F&& new_folder_provider);

	std::future<std::optional<std::string>> open_folder_dialog;
	std::future<std::optional<std::string>> save_folder_dialog;

	std::future<std::optional<std::string>> export_folder_dialog;

	void set_popup(const editor_popup);
	
	using path_operation = intercosm_path_op;

	void open_folder_in_new_tab(path_operation);

	bool save_current_folder();
	bool save_current_folder_to(path_operation);
	void export_current_folder_to(path_operation);

	void fill_with_minimal_scene();
	void fill_with_test_scene();

	void open_last_folders(sol::state& lua);

	void force_autosave_now() const;

	void load_gui_state();
	void save_gui_state();

	void cut_selection();
	void delete_selection();

	void mirror_selection(vec2i direction);
	void duplicate_selection();

	void group_selection();
	void ungroup_selection();

	void make_last_command_a_child();

	void center_view_at_selection();

	auto make_for_each_selected_entity() const {
		return [this](auto callback) {
			for_each_selected_entity(callback);	
		};
	}

	void draw_mode_gui(const draw_setup_gui_input&);

	template <class F>
	void on_mode_with_input(F&& callback) const;

	float get_menu_bar_height() const;
	float get_game_screen_top() const;

	void set_current(const folder_index i);

	std::size_t find_folder_by_path(const augs::path_type& current_path) const;

public:
	static constexpr auto loading_strategy = viewables_loading_type::LOAD_ALL;
	static constexpr bool handles_window_input = true;
	static constexpr bool has_additional_highlights = true;

	editor_setup(sol::state& lua);
	editor_setup(sol::state& lua, const augs::path_type& intercosm_path);
	
	~editor_setup();

	double get_audiovisual_speed() const;
	double get_inv_tickrate() const;
	const cosmos& get_viewed_cosmos() const;
	double get_interpolation_ratio() const;
	entity_id get_viewed_character_id() const;
	const_entity_handle get_viewed_character() const;
	const all_viewables_defs& get_viewable_defs() const;

	void perform_custom_imgui(perform_custom_imgui_input);

	void customize_for_viewing(config_lua_table& cfg) const;
	void apply(const config_lua_table& cfg);

	template <class C>
	void advance(
		const setup_advance_input in,
		const C& callbacks
	) {
		global_time_seconds += in.frame_delta.in_seconds();

		if (anything_opened()) {
			player().advance_player(
				in.frame_delta,
				make_player_input(callbacks),
				[&]() {
					return total_collected.extract(
						get_viewed_character(), 
						view().local_player_id, 
						{ in.settings, in.screen_size }
					);
				}
			);
		}
	}

	template <class T>
	void control(const T& t) {
		if (anything_opened()) {
			if (player().is_recording()) {
				total_collected.control(t);
			}
		}
	}

	void accept_game_gui_events(const game_gui_entropy_type&);

	bool handle_input_before_imgui(
		const augs::event::state& common_input_state,
		const augs::event::change change,

		augs::window& window
	);

	bool handle_input_before_game(
		const app_ingame_intent_map& app_controls,
		const necessary_images_in_atlas_map& sizes_for_icons,

		const augs::event::state& common_input_state,
		const augs::event::change change,

		augs::window& window
	);

	setup_escape_result escape();
	bool confirm_modal_popup();

	void open(const augs::window& owner);
	void save(const augs::window& owner);
	void save_as(const augs::window& owner);
	void export_for_compatibility(const augs::window& owner);
	void undo();
	void redo();

	void copy();
	void cut();
	void paste();

	void new_tab();
	void next_tab();
	void prev_tab();

	bool close_folder();
	bool close_folder(const folder_index i);

	void go_to_all();
	void go_to_entity();
	void select_all_entities(bool has_ctrl);
	void reveal_in_explorer(const augs::window& owner);

	void finish_rectangular_selection();
	void unhover();
	bool is_editing_mode() const;
	bool is_gameplay_on() const;

	bool is_mover_active() const {
		return mover.is_active(folder().history);
	}

	std::optional<camera_eye> find_current_camera_eye() const; 

	std::optional<ltrb> find_screen_space_rect_selection(vec2i screen_size, vec2i mouse_pos) const;

	const editor_view* find_view() const;

	template <class F>
	void for_each_selected_entity(F&& callback) const {
		if (anything_opened()) {
			selector.for_each_selected_entity(std::forward<F>(callback), view_ids().selected_entities);
		}
	}

	editor_command_input make_command_input();
	grouped_selector_op_input make_grouped_selector_op_input() const;

	editor_fae_gui_input make_fae_gui_input();

	entity_mover_input make_mover_input();

	template <class C>
	auto make_player_input(const C& callbacks) {
		return player_advance_input(make_command_input(), callbacks);
	}

	template <class T, class... Args>
	auto make_command_from_selections(Args&&...) const;

	std::unordered_set<entity_id> get_all_selected_entities() const;

	template <class F>
	void for_each_line(F) const {}

	template <class F>
	void for_each_dashed_line(F&&) const;

	template <class F>
	void for_each_icon(
		const visible_entities& entities, 
		const faction_view_settings& settings,
		F&& callback
	) const;

	std::optional<rgba> find_highlight_color_of(const entity_id id) const;
	std::optional<ltrb> find_selection_aabb() const;

	template <class F>
	void for_each_highlight(F&& callback) const {
		if (is_editing_mode()) {
			const auto& world = work().world;

			if (const auto viewed_character = get_viewed_character()) {
				auto color = settings.controlled_entity_color;
				color.a += static_cast<rgba_channel>(augs::zigzag(global_time_seconds, 1.0 / 2) * 25);

				callback(viewed_character.get_id(), color);
			}

			selector.for_each_highlight(
				std::forward<F>(callback),
				settings.entity_selector,
				world,
				make_grouped_selector_op_input()
			);

			if (const auto hovered_guid = fae_gui.get_hovered_guid()) {
				if (const auto hovered = world[hovered_guid]) {
					/* Hovering from GUI, so choose the stronger, held color for it */
					callback(hovered.get_id(), settings.entity_selector.held_color);
				}
			}

			if (const auto hovered_guid = selected_fae_gui.get_hovered_guid()) {
				if (const auto hovered = world[hovered_guid]) {
					/* Hovering from GUI, so choose the stronger, held color for it */
					callback(hovered.get_id(), settings.entity_selector.held_color);
				}
			}

			if (const auto match = get_matching_go_to_entity()) {
				auto color = green;
				color.a += static_cast<rgba_channel>(augs::zigzag(global_time_seconds, 1.0 / 2) * 25);
				
				callback(match.get_id(), settings.matched_entity_color);
			}
		}
	}

	std::optional<vec2> find_world_cursor_pos() const;
	vec2 get_world_cursor_pos(const camera_eye) const;

	augs::path_type get_unofficial_content_dir() const;

	augs::maybe<render_layer_filter> get_render_layer_filter() const;

	void draw_custom_gui(const draw_setup_gui_input&);

	void draw_status_bar(const draw_setup_gui_input&);
	void draw_recent_message(const draw_setup_gui_input&);
	void draw_marks_gui(const draw_setup_gui_input&);

	void hide_layers_of_selected_entities();
	void unhide_all_layers();

	void ensure_handler();

	bool anything_opened() const;

	editor_folder& folder();
	const editor_folder& folder() const;

	intercosm& work();
	const intercosm& work() const;

	editor_player& player();
	const editor_player& player() const;

	editor_view& view();
	const editor_view& view() const;

	editor_view_ids& view_ids();
	const editor_view_ids& view_ids() const;

	void begin_recording();
	void begin_replaying();

	void finish_and_discard();
	void finish_and_reapply();
};
