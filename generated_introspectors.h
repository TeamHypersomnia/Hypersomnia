#pragma once
#include <tuple>
#include <array>

#include "augs/misc/trivially_copyable_tuple.h"
#include "augs/templates/for_each_in_types.h"

#include "game/transcendental/entity_id_declaration.h"
#include "game/detail/shape_variant_declaration.h"

#define FIELD(x) f(#x, _t_.x...)

// Forward declarations

struct rgba;
template <class T>
struct ltrbt;
template <class T>
struct xywht;
template <class type>
struct vec2t;
class recoil_player;
template <class T>
struct zeroed_pod;
struct animation_frame;
struct animation;
struct state_of_behaviour_tree_instance;
struct particle_effect_modifier;
struct particles_emission;
struct particle_effect_logical_meta;
struct particle_effect_response;
struct physical_material;
struct sound_response;
struct spell_logical_meta;
struct spell_data;
class tile_layer;
struct behaviour_tree_instance;
struct car_engine_entities;
struct convex_partitioned_collider;
struct item_slot_mounting_operation;
struct light_value_variation;
struct light_attenuation;
struct movement_subscribtion;
struct particle_effect_input;
struct sound_effect_input;
struct friction_connection;
struct convex_poly_destruction_scar;
struct convex_poly_destruction_data;
struct convex_poly;
struct convex_partitioned_shape;
struct inventory_slot;
template <class id_type>
struct basic_inventory_slot_id;
struct inventory_item_address;
struct inventory_traversal;
template <class id_type>
struct basic_item_slot_transfer_request_data;
struct general_particle;
struct animated_particle;
struct homing_animated_particle;
struct electric_shield_perk;
struct haste_perk;
struct perk_timing;
struct sentience_meter;
struct circle_shape;
struct spell_instance_data;
struct all_simulation_settings;
struct pathfinding_settings;
struct si_scaling;
struct visibility_settings;
template <class key>
struct basic_cosmic_entropy;
struct guid_mapped_entropy;
struct cosmic_entropy;
struct cosmos_metadata;
struct cosmos_significant_state;
struct entity_guid;
struct entity_id;
struct child_entity_id;
class config_lua_table;
struct debug_drawing_settings;
struct hotbar_settings;
struct neon_map_stamp;
struct scripted_image_stamp;
struct texture_atlas_stamp;
struct texture_atlas_metadata;
struct play_scene;
struct play_sound;
struct focus_guid;
struct focus_index;
struct set_sfx_gain;
struct speed_change;
struct b2Vec2;
struct b2Rot;
struct b2Transform;
struct b2Sweep;
struct b2Filter;

namespace augs {
	struct sound_buffer_logical_meta;
	struct sound_effect_modifier;
	struct vertex;
	struct font_glyph_metadata;
	struct font_metadata_from_file;
	struct baked_font;
	struct font_loading_input;
	struct paint_circle_midpoint_command;
	struct paint_circle_filled_command;
	struct paint_line_command;
	template <class T, size_t const_count>
	class constant_size_vector;
	template <class T, class _enum>
	class enum_array;
	template <class key_type, class mapped_type>
	class enum_associative_array;
	struct machine_entropy;
	class pooled_object_raw_id;
	template <class T>
	class pooled_object_id;
	struct stepped_timestamp;
	struct stepped_cooldown;
	template <class A, class B>
	class trivially_copyable_pair;
}

namespace components {
	struct animation;
	struct attitude;
	struct behaviour_tree;
	struct car;
	struct catridge;
	struct child;
	struct container;
	struct crosshair;
	struct damage;
	struct driver;
	struct fixtures;
	struct flags;
	struct force_joint;
	struct grenade;
	struct guid;
	struct gun;
	struct inferred_state;
	struct interpolation;
	struct item;
	struct item_slot_transfers;
	struct light;
	struct melee;
	struct movement;
	struct name;
	struct particles_existence;
	struct physical_relations;
	struct polygon;
	struct position_copying;
	struct processing;
	struct render;
	struct rigid_body;
	struct rotation_copying;
	struct sentience;
	struct sound_existence;
	struct special_physics;
	struct sprite;
	struct tile_layer_instance;
	struct trace;
	struct transform;
	struct tree_of_npo_node;
	struct trigger_collision_detector;
	struct trigger;
	struct trigger_query_detector;
	struct wandering_pixels;
}

