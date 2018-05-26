#pragma once
#include <future>
#include <map>

#include "augs/misc/timing/fixed_delta_timer.h"
#include "augs/drawing/general_border.h"

#include "game/assets/all_logical_assets.h"

#include "game/organization/all_component_includes.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/detail/visible_entities.h"

#include "view/necessary_image_id.h"
#include "view/necessary_resources.h"

#include "view/viewables/all_viewables_defs.h"
#include "view/viewables/viewables_loading_type.h"

#include "application/intercosm.h"

#include "application/debug_settings.h"

#include "application/setups/setup_common.h"

#include "application/setups/editor/editor_player.h"
#include "application/setups/editor/editor_popup.h"
#include "application/setups/editor/editor_significant.h"
#include "application/setups/editor/editor_autosave.h"
#include "application/setups/editor/editor_settings.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/editor_recent_paths.h"

#include "application/setups/editor/gui/editor_entity_selector.h"
#include "application/setups/editor/gui/editor_entity_mover.h"

#include "application/setups/editor/editor_command_input.h"

#include "application/setups/editor/gui/editor_history_gui.h"
#include "application/setups/editor/gui/editor_go_to_gui.h"
#include "application/setups/editor/gui/editor_fae_gui.h"
#include "application/setups/editor/gui/editor_common_state_gui.h"
#include "application/setups/editor/gui/editor_selection_groups_gui.h"
#include "application/setups/editor/gui/editor_summary_gui.h"

#include "application/setups/editor/gui/editor_pathed_asset_gui.h"
#include "application/setups/editor/gui/editor_unpathed_asset_gui.h"

#include "application/setups/editor/detail/current_access_cache.h"
#include "application/setups/editor/detail/make_command_from_selections.h"

struct config_lua_table;

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

class editor_setup : private current_access_cache<editor_setup> {
	using base = current_access_cache<editor_setup>;
	friend base;
	friend augs::introspection_access;

	/* These two friends for handy printing of internal state */
	friend editor_summary_gui;
	friend editor_coordinates_gui;

	double global_time_seconds = 0.0;

	editor_significant signi;
	editor_autosave autosave;
	editor_recent_paths recent;
	editor_settings settings;
	editor_player player;
	editor_entity_selector selector;
	editor_entity_mover mover;

	editor_go_to_entity_gui go_to_entity_gui;

	// GEN INTROSPECTOR class editor_setup
	editor_history_gui history_gui = std::string("History");
	editor_fae_gui fae_gui = std::string("Scene hierarchy");
	editor_selected_fae_gui selected_fae_gui = std::string("Selection hierarchy");
	editor_common_state_gui common_state_gui = std::string("Common state");
	editor_selection_groups_gui selection_groups_gui = std::string("Selection groups");
	editor_summary_gui summary_gui = std::string("Summary");
	editor_coordinates_gui coordinates_gui = std::string("Coordinates");

	editor_images_gui images_gui = std::string("Images");
	editor_sounds_gui sounds_gui = std::string("Sounds");

	editor_plain_animations_gui plain_animations_gui = std::string("Plain animations");
	editor_torso_animations_gui torso_animations_gui = std::string("Torso animations");
	editor_legs_animations_gui legs_animations_gui = std::string("Legs animations");
	// END GEN INTROSPECTOR

	std::optional<editor_popup> ok_only_popup;

	editor_destructor_input destructor_input;

	const_entity_handle get_matching_go_to_entity() const;

	void on_folder_changed();
	void set_locally_viewed(const entity_id);

	void clear_id_caches();

	template <class F>
	void catch_popup(F&& callback) {
		try {
			callback();
		}
		catch (editor_popup p) {
			set_popup(p);
		}
	}

	template <class F>
	void try_to_open_new_folder(F&& new_folder_provider) {
		const auto new_index = signi.current_index + 1;

		signi.folders.reserve(signi.folders.size() + 1);
		signi.folders.emplace(signi.folders.begin() + new_index);

		auto& new_folder = signi.folders[new_index];

		base::refresh();

		try {
			new_folder_provider(new_folder);
			set_current(new_index);
		}
		catch (editor_popup p) {
			signi.folders.erase(signi.folders.begin() + new_index);
			set_popup(p);
		}

		base::refresh();
	}

	void play();
	void pause();

	cosmic_entropy total_collected_entropy;
	augs::fixed_delta_timer timer = { 5, augs::lag_spike_handling_type::DISCARD };

	std::future<std::optional<std::string>> open_folder_dialog;
	std::future<std::optional<std::string>> save_project_dialog;

	void set_popup(const editor_popup);
	
	using path_operation = intercosm_path_op;

	void open_folder_in_new_tab(path_operation);

	void save_current_folder();
	void save_current_folder_to(path_operation);

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
	void reset_zoom();

	auto make_for_each_selected_entity() const {
		return [this](auto callback) {
			for_each_selected_entity(callback);	
		};
	}

public:
	using base::anything_opened;
	using base::folder;
	using base::view;
	using base::work;

	static constexpr auto loading_strategy = viewables_loading_type::LOAD_ALL;
	static constexpr bool handles_window_input = true;
	static constexpr bool handles_escape = true;
	static constexpr bool has_additional_highlights = true;

	editor_setup(sol::state& lua);
	editor_setup(sol::state& lua, const augs::path_type& intercosm_path);
	
	~editor_setup();

