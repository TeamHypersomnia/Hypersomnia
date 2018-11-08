#pragma once
#include <optional>
#include "augs/misc/timing/fixed_delta_timer.h"
#include "augs/math/camera_cone.h"

#include "game/detail/render_layer_filter.h"
#include "game/assets/all_logical_assets.h"
#include "game/organization/all_component_includes.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/modes/test_scene_mode.h"

#include "view/viewables/all_viewables_defs.h"
#include "test_scenes/test_scene_settings.h"

#include "application/intercosm.h"

#include "application/setups/default_setup_settings.h"

#include "application/debug_settings.h"
#include "application/input/entropy_accumulator.h"
#include "application/setups/setup_common.h"

struct config_lua_table;
struct draw_setup_gui_input;

namespace sol {
	class state;
}

class test_scene_setup : public default_setup_settings {
	test_scene_mode mode;
	test_scene_mode_rules mode_rules;

	intercosm scene;
	entropy_accumulator total_collected;
	augs::fixed_delta_timer timer = { 5, augs::lag_spike_handling_type::DISCARD };
	entity_id viewed_character_id;

public:
	test_scene_setup(
		sol::state& lua,
		const test_scene_settings,
		const input_recording_type recording_type
	);

	auto get_audiovisual_speed() const {
		return 1.0;
	}

	const auto& get_viewed_cosmos() const {
		return scene.world;
	}

	auto get_interpolation_ratio() const {
		return timer.fraction_of_step_until_next_step(get_viewed_cosmos().get_fixed_delta());
	}

	auto get_viewed_character_id() const {
		return viewed_character_id;
	}

	auto get_viewed_character() const {
		return get_viewed_cosmos()[get_viewed_character_id()];
	}

	const auto& get_viewable_defs() const {
		return scene.viewables;
	}

	void perform_custom_imgui() {
		return;
	}

	void customize_for_viewing(config_lua_table&) const;

	void apply(const config_lua_table&) {
		return;
	}

	auto escape() {
		return setup_escape_result::IGNORE;
	}

	template <class C>
	void advance(
		const setup_advance_input& in,
		const C& callbacks
	) {
		timer.advance(in.frame_delta);
		auto steps = timer.extract_num_of_logic_steps(get_viewed_cosmos().get_fixed_delta());

		while (steps--) {
			const auto total = total_collected.extract(get_viewed_character(), in);

			mode.advance(
				{ mode_rules, scene.world },
				total,
				callbacks
			);
		}
	}

	template <class T>
	void control(const T& t) {
		total_collected.control(t);
	}

	void accept_game_gui_events(const cosmic_entropy&);

	std::optional<camera_eye> find_current_camera_eye() const {
		return std::nullopt;
	}

	augs::path_type get_unofficial_content_dir() const {
		return {};
	}

	auto get_render_layer_filter() const {
		return render_layer_filter::disabled();
	}

	void draw_custom_gui(const draw_setup_gui_input&) {}

	void ensure_handler() {}
};