namespace augs {
	struct introspection_access {
		/* Hand-written introspectors that do not fit into the standard schema begin here */

		template <class F, class ElemType, size_t count, class... Instances>
		static void introspect_body(
			const std::array<ElemType, count>* const,
			F f,
			Instances&&... t
		) {
			for (size_t i = 0; i < count; ++i) {
				f(std::to_string(i), t[i]...);
			}
		}

		template <class F, class... Types, class... Instances>
		static void introspect_body(
			const std::tuple<Types...>* const,
			F f,
			Instances&&... t
		) {
			templates_detail::for_each_through_std_get(
				[f](auto num, auto&&... args) {
					f(std::to_string(num), std::forward<decltype(args)>(args)...);
				},
				std::index_sequence_for<Types...>{},
				std::forward<Instances>(t)...
			);
		}

		template <class F, class... Types, class... Instances>
		static void introspect_body(
			const augs::trivially_copyable_tuple<Types...>* const,
			F f,
			Instances&&... t
		) {
			templates_detail::for_each_through_std_get(
				[f](auto num, auto&&... args) {
					f(std::to_string(num), std::forward<decltype(args)>(args)...);
				},
				std::index_sequence_for<Types...>{},
				std::forward<Instances>(t)...
			);
		}

		/* Generated introspectors begin here */

		template <class F, class... Instances>
		static void introspect_body(
			const augs::sound_buffer_logical_meta* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(max_duration_in_seconds);
			FIELD(num_of_variations);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const augs::sound_effect_modifier* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(gain);
			FIELD(pitch);
			FIELD(max_distance);
			FIELD(reference_distance);
			FIELD(repetitions);
			FIELD(fade_on_exit);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const rgba* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(r);
			FIELD(g);
			FIELD(b);
			FIELD(a);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const augs::vertex* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(pos);
			FIELD(texcoord);
			FIELD(color);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const augs::font_glyph_metadata* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(adv);
			FIELD(bear_x);
			FIELD(bear_y);
			FIELD(index);
			FIELD(unicode);

			FIELD(size);

			FIELD(kerning);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const augs::font_metadata_from_file* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(ascender);
			FIELD(descender);

			FIELD(pt);

			FIELD(glyphs);
			FIELD(unicode_to_glyph_index);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const augs::baked_font* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(meta_from_file);
			FIELD(glyphs_in_atlas);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const augs::font_loading_input* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(path);
			FIELD(characters);
		
			FIELD(pt);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const augs::paint_circle_midpoint_command* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(radius);
			FIELD(border_width);
			FIELD(scale_alpha);
			FIELD(constrain_angle);
			FIELD(angle_start);
			FIELD(angle_end);
			FIELD(filling);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const augs::paint_circle_filled_command* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(radius);
			FIELD(filling);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const augs::paint_line_command* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(from);
			FIELD(to);
			FIELD(filling);
		}

		template <class F, class T, class... Instances>
		static void introspect_body(
			const ltrbt<T>* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(l);
			FIELD(t);
			FIELD(r);
			FIELD(b);
		}

		template <class F, class T, class... Instances>
		static void introspect_body(
			const xywht<T>* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(x);
			FIELD(y);
			FIELD(w);
			FIELD(h);
		}

		template <class F, class type, class... Instances>
		static void introspect_body(
			const vec2t<type>* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(x);
			FIELD(y);
		}

		template <class F, class T, size_t const_count, class... Instances>
		static void introspect_body(
			const augs::constant_size_vector<T, const_count>* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(count);
			FIELD(raw);
		}

		template <class F, class T, class _enum, class... Instances>
		static void introspect_body(
			const augs::enum_array<T, _enum>* const,
			F f,
			Instances&&... _t_
		) {
		introspect_body(static_cast<std::array<T, static_cast<size_t>(_enum::COUNT)>*>(nullptr), f, std::forward<Instances>(_t_)...);
		}

		template <class F, class key_type, class mapped_type, class... Instances>
		static void introspect_body(
			const augs::enum_associative_array<key_type, mapped_type>* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(is_set);
			FIELD(raw);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const augs::machine_entropy* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(local);
			FIELD(remote);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const augs::pooled_object_raw_id* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(version);
			FIELD(indirection_index);
		}

