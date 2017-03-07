#include "game/transcendental/types_specification/all_components_declaration.h"
#include "game/components/behaviour_tree_component.h"
#include "game/components/car_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/light_component.h"
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

	template <bool C, class F, class T, int const_count>
	void introspect(
		maybe_const_ref_t<C, constant_size_vector<T, const_count>> t,
		F f
	) {
		f(t.NVP(count));
		f(t.NVP(raw));

	}

	template <bool C, class F, class Enum, class T>
	void introspect(
		maybe_const_ref_t<C, enum_associative_array<Enum, T>> t,
		F f
	) {
		f(t.NVP(is_set));
		f(t.NVP(raw));

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
		maybe_const_ref_t<C, components::damage> t,
		F f
	) {
		f(t.NVP(amount));

		f(t.NVP(impulse_upon_hit));

		f(t.NVP(sender));

		f(t.NVP(damage_upon_collision));
		f(t.NVP(destroy_upon_damage));
		f(t.NVP(constrain_lifetime));
		f(t.NVP(constrain_distance));

		f(t.NVP(damage_charges_before_destruction));

		f(t.NVP(custom_impact_velocity));

		f(t.NVP(damage_falloff));

		f(t.NVP(damage_falloff_starting_distance));
		f(t.NVP(minimum_amount_after_falloff));

		f(t.NVP(distance_travelled));
		f(t.NVP(max_distance));
		f(t.NVP(max_lifetime_ms));
		f(t.NVP(recoil_multiplier));

		f(t.NVP(lifetime_ms));

		f(t.NVP(homing_towards_hostile_strength));
		f(t.NVP(particular_homing_target));

		f(t.NVP(saved_point_of_impact_before_death));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, components::driver> t,
		F f
	) {
		f(t.NVP(owned_vehicle));
		f(t.NVP(density_multiplier_while_driving));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, components::dynamic_tree_node> t,
		F f
	) {
		f(t.NVP(always_visible));
		f(t.NVP(activated));
		f(t.NVP(type));


		f(t.NVP(aabb));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, components::fixtures::convex_partitioned_collider> t,
		F f
	) {
		f(t.NVP(shape));
		f(t.NVP(material));

		f(t.NVP(collision_sound_gain_mult));

		f(t.NVP(density));
		f(t.NVP(density_multiplier));
		f(t.NVP(friction));
		f(t.NVP(restitution));
			
		f(t.NVP(filter));
		f(t.NVP(destructible));
		f(t.NVP(sensor));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, components::fixtures> t,
		F f
	) {
		f(t.NVP(colliders));
		f(t.NVP(offsets_for_created_shapes));

		f(t.NVP(activated));
		f(t.NVP(is_friction_ground));
		f(t.NVP(disable_standard_collision_resolution));
		f(t.NVP(can_driver_shoot_through));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, components::flags> t,
		F f
	) {
		f(t.NVP(bit_flags));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, components::force_joint> t,
		F f
	) {
		f(t.NVP(chased_entity));

		f(t.NVP(force_towards_chased_entity));
		f(t.NVP(distance_when_force_easing_starts));
		f(t.NVP(power_of_force_easing_multiplier));

		f(t.NVP(percent_applied_to_chased_entity));

		f(t.NVP(divide_transform_mode));
		f(t.NVP(consider_rotation));

		f(t.NVP(chased_entity_offset));

		f(t.NVP(force_offsets));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, components::grenade> t,
		F f
	) {
		f(t.NVP(spoon));
		f(t.NVP(type));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, components::guid> t,
		F f
	) {
		f(t.NVP(value));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, components::gun> t,
		F f
	) {
		f(t.NVP(shot_cooldown));
		f(t.NVP(action_mode));
		f(t.NVP(num_last_bullets_to_trigger_low_ammo_cue));

		f(t.NVP(muzzle_velocity));

		f(t.NVP(damage_multiplier));

		f(t.NVP(bullet_spawn_offset));

		f(t.NVP(camera_shake_radius));
		f(t.NVP(camera_shake_spread_degrees));

		f(t.NVP(trigger_pressed));

		f(t.NVP(shell_velocity));
		f(t.NVP(shell_angular_velocity));

		f(t.NVP(shell_spread_degrees));

		f(t.NVP(recoil));

		f(t.NVP(shell_spawn_offset));

		f(t.NVP(magic_missile_definition));

		f(t.NVP(current_heat));
		f(t.NVP(gunshot_adds_heat));
		f(t.NVP(maximum_heat));
		f(t.NVP(engine_sound_strength));

		f(t.NVP(firing_engine_sound));
		f(t.NVP(muzzle_particles));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, components::interpolation> t,
		F f
	) {
		f(t.NVP(base_exponent));
		f(t.NVP(place_of_birth));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, components::item> t,
		F f
	) {
		f(t.NVP(current_mounting));
		f(t.NVP(intended_mounting));

		f(t.NVP(categories_for_slot_compatibility));

		f(t.NVP(charges));
		f(t.NVP(space_occupied_per_charge));
		f(t.NVP(stackable));

		f(t.NVP(dual_wield_accuracy_loss_percentage));
		f(t.NVP(dual_wield_accuracy_loss_multiplier));

		f(t.NVP(current_slot));
		f(t.NVP(target_slot_after_unmount));

		f(t.NVP(montage_time_ms));
		f(t.NVP(montage_time_left_ms));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, components::item_slot_transfers> t,
		F f
	) {
		f(t.NVP(pickup_timeout));
		f(t.NVP(mounting));

		f(t.NVP(only_pick_these_items));
		f(t.NVP(pick_all_touched_items_if_list_to_pick_empty));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, components::light::value_variation> t,
		F f
	) {
		f(t.NVP(min_value));
		f(t.NVP(max_value));
		f(t.NVP(change_speed));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, components::light::attenuation> t,
		F f
	) {
		f(t.NVP(base_value));
		f(t.NVP(variation));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, components::light> t,
		F f
	) {
		f(t.NVP(color));

		f(t.NVP(constant));
		f(t.NVP(linear));
		f(t.NVP(quadratic));
		f(t.NVP(max_distance));
		
		f(t.NVP(wall_constant));
		f(t.NVP(wall_linear));
		f(t.NVP(wall_quadratic));
		f(t.NVP(wall_max_distance));

		f(t.NVP(position_variations));

	}

	template <bool C, class F, class id_type>
	void introspect(
		maybe_const_ref_t<C, basic_inventory_slot_id<id_type>> t,
		F f
	) {
		f(t.NVP(type));
		f(t.NVP(container_entity));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, inventory_item_address> t,
		F f
	) {
		f(t.NVP(root_container));
		f(t.NVP(directions));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, inventory_traversal> t,
		F f
	) {
		f(t.NVP(parent_slot));
		f(t.NVP(current_address));
		f(t.NVP(attachment_offset));
		f(t.NVP(item_remains_physical));

	}

	template <bool C, class F, class key>
	void introspect(
		maybe_const_ref_t<C, basic_cosmic_entropy<key>> t,
		F f
	) {
		f(t.NVP(cast_spells));
		f(t.NVP(intents_per_entity));
		f(t.NVP(transfer_requests));

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

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, scripted_image_stamp> t,
		F f
	) {
		f(t.NVP(commands));

	}


}