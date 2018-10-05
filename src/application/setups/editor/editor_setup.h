#pragma once
#include <future>
#include <map>

#include "augs/drawing/general_border.h"
#include "augs/templates/wrap_templates.h"

#include "game/assets/all_logical_assets.h"

#include "game/organization/all_component_includes.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/detail/visible_entities.h"

#include "view/necessary_image_id.h"
#include "view/necessary_resources.h"

#include "view/viewables/all_viewables_defs.h"
#include "view/viewables/viewables_loading_type.h"
#include "view/mode_gui/arena/arena_mode_gui.h"

#include "application/intercosm.h"

#include "application/debug_settings.h"

#include "application/setups/setup_common.h"

#include "application/setups/editor/editor_player.hpp"
#include "application/setups/editor/editor_popup.h"
#include "application/setups/editor/editor_significant.h"
#include "application/setups/editor/editor_autosave.h"
#include "application/setups/editor/editor_settings.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/editor_recent_paths.h"

#include "application/setups/editor/gui/editor_entity_selector.inl"
#include "application/setups/editor/gui/editor_entity_mover.h"

#include "application/setups/editor/editor_command_input.h"

#include "application/setups/editor/gui/editor_history_gui.h"
#include "application/setups/editor/gui/editor_go_to_gui.h"
#include "application/setups/editor/gui/editor_fae_gui.h"
#include "application/setups/editor/gui/editor_common_state_gui.h"
#include "application/setups/editor/gui/editor_selection_groups_gui.h"
#include "application/setups/editor/gui/editor_summary_gui.h"
#include "application/setups/editor/gui/editor_filters_gui.h"
#include "application/setups/editor/gui/editor_player_gui.h"
#include "application/setups/editor/gui/editor_modes_gui.h"

#include "application/setups/editor/gui/editor_pathed_asset_gui.h"
#include "application/setups/editor/gui/editor_unpathed_asset_gui.h"

#include "application/setups/editor/gui/for_each_iconed_entity.h"

#include "application/setups/editor/detail/current_access_cache.h"
#include "application/setups/editor/detail/make_command_from_selections.h"
#include "application/app_intent_type.h"

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
	editor_entity_selector selector;
	editor_entity_mover mover;

	editor_go_to_entity_gui go_to_entity_gui;

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
	editor_filters_gui filters_gui = std::string("Filters");

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
		catch (const editor_popup& p) {
			signi.folders.erase(signi.folders.begin() + new_index);
			set_popup(p);
		}

		base::refresh();
	}

	void enter_testing_mode();
	void quit_testing_mode();

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

public:
	using base::anything_opened;
	using base::folder;
	using base::view;
	using base::work;

	static constexpr auto loading_strategy = viewables_loading_type::LOAD_ALL;
	static constexpr bool handles_window_input = true;
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
		const images_in_atlas_map&,

		const config_lua_table& cfg
	);

	void customize_for_viewing(config_lua_table& cfg) const;
	void apply(const config_lua_table& cfg);

	template <class... Callbacks>
	void advance(
		augs::delta frame_delta,
		Callbacks&&... callbacks
	) {
		global_time_seconds += frame_delta.in_seconds();

		if (anything_opened()) {
			player().advance_player(
				frame_delta,
				folder().mode_vars,
				work().world,
				std::forward<Callbacks>(callbacks)...
			);
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
	bool is_gameplay_on() const;

	bool is_mover_active() const {
		return mover.is_active();
	}

	std::optional<camera_eye> find_current_camera_eye() const; 

	std::optional<ltrb> find_screen_space_rect_selection(vec2i screen_size, vec2i mouse_pos) const;

	const editor_view* find_view() const;

	template <class F>
	void for_each_selected_entity(F&& callback) const {
		if (anything_opened()) {
			selector.for_each_selected_entity(std::forward<F>(callback), view().ids.selected_entities);
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
		if (is_editing_mode()) {
			const auto& world = work().world;

			if (const auto handle = world[selector.get_hovered()]) {
				if (const auto tr = handle.find_logic_transform()) {
					/* Draw dashed lines around the selected entity */
					const auto ps = augs::make_rect_points<vec2>(handle.get_logical_size(), tr->pos, tr->rotation);

					for (std::size_t i = 0; i < ps.size(); ++i) {
						const auto& v = ps[i];
						const auto& nv = wrap_next(ps, i);

						callback(v, nv, settings.entity_selector.hovered_dashed_line_color, 0);
					}
				}
			}

			for_each_selected_entity(
				[&](const entity_id id) {
					const auto handle = world[id];

					handle.dispatch_on_having_all<invariants::light>([&](const auto typed_handle) {
						const auto center = typed_handle.get_logic_transform().pos;

						const auto& light_def = typed_handle.template get<invariants::light>();
						const auto& light = typed_handle.template get<components::light>();

						const auto light_color = light.color;

						auto draw_reach_indicator = [&](const auto reach, const auto col) {
							const auto h_size = vec2::square(reach);
							const auto size = vec2::square(reach * 2);

							callback(center, center + h_size, col);

							augs::general_border_from_to(
								ltrb(xywh::center_and_size(center, size)),
								0,
								[&](const vec2 from, const vec2 to) {
									callback(from, to, col);
								}
							);
						};

						draw_reach_indicator(light_def.calc_reach_trimmed(), light_color);
						draw_reach_indicator(light_def.calc_wall_reach_trimmed(), rgba(light_color).mult_alpha(0.7f));
					});

					if (mover.is_active()) {
						handle.dispatch_on_having_all<components::overridden_geo>([&](const auto typed_handle) {
							const auto s = typed_handle.get_logical_size();
							const auto tr = typed_handle.get_logic_transform();

							const auto& history = folder().history;
							const auto& last = history.last_command();

							if (const auto* const cmd = std::get_if<resize_entities_command>(std::addressof(last))) {
								const auto active = cmd->get_active_edges();
								const auto edges = ltrb::center_and_size(tr.pos, s).make_edges();

								auto draw_edge = [&](auto e) {
									callback(e[0].mult(tr), e[1].mult(tr), red, global_time_seconds * 8, true);
								};

								if (active.top) {
									draw_edge(edges[0]);
								}
								if (active.right) {
									draw_edge(edges[1]);
								}
								if (active.bottom) {
									draw_edge(edges[2]);
								}
								if (active.left) {
									draw_edge(edges[3]);
								}
							}
						});
					}
				}
			);
		}
	}

	template <class F>
	void for_each_icon(
		const visible_entities& entities, 
		const faction_view_settings& settings,
		F callback
	) const {
		if (is_editing_mode()) {
			const auto& world = work().world;

			::for_each_iconed_entity(
				world, 
				entities,
				settings,

				[&](auto&&... args) {
					callback(std::forward<decltype(args)>(args)...);
				}
			);
		}
	}

	std::optional<rgba> find_highlight_color_of(const entity_id id) const;
	std::optional<ltrb> find_selection_aabb() const;

	template <class F>
	void for_each_highlight(F callback) const {
		if (is_editing_mode()) {
			const auto& world = work().world;

			if (const auto viewed_character = get_viewed_character()) {
				auto color = settings.controlled_entity_color;
				color.a += static_cast<rgba_channel>(augs::zigzag(global_time_seconds, 1.0 / 2) * 25);

				callback(viewed_character.get_id(), color);
			}

			selector.for_each_highlight(
				callback,
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

	void hide_layers_of_selected_entities();
	void unhide_all_layers();

	void ensure_handler();
};