		template <class F, class T, class... Instances>
		static void introspect_body(
			const augs::pooled_object_id<T>* const,
			F f,
			Instances&&... _t_
		) {
		introspect_body(static_cast<augs::pooled_object_raw_id*>(nullptr), f, std::forward<Instances>(_t_)...);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const recoil_player* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(offsets);
			FIELD(current_offset);
			FIELD(reversed);
			FIELD(repeat_last_n_offsets);

			FIELD(single_cooldown_duration_ms);
			FIELD(remaining_cooldown_duration);
			FIELD(scale);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const augs::stepped_timestamp* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(step);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const augs::stepped_cooldown* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(when_last_fired);
			FIELD(cooldown_duration_ms);
		}

		template <class F, class A, class B, class... Instances>
		static void introspect_body(
			const augs::trivially_copyable_pair<A, B>* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(first);
			FIELD(second);
		}

		template <class F, class T, class... Instances>
		static void introspect_body(
			const zeroed_pod<T>* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(pod);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const animation_frame* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(image_id);
			FIELD(duration_milliseconds);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const animation* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(frames);

			FIELD(loop_mode);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const state_of_behaviour_tree_instance* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(previously_executed_leaf_id);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const particle_effect_modifier* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(colorize);
			FIELD(scale_amounts);
			FIELD(scale_lifetimes);
			FIELD(homing_target);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const particles_emission* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(spread_degrees);
			FIELD(base_speed);
			FIELD(base_speed_variation);
			FIELD(rotation_speed);
			FIELD(particles_per_sec);
			FIELD(stream_lifetime_ms);
			FIELD(particle_lifetime_ms);
			FIELD(size_multiplier);
			FIELD(acceleration);
			FIELD(angular_offset);
			FIELD(swing_spread);
			FIELD(swings_per_sec);
			FIELD(min_swing_spread);
			FIELD(max_swing_spread);
			FIELD(min_swings_per_sec);
			FIELD(max_swings_per_sec);
			FIELD(swing_spread_change_rate);
			FIELD(swing_speed_change_rate);
			FIELD(fade_when_ms_remaining);
			FIELD(num_of_particles_to_spawn_initially);

			FIELD(randomize_spawn_point_within_circle_of_outer_radius);
			FIELD(randomize_spawn_point_within_circle_of_inner_radius);

			FIELD(starting_spawn_circle_size_multiplier);
			FIELD(ending_spawn_circle_size_multiplier);

			FIELD(starting_homing_force);
			FIELD(ending_homing_force);

			FIELD(homing_target);

			FIELD(initial_rotation_variation);
			FIELD(randomize_acceleration);
			FIELD(should_particles_look_towards_velocity);

			FIELD(particle_definitions);
			FIELD(target_render_layer);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const particle_effect_logical_meta* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(max_duration_in_seconds);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const particle_effect_response* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(id);
			FIELD(modifier);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const physical_material* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(collision_sound_matrix);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const sound_response* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(id);
			FIELD(modifier);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const spell_logical_meta* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(personal_electricity_required);
			FIELD(cooldown_ms);
			FIELD(casting_time_ms);
			FIELD(perk_duration_seconds);

			FIELD(border_col);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const spell_data* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(logical);

			FIELD(icon);
			FIELD(incantation);
			FIELD(spell_name);
			FIELD(spell_description);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const tile_layer* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(tileset);

			FIELD(tiles);
			FIELD(size);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::animation* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(current_animation);

			FIELD(priority);
			FIELD(frame_num);
			FIELD(player_position_ms);
			FIELD(speed_factor);

			FIELD(state);
			FIELD(paused_state);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::attitude* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(maximum_divergence_angle_before_shooting);

			FIELD(parties);
			FIELD(hostile_parties);

			FIELD(specific_hostile_entities);
		
			FIELD(currently_attacked_visible_entity);
			FIELD(target_attitude);

			FIELD(is_alert);
			FIELD(last_seen_target_position_inspected);

			FIELD(last_seen_target_position);
			FIELD(last_seen_target_velocity);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const behaviour_tree_instance* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(state);
			FIELD(tree_id);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::behaviour_tree* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(concurrent_trees);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const car_engine_entities* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(physical);
			FIELD(particles);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::car* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(current_driver);

			FIELD(interior);

			FIELD(left_wheel_trigger);
			FIELD(right_wheel_trigger);

			FIELD(acceleration_engine);
			FIELD(deceleration_engine);

			FIELD(left_engine);
			FIELD(right_engine);

			FIELD(engine_sound);

			FIELD(accelerating);
			FIELD(decelerating);
			FIELD(turning_right);
			FIELD(turning_left);

			FIELD(hand_brake);
		
			FIELD(braking_damping);
			FIELD(braking_angular_damping);

			FIELD(input_acceleration);

			FIELD(acceleration_length);

			FIELD(maximum_speed_with_static_air_resistance);
			FIELD(maximum_speed_with_static_damping);
			FIELD(static_air_resistance);
			FIELD(dynamic_air_resistance);
			FIELD(static_damping);
			FIELD(dynamic_damping);

			FIELD(maximum_lateral_cancellation_impulse);
			FIELD(lateral_impulse_multiplier);

			FIELD(angular_damping);
			FIELD(angular_damping_while_hand_braking);

			FIELD(minimum_speed_for_maneuverability_decrease);
			FIELD(maneuverability_decrease_multiplier);

			FIELD(angular_air_resistance);
			FIELD(angular_air_resistance_while_hand_braking);

			FIELD(speed_for_pitch_unit);

			FIELD(wheel_offset);

			FIELD(last_turned_on);
			FIELD(last_turned_off);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::catridge* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(shell);
			FIELD(round);

			FIELD(shell_trace_particle_effect_response);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::child* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(parent);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::container* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(slots);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::crosshair* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(orbit_mode);

			FIELD(recoil_entity);

			FIELD(character_entity_to_chase);
			FIELD(base_offset);
			FIELD(bounds_for_base_offset);

			FIELD(visible_world_area);
			FIELD(max_look_expand);

			FIELD(rotation_offset);
			FIELD(sensitivity);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::damage* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(amount);

			FIELD(impulse_upon_hit);
			FIELD(impulse_multiplier_against_sentience);

			FIELD(sender);

			FIELD(damage_upon_collision);
			FIELD(destroy_upon_damage);
			FIELD(constrain_lifetime);
			FIELD(constrain_distance);

			FIELD(damage_charges_before_destruction);

			FIELD(custom_impact_velocity);

			FIELD(damage_falloff);

			FIELD(damage_falloff_starting_distance);
			FIELD(minimum_amount_after_falloff);

			FIELD(distance_travelled);
			FIELD(max_distance);
			FIELD(max_lifetime_ms);
			FIELD(recoil_multiplier);

			FIELD(lifetime_ms);

			FIELD(homing_towards_hostile_strength);
			FIELD(particular_homing_target);
		
			FIELD(trace_sound);

			FIELD(bullet_trace_sound_response);
			FIELD(destruction_sound_response);

			FIELD(muzzle_leave_particle_effect_response);
			FIELD(bullet_trace_particle_effect_response);
			FIELD(destruction_particle_effect_response);

			FIELD(saved_point_of_impact_before_death);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::driver* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(owned_vehicle);
			FIELD(density_multiplier_while_driving);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const convex_partitioned_collider* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(shape);
			FIELD(destruction);

			FIELD(material);

			FIELD(collision_sound_gain_mult);

			FIELD(density);
			FIELD(density_multiplier);
			FIELD(friction);
			FIELD(restitution);

			FIELD(filter);
			FIELD(destructible);
			FIELD(sensor);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::fixtures* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(colliders);
			FIELD(offsets_for_created_shapes);

			FIELD(activated);
			FIELD(is_friction_ground);
			FIELD(disable_standard_collision_resolution);
			FIELD(can_driver_shoot_through);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::flags* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(bit_flags);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::force_joint* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(chased_entity);

			FIELD(force_towards_chased_entity);
			FIELD(distance_when_force_easing_starts);
			FIELD(power_of_force_easing_multiplier);

			FIELD(percent_applied_to_chased_entity);

			FIELD(divide_transform_mode);
			FIELD(consider_rotation);

			FIELD(chased_entity_offset);

			FIELD(force_offsets);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::grenade* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(spoon);
			FIELD(released_spoon);
			FIELD(type);
			FIELD(released_image_id);

			FIELD(when_released);
			FIELD(when_explodes);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::guid* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(value);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::gun* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(shot_cooldown);
			FIELD(action_mode);
			FIELD(num_last_bullets_to_trigger_low_ammo_cue);

			FIELD(muzzle_velocity);

			FIELD(damage_multiplier);

			FIELD(bullet_spawn_offset);

			FIELD(camera_shake_radius);
			FIELD(camera_shake_spread_degrees);

			FIELD(trigger_pressed);

			FIELD(shell_velocity);
			FIELD(shell_angular_velocity);

			FIELD(shell_spread_degrees);

			FIELD(recoil);

			FIELD(shell_spawn_offset);

			FIELD(magic_missile_definition);

			FIELD(current_heat);
			FIELD(gunshot_adds_heat);
			FIELD(maximum_heat);
			FIELD(engine_sound_strength);

			FIELD(firing_engine_sound);
			FIELD(muzzle_particles);

			FIELD(muzzle_shot_sound_response);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::inferred_state* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(dummy);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::interpolation* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(base_exponent);
			FIELD(place_of_birth);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::item* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(current_mounting);
			FIELD(intended_mounting);

			FIELD(categories_for_slot_compatibility);

			FIELD(charges);
			FIELD(space_occupied_per_charge);
			FIELD(stackable);

			FIELD(dual_wield_accuracy_loss_percentage);
			FIELD(dual_wield_accuracy_loss_multiplier);

			FIELD(current_slot);
			FIELD(target_slot_after_unmount);

			FIELD(montage_time_ms);
			FIELD(montage_time_left_ms);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const item_slot_mounting_operation* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(current_item);
			FIELD(intented_mounting_slot);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::item_slot_transfers* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(pickup_timeout);
			FIELD(mounting);

			FIELD(only_pick_these_items);
			FIELD(pick_all_touched_items_if_list_to_pick_empty);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const light_value_variation* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(min_value);
			FIELD(max_value);
			FIELD(change_speed);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const light_attenuation* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(base_value);
			FIELD(variation);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::light* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(color);

			FIELD(constant);
			FIELD(linear);
			FIELD(quadratic);
			FIELD(max_distance);
		
			FIELD(wall_constant);
			FIELD(wall_linear);
			FIELD(wall_quadratic);
			FIELD(wall_max_distance);

			FIELD(position_variations);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::melee* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(primary_move_flag);
			FIELD(secondary_move_flag);
			FIELD(tertiary_move_flag);

			FIELD(current_state);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const movement_subscribtion* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(target);
			FIELD(stop_response_at_zero_speed);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::movement* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(response_receivers);
		
			FIELD(moving_left);
			FIELD(moving_right);
			FIELD(moving_forward);
			FIELD(moving_backward);

			FIELD(walking_enabled);
			FIELD(enable_braking_damping);
			FIELD(enable_animation);
			FIELD(sprint_enabled);

			FIELD(input_acceleration_axes);
			FIELD(acceleration_length);

			FIELD(applied_force_offset);

			FIELD(non_braking_damping);
			FIELD(braking_damping);

			FIELD(standard_linear_damping);

			FIELD(make_inert_for_ms);
			FIELD(max_speed_for_movement_event);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::name* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(id);

			FIELD(custom_nickname);
			FIELD(nickname);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const particle_effect_input* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(effect);
			FIELD(delete_entity_after_effect_lifetime);

			FIELD(displace_source_position_within_radius);
			FIELD(single_displacement_duration_ms);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::particles_existence* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(input);

			FIELD(current_displacement);
			FIELD(time_of_last_displacement);
			FIELD(current_displacement_duration_bound_ms);

			FIELD(time_of_birth);
			FIELD(max_lifetime_in_steps);

			FIELD(distribute_within_segment_of_length);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::physical_relations* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(owner_body);
			FIELD(fixture_entities);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::polygon* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(texture_map);

			FIELD(vertices);
			FIELD(triangulation_indices);

		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::position_copying* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(target);

			FIELD(offset);
			FIELD(rotation_orbit_offset);
		
			FIELD(reference_position);
			FIELD(target_reference_position);
		
			FIELD(scrolling_speed);

			FIELD(rotation_offset);
			FIELD(rotation_multiplier);

			FIELD(position_copying_mode);
			FIELD(position_copying_rotation);
			FIELD(track_origin);
			FIELD(target_newly_set);
			FIELD(previous);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::processing* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(activated);

			FIELD(processing_subject_categories);
			FIELD(disabled_categories);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::render* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(draw_border);
			FIELD(layer);

			FIELD(border_color);

			FIELD(last_step_when_visible);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::rigid_body* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(fixed_rotation);
			FIELD(bullet);
			FIELD(angled_damping);
			FIELD(activated);

			FIELD(body_type);

			FIELD(angular_damping);
			FIELD(linear_damping);
			FIELD(linear_damping_vec);
			FIELD(gravity_scale);

			FIELD(transform);
			FIELD(sweep);

			FIELD(velocity);
			FIELD(angular_velocity);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::rotation_copying* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(target);
			FIELD(stashed_target);

			FIELD(easing_mode);

			FIELD(colinearize_item_in_hand);
			FIELD(update_value);
		
			FIELD(smoothing_average_factor);
			FIELD(averages_per_sec);
		
			FIELD(last_rotation_interpolant);

			FIELD(look_mode);
			FIELD(stashed_look_mode);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::sentience* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(time_of_last_received_damage);
			FIELD(time_of_last_exertion);

			FIELD(cast_cooldown_for_all_spells);

			FIELD(health);
			FIELD(personal_electricity);
			FIELD(consciousness);

			FIELD(haste);
			FIELD(electric_shield);

			FIELD(spells);

			FIELD(currently_casted_spell);
			FIELD(transform_when_spell_casted);
			FIELD(time_of_last_spell_cast);
			FIELD(time_of_last_exhausted_cast);

			FIELD(time_of_last_shake);
			FIELD(shake_for_ms);

			FIELD(comfort_zone);
			FIELD(minimum_danger_amount_to_evade);
			FIELD(danger_amount_from_hostile_attitude);

			FIELD(aimpunch);
			FIELD(health_damage_particles);
			FIELD(character_crosshair);

			FIELD(health_decrease_sound_response);
			FIELD(death_sound_response);

			FIELD(health_decrease_particle_effect_response);

		}