	double get_audiovisual_speed() const;
	const cosmos& get_viewed_cosmos() const;
	real32 get_interpolation_ratio() const;
	entity_id get_viewed_character_id() const;
	const_entity_handle get_viewed_character() const;
	const all_viewables_defs& get_viewable_defs() const;

	void perform_custom_imgui(
		sol::state& lua,
		augs::window& owner,
		bool in_direct_gameplay,
		const images_in_atlas_map&
	);

	void customize_for_viewing(config_lua_table& cfg) const;
	void apply(const config_lua_table& cfg);

	template <class... Callbacks>
	void advance(
		augs::delta frame_delta,
		Callbacks&&... callbacks
	) {
		global_time_seconds += frame_delta.in_seconds();

		if (!player.paused) {
			timer.advance(frame_delta *= player.speed);
		}

		auto steps = timer.extract_num_of_logic_steps(get_viewed_cosmos().get_fixed_delta());

		while (steps--) {
			total_collected_entropy.clear_dead_entities(work().world);

			work().advance(
				{ total_collected_entropy },
				std::forward<Callbacks>(callbacks)...
			);

			total_collected_entropy.clear();
		}
	}

	void control(const cosmic_entropy&);
	void accept_game_gui_events(const cosmic_entropy&);

	bool handle_input_before_imgui(
		const augs::event::state& common_input_state,
		const augs::event::change change,

		augs::window& window
	);

	bool handle_input_before_game(
		const necessary_images_in_atlas_map& sizes_for_icons,

		const augs::event::state& common_input_state,
		const augs::event::change change,

		augs::window& window
	);

	std::optional<setup_escape_result> escape();
	bool confirm_modal_popup();

	void open(const augs::window& owner);
	void save(const augs::window& owner);
	void save_as(const augs::window& owner);
	void undo();
	void redo();

	void copy();
	void cut();
	void paste();

	void play_pause();
	void stop();
	void prev();
	void next();

	void new_tab();
	void next_tab();
	void prev_tab();

	void close_folder();
	void close_folder(const folder_index i);

	void go_to_all();
	void go_to_entity();
	void select_all_entities(bool has_ctrl);
	void reveal_in_explorer(const augs::window& owner);

	void finish_rectangular_selection();
	void unhover();
	bool is_editing_mode() const;
	std::optional<camera_cone> find_current_camera() const; 

	std::optional<ltrb> find_screen_space_rect_selection(vec2i screen_size, vec2i mouse_pos) const;

	const editor_view* find_view() const;

	template <class F>
	void for_each_selected_entity(F&& callback) const {
		if (anything_opened()) {
			selector.for_each_selected_entity(std::forward<F>(callback), view().selected_entities);
		}
	}

	editor_command_input make_command_input();
	grouped_selector_op_input make_grouped_selector_op_input() const;

	editor_fae_gui_input make_fae_gui_input();

	entity_mover_input make_mover_input();

	template <class T, class... Args>
	auto make_command_from_selections(Args&&... args) const {
		return ::make_command_from_selections<T>(
			make_for_each_selected_entity(),
			work().world,
			std::forward<Args>(args)...
		);
	}

	std::unordered_set<entity_id> get_all_selected_entities() const;

	template <class F>
	void for_each_line(F) const {

	}

	template <class F>
	void for_each_dashed_line(F callback) const {
		if (anything_opened()) {
			const auto& world = work().world;

			if (player.paused) {
				for_each_selected_entity(
					[&](const entity_id id) {
						const auto handle = world[id];

						handle.dispatch_on_having<invariants::light>([&](const auto typed_handle) {
							const auto max_distance = typed_handle.template get<invariants::light>().get_max_distance();
							const auto center = typed_handle.get_logic_transform().pos;

							const auto light_color = typed_handle.template get<components::light>().color;

							const auto reach = vec2(max_distance, max_distance);
							callback(center, center + reach, light_color);

							augs::general_border_from_to(
								ltrb(xywh::center_and_size(center, reach * 2)),
								0,
								[&](const vec2 from, const vec2 to) {
									callback(from, to, light_color);
								}
							);
						});
					}
				);
			}
		}
	}

	template <class F>
	void for_each_icon(F callback) const {
		if (anything_opened() && player.paused) {
			const auto& world = work().world;

			::for_each_iconed_entity(world, [&](auto&&... args) {
				callback(std::forward<decltype(args)>(args)...);
			});
		}
	}

	std::optional<rgba> find_highlight_color_of(const entity_id id) const;
	std::optional<ltrb> find_selection_aabb() const;

	template <class F>
	void for_each_highlight(F callback) const {
		if (anything_opened() && player.paused) {
			const auto& world = work().world;

			if (get_viewed_character().alive()) {
				auto color = settings.controlled_entity_color;
				color.a += static_cast<rgba_channel>(augs::zigzag(global_time_seconds, 1.0 / 2) * 25);

				callback(work().local_test_subject, color);
			}

			selector.for_each_highlight(
				callback,
				settings.entity_selector,
				world,
				make_grouped_selector_op_input()
			);

			if (const auto match = get_matching_go_to_entity()) {
				auto color = green;
				color.a += static_cast<rgba_channel>(augs::zigzag(global_time_seconds, 1.0 / 2) * 25);
				
				callback(match.get_id(), settings.matched_entity_color);
			}

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
		}
	}

	vec2 get_world_cursor_pos() const;
	vec2 get_world_cursor_pos(const camera_cone) const;

	augs::path_type get_unofficial_content_dir() const;
};
