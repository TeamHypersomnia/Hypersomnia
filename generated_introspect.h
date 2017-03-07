#include "game/transcendental/types_specification/all_components_declaration.h"
#include "game/components/behaviour_tree_component.h"
#include "game/components/car_component.h"
#include "augs/image/image.h"
class config_lua_table;

#define NVP(x) x, #x

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, augs::image::paint_circle_midpoint_command> t,
		F f
	) {
		f(t.NVP(radius));
		f(t.NVP(border_width));
		f(t.NVP(scale_alpha));
		f(t.NVP(constrain_angle));
		f(t.NVP(angle_start));
		f(t.NVP(angle_end));
		f(t.NVP(filling));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, augs::image::paint_circle_filled_command> t,
		F f
	) {
		f(t.NVP(radius));
		f(t.NVP(filling));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, augs::image::paint_line_command> t,
		F f
	) {
		f(t.NVP(from));
		f(t.NVP(to));
		f(t.NVP(filling));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, components::animation> t,
		F f
	) {
		f(t.NVP(current_animation));

		f(t.NVP(priority));
		f(t.NVP(frame_num));
		f(t.NVP(player_position_ms));
		f(t.NVP(speed_factor));

		f(t.NVP(state));
		f(t.NVP(paused_state));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, components::animation_response> t,
		F f
	) {
		f(t.NVP(response));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, components::attitude> t,
		F f
	) {
		f(t.NVP(maximum_divergence_angle_before_shooting));

		f(t.NVP(parties));
		f(t.NVP(hostile_parties));

		f(t.NVP(specific_hostile_entities));
		
		f(t.NVP(currently_attacked_visible_entity));
		f(t.NVP(target_attitude));

		f(t.NVP(is_alert));
		f(t.NVP(last_seen_target_position_inspected));

		f(t.NVP(last_seen_target_position));
		f(t.NVP(last_seen_target_velocity));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, components::behaviour_tree::instance> t,
		F f
	) {
		f(t.NVP(state));
		f(t.NVP(tree_id));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, components::behaviour_tree> t,
		F f
	) {
		f(t.NVP(concurrent_trees));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, components::car::engine_entities> t,
		F f
	) {
		f(t.NVP(physical));
		f(t.NVP(particles));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, components::car> t,
		F f
	) {
		f(t.NVP(current_driver));

		f(t.NVP(interior));

		f(t.NVP(left_wheel_trigger));
		f(t.NVP(right_wheel_trigger));

		f(t.NVP(acceleration_engine));
		f(t.NVP(deceleration_engine));

		f(t.NVP(left_engine));
		f(t.NVP(right_engine));

		f(t.NVP(engine_sound));

		f(t.NVP(accelerating));
		f(t.NVP(decelerating));
		f(t.NVP(turning_right));
		f(t.NVP(turning_left));

		f(t.NVP(hand_brake));
		
		f(t.NVP(braking_damping));
		f(t.NVP(braking_angular_damping));

		f(t.NVP(input_acceleration));

		f(t.NVP(acceleration_length));

		f(t.NVP(maximum_speed_with_static_air_resistance));
		f(t.NVP(maximum_speed_with_static_damping));
		f(t.NVP(static_air_resistance));
		f(t.NVP(dynamic_air_resistance));
		f(t.NVP(static_damping));
		f(t.NVP(dynamic_damping));

		f(t.NVP(maximum_lateral_cancellation_impulse));
		f(t.NVP(lateral_impulse_multiplier));

		f(t.NVP(angular_damping));
		f(t.NVP(angular_damping_while_hand_braking));

		f(t.NVP(minimum_speed_for_maneuverability_decrease));
		f(t.NVP(maneuverability_decrease_multiplier));

		f(t.NVP(angular_air_resistance));
		f(t.NVP(angular_air_resistance_while_hand_braking));

		f(t.NVP(speed_for_pitch_unit));

		f(t.NVP(wheel_offset));

		f(t.NVP(last_turned_on));
		f(t.NVP(last_turned_off));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, components::child> t,
		F f
	) {
		f(t.NVP(parent));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, components::container> t,
		F f
	) {
		f(t.NVP(slots));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, components::crosshair> t,
		F f
	) {
		f(t.NVP(orbit_mode));

		f(t.NVP(recoil_entity));

		f(t.NVP(character_entity_to_chase));
		f(t.NVP(base_offset));
		f(t.NVP(bounds_for_base_offset));

		f(t.NVP(visible_world_area));
		f(t.NVP(max_look_expand));

		f(t.NVP(rotation_offset));
		f(t.NVP(size_multiplier));
		f(t.NVP(sensitivity));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, config_lua_table> t,
		F f
	) {
		f(t.NVP(launch_mode));
		f(t.NVP(input_recording_mode));

		f(t.NVP(recording_replay_speed));

		f(t.NVP(determinism_test_cloned_cosmoi_count));

		f(t.NVP(check_content_integrity_every_launch));
		f(t.NVP(save_regenerated_atlases_as_binary));
		f(t.NVP(debug_regenerate_content_every_launch));

		f(t.NVP(enable_hrtf));
		f(t.NVP(max_number_of_sound_sources));

		f(t.NVP(audio_output_device));

		f(t.NVP(sound_effects_volume));
		f(t.NVP(music_volume));

		f(t.NVP(debug_disable_cursor_clipping));

		f(t.NVP(connect_address));
		f(t.NVP(connect_port));
		f(t.NVP(server_port));
		f(t.NVP(alternative_port));

		f(t.NVP(nickname));
		f(t.NVP(debug_second_nickname));

		f(t.NVP(mouse_sensitivity));

		f(t.NVP(tickrate));

		f(t.NVP(jitter_buffer_ms));
		f(t.NVP(client_commands_jitter_buffer_ms));

		f(t.NVP(interpolation_speed));
		f(t.NVP(misprediction_smoothing_multiplier));

		f(t.NVP(debug_var));
		f(t.NVP(debug_randomize_entropies_in_client_setup));
		f(t.NVP(debug_randomize_entropies_in_client_setup_once_every_steps));

		f(t.NVP(server_launch_http_daemon));
		f(t.NVP(server_http_daemon_port));
		f(t.NVP(server_http_daemon_html_file_path));

		f(t.NVP(db_path));
		f(t.NVP(survey_num_file_path));
		f(t.NVP(post_data_file_path));
		f(t.NVP(last_session_update_link));

		f(t.NVP(director_scenario_filename));
		f(t.NVP(menu_intro_scenario_filename));

		f(t.NVP(menu_theme_filename));

		f(t.NVP(rewind_intro_scene_by_secs));
		f(t.NVP(start_menu_music_at_secs));

		f(t.NVP(skip_credits));
		f(t.NVP(latest_news_url));

	}


}