		template <class F, class... Instances>
		static void introspect_body(
			const sound_effect_input* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(effect);
			FIELD(delete_entity_after_effect_lifetime);
			FIELD(variation_number);
			FIELD(direct_listener);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::sound_existence* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(input);

			FIELD(time_of_birth);
			FIELD(max_lifetime_in_steps);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const friction_connection* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(target);
			FIELD(fixtures_connected);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::special_physics* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(dropped_or_created_cooldown);
			FIELD(during_cooldown_ignore_collision_with);
			FIELD(owner_friction_ground);
			FIELD(owner_friction_grounds);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::sprite* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(tex);
			FIELD(color);
			FIELD(size);
			FIELD(center_offset);
			FIELD(rotation_offset);

			FIELD(flip_horizontally);
			FIELD(flip_vertically);
		
			FIELD(effect);

			FIELD(max_specular_blinks);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::tile_layer_instance* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(id);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::trace* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(max_multiplier_x);
			FIELD(max_multiplier_y);

			FIELD(additional_multiplier);
			FIELD(chosen_multiplier);

			FIELD(lengthening_duration_ms);
			FIELD(chosen_lengthening_duration_ms);
			FIELD(lengthening_time_passed_ms);

			FIELD(is_it_finishing_trace);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::transform* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(pos);
			FIELD(rotation);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::tree_of_npo_node* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(always_visible);
			FIELD(activated);
			FIELD(type);


			FIELD(aabb);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::trigger_collision_detector* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(detection_intent_enabled);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::trigger* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(entity_to_be_notified);
			FIELD(react_to_collision_detectors);
			FIELD(react_to_query_detectors);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::trigger_query_detector* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(detection_intent_enabled);
			FIELD(spam_trigger_requests_when_detection_intented);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const components::wandering_pixels* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(reach);
			FIELD(face);
			FIELD(count);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const convex_poly_destruction_scar* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(first_impact);
			FIELD(depth_point);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const convex_poly_destruction_data* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(scars);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const convex_poly* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(vertices);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const convex_partitioned_shape* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(convex_polys);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const inventory_slot* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(category_allowed);

			FIELD(items_need_mounting);
			FIELD(only_last_inserted_is_movable);

			FIELD(for_categorized_items_only);

			FIELD(is_physical_attachment_slot);
			FIELD(always_allow_exactly_one_item);


			FIELD(montage_time_multiplier);

			FIELD(space_available);

			FIELD(attachment_density_multiplier);

			FIELD(attachment_sticking_mode);
			FIELD(attachment_offset);

			FIELD(items_inside);
		}

