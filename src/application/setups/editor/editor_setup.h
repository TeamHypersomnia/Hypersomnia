#pragma once
#include <future>
#include <map>

#include "augs/misc/timing/fixed_delta_timer.h"

#include "game/assets/all_logical_assets.h"

#include "game/organization/all_component_includes.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/detail/visible_entities.h"

#include "view/viewables/all_viewables_defs.h"
#include "view/viewables/viewables_loading_type.h"

#include "application/intercosm.h"

#include "application/debug_settings.h"

#include "application/setups/editor/editor_significant.h"
#include "application/setups/editor/editor_autosave.h"
#include "application/setups/editor/editor_settings.h"
#include "application/setups/editor/editor_tab.h"
#include "application/setups/editor/editor_recent_paths.h"
#include "application/setups/editor/current_tab_access_cache.h"

struct config_lua_table;

namespace augs {
	class window;

	namespace event {
		struct change;
		struct state;
	}
}

struct editor_player {
	bool show = true;
	bool paused = true;
	double speed = 1.0;

	auto get_speed() const {
		return paused ? 0.0 : speed;
	}
};

struct editor_destructor_input {
	sol::state& lua;
};

class editor_setup : private current_tab_access_cache<editor_setup> {
	using base = current_tab_access_cache<editor_setup>;
	friend base;

	editor_significant signi;
	editor_autosave autosave;
	editor_recent_paths recent;
	editor_settings settings;
	editor_player player;

	editor_destructor_input destructor_input;

	double global_time_seconds = 0.0;

	std::optional<editor_popup> current_popup;

	bool show_summary = true;
	bool show_common_state = false;
	bool show_entities = false;
	bool show_go_to_all = false;
	bool show_go_to_entity = false;
	std::string go_to_entity_query;

	std::string last_go_to_entities_input;
	unsigned go_to_entities_selected_index = 0;
	std::vector<entity_guid> matching_go_to_entities;

	const_entity_handle get_matching_go_to_entity() const;
	
	entity_id hovered_entity;
	entity_id held_entity;
	vec2 last_ldown_position;
	visible_entities in_rectangular_selection;

	void on_tab_changed();
	void set_locally_viewed(const entity_id);
	void finish_rectangular_selection();

	template <class F>
	bool try_to_open_new_tab(F&& new_intercosm_provider) {
		const auto new_index = has_current_tab() ? signi.current_index + 1 : 0;

		signi.works.reserve(signi.works.size() + 1);
		signi.tabs.reserve(signi.tabs.size() + 1);

		signi.tabs.emplace(signi.tabs.begin() + new_index);
		signi.works.emplace(signi.works.begin() + new_index, std::make_unique<intercosm>());
		base::refresh();

		if (/* successfully_opened */ new_intercosm_provider(signi.tabs[new_index], *signi.works[new_index])) {
			set_current_tab(new_index);
			return true;
		}

		signi.tabs.erase(signi.tabs.begin() + new_index);
		signi.works.erase(signi.works.begin() + new_index);

		return false;
	}

	void play();
	void pause();

	cosmic_entropy total_collected_entropy;
	augs::fixed_delta_timer timer = { 5, augs::lag_spike_handling_type::DISCARD };

	std::future<std::optional<std::string>> open_file_dialog;
	std::future<std::optional<std::string>> save_file_dialog;

	void set_if_popup(const std::optional<editor_popup> p); 
	void set_popup(const editor_popup);
	
	using path_operation = intercosm_path_op;

	bool open_intercosm_in_new_tab(path_operation);
	void save_current_tab_to(path_operation);

	void fill_with_minimal_scene(sol::state& lua);
	void fill_with_test_scene(sol::state& lua);

	void open_last_tabs(sol::state& lua);

	void clear_all_selections();

public:
	static constexpr auto loading_strategy = viewables_loading_type::LOAD_ALL;
	static constexpr bool handles_window_input = true;
	static constexpr bool handles_escape = true;
	static constexpr bool has_additional_highlights = true;

	std::optional<vec2> rectangular_drag_origin;

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
		const bool in_direct_gameplay,
		const camera_cone current_cone
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

		augs::window& window,
		sol::state& lua
	);

	bool handle_unfetched_window_input(
		const augs::event::state& common_input_state,
		const augs::event::change change,

		augs::window& window,
		sol::state& lua,

		const camera_cone current_cone
	);

	bool escape();
	bool confirm_modal_popup();

	void open(const augs::window& owner);
	void save(sol::state& lua, const augs::window& owner);
	void save_as(const augs::window& owner);
	void undo();
	void redo();

	void copy();
	void cut();
	void paste();

	void del();

	void play_pause();
	void stop();
	void prev();
	void next();

	void new_tab();
	void next_tab();
	void prev_tab();
	void close_tab();
	void close_tab(const tab_index_type i);

	void go_to_all();
	void go_to_entity();
	void reveal_in_explorer();

	void unhover();
	bool is_normal_mode() const;
	std::optional<camera_cone> get_custom_camera() const; 

	template <class F>
	void for_each_selected_entity(F callback) const {
		if (has_current_tab()) {
			for (const auto e : tab().selected_entities) {
				if (!found_in(in_rectangular_selection.all, e)) {
					callback(e);
				}
			}

			for (const auto e : in_rectangular_selection.all) {
				if (!found_in(tab().selected_entities, e)) {
					callback(e);
				}
			}
		}
	}

	template <class F>
	void for_each_line(F callback) const {

	}

	template <class F>
	void for_each_highlight(F callback) const {
		if (has_current_tab() && player.paused) {
			if (get_viewed_character().alive()) {
				auto color = settings.controlled_entity_color;
				color.a += static_cast<rgba_channel>(augs::zigzag(global_time_seconds, 1.0 / 2) * 25);

				callback(work().locally_viewed, color);
			}

			for_each_selected_entity(
				[&](const auto e) {
					callback(e, settings.selected_entity_color);
				}
			);

			if (work().world[held_entity].alive()) {
				callback(held_entity, settings.held_entity_color);
			}

			if (work().world[hovered_entity].alive()) {
				callback(hovered_entity, settings.hovered_entity_color);
			}
			
			if (const auto match = get_matching_go_to_entity()) {
				auto color = green;
				color.a += static_cast<rgba_channel>(augs::zigzag(global_time_seconds, 1.0 / 2) * 25);
				
				callback(match.get_id(), settings.matched_entity_color);
			}
		}
	}
};