		template <class F, class id_type, class... Instances>
		static void introspect_body(
			const basic_inventory_slot_id<id_type>* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(type);
			FIELD(container_entity);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const inventory_item_address* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(root_container);
			FIELD(directions);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const inventory_traversal* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(parent_slot);
			FIELD(current_address);
			FIELD(attachment_offset);
			FIELD(item_remains_physical);
		}

		template <class F, class id_type, class... Instances>
		static void introspect_body(
			const basic_item_slot_transfer_request_data<id_type>* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(item);
			FIELD(target_slot);

			FIELD(specified_quantity);
			FIELD(force_immediate_mount);
			FIELD(impulse_applied_on_drop);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const general_particle* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(pos);
			FIELD(vel);
			FIELD(acc);
			FIELD(image_id);
			FIELD(color);
			FIELD(size);
			FIELD(rotation);
			FIELD(rotation_speed);
			FIELD(linear_damping);
			FIELD(angular_damping);
			FIELD(lifetime_ms);
			FIELD(max_lifetime_ms);
			FIELD(shrink_when_ms_remaining);
			FIELD(unshrinking_time_ms);

			FIELD(alpha_levels);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const animated_particle* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(pos);
			FIELD(vel);
			FIELD(acc);
	
			FIELD(linear_damping);
			FIELD(lifetime_ms);

			FIELD(first_face);
			FIELD(color);
			FIELD(frame_duration_ms);
			FIELD(frame_count);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const homing_animated_particle* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(pos);
			FIELD(vel);
			FIELD(acc);

			FIELD(linear_damping);
			FIELD(lifetime_ms);

			FIELD(homing_force);

			FIELD(first_face);
			FIELD(color);
			FIELD(frame_duration_ms);
			FIELD(frame_count);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const electric_shield_perk* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(timing);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const haste_perk* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(timing);
			FIELD(is_greater);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const perk_timing* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(duration);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const sentience_meter* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(value);
			FIELD(maximum);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const circle_shape* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(radius);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const spell_instance_data* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(cast_cooldown);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const all_simulation_settings* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(visibility);
			FIELD(pathfinding);
			FIELD(si);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const pathfinding_settings* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(epsilon_distance_visible_point);
			FIELD(epsilon_distance_the_same_vertex);

			FIELD(draw_memorised_walls);
			FIELD(draw_undiscovered);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const si_scaling* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(to_meters_multiplier);
			FIELD(to_pixels_multiplier);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const visibility_settings* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(draw_triangle_edges);
			FIELD(draw_cast_rays);
			FIELD(draw_discontinuities);
			FIELD(draw_visible_walls);

			FIELD(epsilon_ray_distance_variation);
			FIELD(epsilon_distance_vertex_hit);
			FIELD(epsilon_threshold_obstacle_hit);
		}

		template <class F, class key, class... Instances>
		static void introspect_body(
			const basic_cosmic_entropy<key>* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(cast_spells);
			FIELD(intents_per_entity);
			FIELD(transfer_requests);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const guid_mapped_entropy* const,
			F f,
			Instances&&... _t_
		) {
		introspect_body(static_cast<basic_cosmic_entropy<entity_guid>*>(nullptr), f, std::forward<Instances>(_t_)...);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const cosmic_entropy* const,
			F f,
			Instances&&... _t_
		) {
		introspect_body(static_cast<basic_cosmic_entropy<entity_id>*>(nullptr), f, std::forward<Instances>(_t_)...);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const cosmos_metadata* const,
			F f,
			Instances&&... _t_
		) {

			FIELD(delta);
			FIELD(total_steps_passed);

#if COSMOS_TRACKS_GUIDS
			FIELD(next_entity_guid);
#endif
			FIELD(settings);

			FIELD(logical_metas_of_assets);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const cosmos_significant_state* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(meta);

			FIELD(pool_for_aggregates);
			FIELD(pools_for_components);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const entity_guid* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(value);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const entity_id* const,
			F f,
			Instances&&... _t_
		) {
		introspect_body(static_cast<augs::pooled_object_id<put_all_components_into_t<augs::component_aggregate>>*>(nullptr), f, std::forward<Instances>(_t_)...);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const child_entity_id* const,
			F f,
			Instances&&... _t_
		) {
		introspect_body(static_cast<entity_id*>(nullptr), f, std::forward<Instances>(_t_)...);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const config_lua_table* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(launch_mode);
			FIELD(input_recording_mode);

			FIELD(recording_replay_speed);

			FIELD(determinism_test_cloned_cosmoi_count);

			FIELD(check_content_integrity_every_launch);
			FIELD(save_regenerated_atlases_as_binary);
			FIELD(debug_regenerate_content_every_launch);

			FIELD(enable_hrtf);
			FIELD(max_number_of_sound_sources);

			FIELD(audio_output_device);

			FIELD(sound_effects_volume);
			FIELD(music_volume);

			FIELD(debug_disable_cursor_clipping);

			FIELD(connect_address);
			FIELD(connect_port);
			FIELD(server_port);
			FIELD(alternative_port);

			FIELD(nickname);
			FIELD(debug_second_nickname);

			FIELD(mouse_sensitivity);

			FIELD(default_tickrate);

			FIELD(jitter_buffer_ms);
			FIELD(client_commands_jitter_buffer_ms);

			FIELD(interpolation_speed);
			FIELD(misprediction_smoothing_multiplier);

			FIELD(debug_var);
			FIELD(debug_randomize_entropies_in_client_setup);
			FIELD(debug_randomize_entropies_in_client_setup_once_every_steps);

			FIELD(server_launch_http_daemon);
			FIELD(server_http_daemon_port);
			FIELD(server_http_daemon_html_file_path);

			FIELD(db_path);
			FIELD(survey_num_file_path);
			FIELD(post_data_file_path);
			FIELD(last_session_update_link);

			FIELD(director_input_scene_entropy_path);
			FIELD(choreographic_input_scenario_path);
			FIELD(menu_intro_scene_entropy_path);

			FIELD(menu_theme_path);

			FIELD(rewind_intro_scene_by_secs);
			FIELD(start_menu_music_at_secs);

			FIELD(skip_credits);
			FIELD(latest_news_url);
	
			FIELD(debug);
			FIELD(hotbar);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const debug_drawing_settings* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(drawing_enabled);

			FIELD(draw_colinearization);
			FIELD(draw_forces);
			FIELD(draw_friction_field_collisions_of_entering);
			FIELD(draw_explosion_forces);
			FIELD(draw_visibility);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const hotbar_settings* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(increase_inside_alpha_when_selected);
			FIELD(colorize_inside_when_selected);

			FIELD(primary_selected_color);
			FIELD(secondary_selected_color);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const neon_map_stamp* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(standard_deviation);
			FIELD(radius_towards_x_axis);
			FIELD(radius_towards_y_axis);
			FIELD(amplification);
			FIELD(alpha_multiplier);
			FIELD(last_write_time_of_source);

			FIELD(light_colors);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const scripted_image_stamp* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(commands);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const texture_atlas_stamp* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(image_stamps);
			FIELD(font_stamps);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const texture_atlas_metadata* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(atlas_image_size);

			FIELD(images);
			FIELD(fonts);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const play_scene* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(id);
			FIELD(at_time);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const play_sound* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(id);
			FIELD(at_time);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const focus_guid* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(guid);
			FIELD(at_time);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const focus_index* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(index);
			FIELD(at_time);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const set_sfx_gain* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(gain);
			FIELD(at_time);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const speed_change* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(speed_multiplier);

			FIELD(at_time);
			FIELD(to_time);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const b2Vec2* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(x);
			FIELD(y);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const b2Rot* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(s);
			FIELD(c);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const b2Transform* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(p);
			FIELD(q);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const b2Sweep* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(localCenter);
			FIELD(c0);
			FIELD(c);
			FIELD(a0);
			FIELD(a);
			FIELD(alpha0);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const b2Filter* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(categoryBits);
			FIELD(maskBits);
			FIELD(groupIndex);
		}

	};
}