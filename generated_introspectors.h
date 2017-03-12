#pragma once
#include "game/transcendental/entity_id.h"

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
struct behaviour_tree_instance;
struct car_engine_entities;
struct convex_partitioned_collider;
struct item_slot_mounting_operation;
struct light_value_variation;
struct light_attenuation;
struct movement_subscribtion;
struct particles_effect_input;
struct sentience_meter;
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
struct electric_shield_perk;
struct haste_perk;
struct perk_timing;
struct spell_instance_data;
struct all_simulation_settings;
struct pathfinding_settings;
struct si_scaling;
struct visibility_settings;
template <class key>
struct basic_cosmic_entropy;
struct guid_mapped_entropy;
struct cosmic_entropy;
struct cosmos_flyweights_state;
class cosmos_metadata;
struct cosmos_significant_state;
struct hotbar_settings;
class config_lua_table;
struct neon_map_stamp;
struct scripted_image_stamp;
struct texture_atlas_stamp;
struct texture_atlas_metadata;
struct b2Vec2;
struct b2Rot;
struct b2Transform;
struct b2Sweep;
struct b2Filter;

namespace augs {
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
	template <class Enum, class T>
	class enum_associative_array;
	struct machine_entropy;
	struct stepped_timestamp;
	struct stepped_cooldown;
	template <class A, class B>
	class trivial_pair;
}

namespace components {
	struct animation;
	struct animation_response;
	struct attitude;
	struct behaviour_tree;
	struct car;
	struct catridge;
	struct child;
	struct container;
	struct crosshair;
	struct damage;
	struct driver;
	struct dynamic_tree_node;
	struct fixtures;
	struct flags;
	struct force_joint;
	struct grenade;
	struct guid;
	struct gun;
	struct interpolation;
	struct item;
	struct item_slot_transfers;
	struct light;
	struct melee;
	struct movement;
	struct name;
	struct particles_existence;
	struct particle_effect_response;
	struct physical_relations;
	struct physics;
	struct polygon;
	struct position_copying;
	struct processing;
	struct render;
	struct rotation_copying;
	struct sentience;
	struct sound_existence;
	struct sound_response;
	struct special_physics;
	struct sprite;
	struct substance;
	struct tile_layer_instance;
	struct trace;
	struct transform;
	struct trigger_collision_detector;
	struct trigger;
	struct trigger_query_detector;
	struct wandering_pixels;
}

namespace resources {
	struct state_of_behaviour_tree_instance;
	struct particle_effect_modifier;
	struct emission;
}

namespace augs {
	template <class F, class... MemberInstances>
	void introspect_body(
		const augs::sound_effect_modifier* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("repetitions", _t_.repetitions...);
		f("gain", _t_.gain...);
		f("pitch", _t_.pitch...);
		f("max_distance", _t_.max_distance...);
		f("reference_distance", _t_.reference_distance...);
		f("fade_on_exit", _t_.fade_on_exit...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const rgba* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("r", _t_.r...);
		f("g", _t_.g...);
		f("b", _t_.b...);
		f("a", _t_.a...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const augs::vertex* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("pos", _t_.pos...);
		f("texcoord", _t_.texcoord...);
		f("color", _t_.color...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const augs::font_glyph_metadata* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("adv", _t_.adv...);
		f("bear_x", _t_.bear_x...);
		f("bear_y", _t_.bear_y...);
		f("index", _t_.index...);
		f("unicode", _t_.unicode...);

		f("size", _t_.size...);

		f("kerning", _t_.kerning...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const augs::font_metadata_from_file* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("ascender", _t_.ascender...);
		f("descender", _t_.descender...);

		f("pt", _t_.pt...);

		f("glyphs", _t_.glyphs...);
		f("unicode_to_glyph_index", _t_.unicode_to_glyph_index...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const augs::baked_font* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("meta_from_file", _t_.meta_from_file...);
		f("glyphs_in_atlas", _t_.glyphs_in_atlas...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const augs::font_loading_input* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("path", _t_.path...);
		f("characters", _t_.characters...);
		
		f("pt", _t_.pt...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const augs::paint_circle_midpoint_command* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("radius", _t_.radius...);
		f("border_width", _t_.border_width...);
		f("scale_alpha", _t_.scale_alpha...);
		f("constrain_angle", _t_.constrain_angle...);
		f("angle_start", _t_.angle_start...);
		f("angle_end", _t_.angle_end...);
		f("filling", _t_.filling...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const augs::paint_circle_filled_command* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("radius", _t_.radius...);
		f("filling", _t_.filling...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const augs::paint_line_command* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("from", _t_.from...);
		f("to", _t_.to...);
		f("filling", _t_.filling...);
	}

	template <class F, class T, class... MemberInstances>
	void introspect_body(
		const ltrbt<T>* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("l", _t_.l...);
		f("t", _t_.t...);
		f("r", _t_.r...);
		f("b", _t_.b...);
	}

	template <class F, class T, class... MemberInstances>
	void introspect_body(
		const xywht<T>* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("x", _t_.x...);
		f("y", _t_.y...);
		f("w", _t_.w...);
		f("h", _t_.h...);
	}

	template <class F, class type, class... MemberInstances>
	void introspect_body(
		const vec2t<type>* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("x", _t_.x...);
		f("y", _t_.y...);
	}

	template <class F, class T, size_t const_count, class... MemberInstances>
	void introspect_body(
		const augs::constant_size_vector<T, const_count>* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("count", _t_.count...);
		f("raw", _t_.raw...);
	}

	template <class F, class Enum, class T, class... MemberInstances>
	void introspect_body(
		const augs::enum_associative_array<Enum, T>* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("is_set", _t_.is_set...);
		f("raw", _t_.raw...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const augs::machine_entropy* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("local", _t_.local...);
		f("remote", _t_.remote...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const recoil_player* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("offsets", _t_.offsets...);
		f("current_offset", _t_.current_offset...);
		f("reversed", _t_.reversed...);
		f("repeat_last_n_offsets", _t_.repeat_last_n_offsets...);

		f("single_cooldown_duration_ms", _t_.single_cooldown_duration_ms...);
		f("remaining_cooldown_duration", _t_.remaining_cooldown_duration...);
		f("scale", _t_.scale...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const augs::stepped_timestamp* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("step", _t_.step...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const augs::stepped_cooldown* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("when_last_fired", _t_.when_last_fired...);
		f("cooldown_duration_ms", _t_.cooldown_duration_ms...);
	}

	template <class F, class A, class B, class... MemberInstances>
	void introspect_body(
		const augs::trivial_pair<A, B>* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("first", _t_.first...);
		f("second", _t_.second...);
	}

	template <class F, class T, class... MemberInstances>
	void introspect_body(
		const zeroed_pod<T>* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("pod", _t_.pod...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::animation* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("current_animation", _t_.current_animation...);

		f("priority", _t_.priority...);
		f("frame_num", _t_.frame_num...);
		f("player_position_ms", _t_.player_position_ms...);
		f("speed_factor", _t_.speed_factor...);

		f("state", _t_.state...);
		f("paused_state", _t_.paused_state...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::animation_response* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("response", _t_.response...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::attitude* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("maximum_divergence_angle_before_shooting", _t_.maximum_divergence_angle_before_shooting...);

		f("parties", _t_.parties...);
		f("hostile_parties", _t_.hostile_parties...);

		f("specific_hostile_entities", _t_.specific_hostile_entities...);
		
		f("currently_attacked_visible_entity", _t_.currently_attacked_visible_entity...);
		f("target_attitude", _t_.target_attitude...);

		f("is_alert", _t_.is_alert...);
		f("last_seen_target_position_inspected", _t_.last_seen_target_position_inspected...);

		f("last_seen_target_position", _t_.last_seen_target_position...);
		f("last_seen_target_velocity", _t_.last_seen_target_velocity...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const behaviour_tree_instance* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("state", _t_.state...);
		f("tree_id", _t_.tree_id...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::behaviour_tree* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("concurrent_trees", _t_.concurrent_trees...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const car_engine_entities* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("physical", _t_.physical...);
		f("particles", _t_.particles...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::car* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("current_driver", _t_.current_driver...);

		f("interior", _t_.interior...);

		f("left_wheel_trigger", _t_.left_wheel_trigger...);
		f("right_wheel_trigger", _t_.right_wheel_trigger...);

		f("acceleration_engine", _t_.acceleration_engine...);
		f("deceleration_engine", _t_.deceleration_engine...);

		f("left_engine", _t_.left_engine...);
		f("right_engine", _t_.right_engine...);

		f("engine_sound", _t_.engine_sound...);

		f("accelerating", _t_.accelerating...);
		f("decelerating", _t_.decelerating...);
		f("turning_right", _t_.turning_right...);
		f("turning_left", _t_.turning_left...);

		f("hand_brake", _t_.hand_brake...);
		
		f("braking_damping", _t_.braking_damping...);
		f("braking_angular_damping", _t_.braking_angular_damping...);

		f("input_acceleration", _t_.input_acceleration...);

		f("acceleration_length", _t_.acceleration_length...);

		f("maximum_speed_with_static_air_resistance", _t_.maximum_speed_with_static_air_resistance...);
		f("maximum_speed_with_static_damping", _t_.maximum_speed_with_static_damping...);
		f("static_air_resistance", _t_.static_air_resistance...);
		f("dynamic_air_resistance", _t_.dynamic_air_resistance...);
		f("static_damping", _t_.static_damping...);
		f("dynamic_damping", _t_.dynamic_damping...);

		f("maximum_lateral_cancellation_impulse", _t_.maximum_lateral_cancellation_impulse...);
		f("lateral_impulse_multiplier", _t_.lateral_impulse_multiplier...);

		f("angular_damping", _t_.angular_damping...);
		f("angular_damping_while_hand_braking", _t_.angular_damping_while_hand_braking...);

		f("minimum_speed_for_maneuverability_decrease", _t_.minimum_speed_for_maneuverability_decrease...);
		f("maneuverability_decrease_multiplier", _t_.maneuverability_decrease_multiplier...);

		f("angular_air_resistance", _t_.angular_air_resistance...);
		f("angular_air_resistance_while_hand_braking", _t_.angular_air_resistance_while_hand_braking...);

		f("speed_for_pitch_unit", _t_.speed_for_pitch_unit...);

		f("wheel_offset", _t_.wheel_offset...);

		f("last_turned_on", _t_.last_turned_on...);
		f("last_turned_off", _t_.last_turned_off...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::catridge* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("shell", _t_.shell...);
		f("round", _t_.round...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::child* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("parent", _t_.parent...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::container* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("slots", _t_.slots...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::crosshair* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("orbit_mode", _t_.orbit_mode...);

		f("recoil_entity", _t_.recoil_entity...);

		f("character_entity_to_chase", _t_.character_entity_to_chase...);
		f("base_offset", _t_.base_offset...);
		f("bounds_for_base_offset", _t_.bounds_for_base_offset...);

		f("visible_world_area", _t_.visible_world_area...);
		f("max_look_expand", _t_.max_look_expand...);

		f("rotation_offset", _t_.rotation_offset...);
		f("size_multiplier", _t_.size_multiplier...);
		f("sensitivity", _t_.sensitivity...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::damage* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("amount", _t_.amount...);

		f("impulse_upon_hit", _t_.impulse_upon_hit...);

		f("sender", _t_.sender...);

		f("damage_upon_collision", _t_.damage_upon_collision...);
		f("destroy_upon_damage", _t_.destroy_upon_damage...);
		f("constrain_lifetime", _t_.constrain_lifetime...);
		f("constrain_distance", _t_.constrain_distance...);

		f("damage_charges_before_destruction", _t_.damage_charges_before_destruction...);

		f("custom_impact_velocity", _t_.custom_impact_velocity...);

		f("damage_falloff", _t_.damage_falloff...);

		f("damage_falloff_starting_distance", _t_.damage_falloff_starting_distance...);
		f("minimum_amount_after_falloff", _t_.minimum_amount_after_falloff...);

		f("distance_travelled", _t_.distance_travelled...);
		f("max_distance", _t_.max_distance...);
		f("max_lifetime_ms", _t_.max_lifetime_ms...);
		f("recoil_multiplier", _t_.recoil_multiplier...);

		f("lifetime_ms", _t_.lifetime_ms...);

		f("homing_towards_hostile_strength", _t_.homing_towards_hostile_strength...);
		f("particular_homing_target", _t_.particular_homing_target...);

		f("saved_point_of_impact_before_death", _t_.saved_point_of_impact_before_death...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::driver* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("owned_vehicle", _t_.owned_vehicle...);
		f("density_multiplier_while_driving", _t_.density_multiplier_while_driving...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::dynamic_tree_node* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("always_visible", _t_.always_visible...);
		f("activated", _t_.activated...);
		f("type", _t_.type...);


		f("aabb", _t_.aabb...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const convex_partitioned_collider* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("shape", _t_.shape...);
		f("material", _t_.material...);

		f("collision_sound_gain_mult", _t_.collision_sound_gain_mult...);

		f("density", _t_.density...);
		f("density_multiplier", _t_.density_multiplier...);
		f("friction", _t_.friction...);
		f("restitution", _t_.restitution...);

		f("filter", _t_.filter...);
		f("destructible", _t_.destructible...);
		f("sensor", _t_.sensor...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::fixtures* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("colliders", _t_.colliders...);
		f("offsets_for_created_shapes", _t_.offsets_for_created_shapes...);

		f("activated", _t_.activated...);
		f("is_friction_ground", _t_.is_friction_ground...);
		f("disable_standard_collision_resolution", _t_.disable_standard_collision_resolution...);
		f("can_driver_shoot_through", _t_.can_driver_shoot_through...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::flags* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("bit_flags", _t_.bit_flags...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::force_joint* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("chased_entity", _t_.chased_entity...);

		f("force_towards_chased_entity", _t_.force_towards_chased_entity...);
		f("distance_when_force_easing_starts", _t_.distance_when_force_easing_starts...);
		f("power_of_force_easing_multiplier", _t_.power_of_force_easing_multiplier...);

		f("percent_applied_to_chased_entity", _t_.percent_applied_to_chased_entity...);

		f("divide_transform_mode", _t_.divide_transform_mode...);
		f("consider_rotation", _t_.consider_rotation...);

		f("chased_entity_offset", _t_.chased_entity_offset...);

		f("force_offsets", _t_.force_offsets...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::grenade* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("spoon", _t_.spoon...);
		f("type", _t_.type...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::guid* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("value", _t_.value...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::gun* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("shot_cooldown", _t_.shot_cooldown...);
		f("action_mode", _t_.action_mode...);
		f("num_last_bullets_to_trigger_low_ammo_cue", _t_.num_last_bullets_to_trigger_low_ammo_cue...);

		f("muzzle_velocity", _t_.muzzle_velocity...);

		f("damage_multiplier", _t_.damage_multiplier...);

		f("bullet_spawn_offset", _t_.bullet_spawn_offset...);

		f("camera_shake_radius", _t_.camera_shake_radius...);
		f("camera_shake_spread_degrees", _t_.camera_shake_spread_degrees...);

		f("trigger_pressed", _t_.trigger_pressed...);

		f("shell_velocity", _t_.shell_velocity...);
		f("shell_angular_velocity", _t_.shell_angular_velocity...);

		f("shell_spread_degrees", _t_.shell_spread_degrees...);

		f("recoil", _t_.recoil...);

		f("shell_spawn_offset", _t_.shell_spawn_offset...);

		f("magic_missile_definition", _t_.magic_missile_definition...);

		f("current_heat", _t_.current_heat...);
		f("gunshot_adds_heat", _t_.gunshot_adds_heat...);
		f("maximum_heat", _t_.maximum_heat...);
		f("engine_sound_strength", _t_.engine_sound_strength...);

		f("firing_engine_sound", _t_.firing_engine_sound...);
		f("muzzle_particles", _t_.muzzle_particles...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::interpolation* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("base_exponent", _t_.base_exponent...);
		f("place_of_birth", _t_.place_of_birth...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::item* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("current_mounting", _t_.current_mounting...);
		f("intended_mounting", _t_.intended_mounting...);

		f("categories_for_slot_compatibility", _t_.categories_for_slot_compatibility...);

		f("charges", _t_.charges...);
		f("space_occupied_per_charge", _t_.space_occupied_per_charge...);
		f("stackable", _t_.stackable...);

		f("dual_wield_accuracy_loss_percentage", _t_.dual_wield_accuracy_loss_percentage...);
		f("dual_wield_accuracy_loss_multiplier", _t_.dual_wield_accuracy_loss_multiplier...);

		f("current_slot", _t_.current_slot...);
		f("target_slot_after_unmount", _t_.target_slot_after_unmount...);

		f("montage_time_ms", _t_.montage_time_ms...);
		f("montage_time_left_ms", _t_.montage_time_left_ms...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const item_slot_mounting_operation* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("current_item", _t_.current_item...);
		f("intented_mounting_slot", _t_.intented_mounting_slot...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::item_slot_transfers* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("pickup_timeout", _t_.pickup_timeout...);
		f("mounting", _t_.mounting...);

		f("only_pick_these_items", _t_.only_pick_these_items...);
		f("pick_all_touched_items_if_list_to_pick_empty", _t_.pick_all_touched_items_if_list_to_pick_empty...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const light_value_variation* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("min_value", _t_.min_value...);
		f("max_value", _t_.max_value...);
		f("change_speed", _t_.change_speed...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const light_attenuation* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("base_value", _t_.base_value...);
		f("variation", _t_.variation...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::light* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("color", _t_.color...);

		f("constant", _t_.constant...);
		f("linear", _t_.linear...);
		f("quadratic", _t_.quadratic...);
		f("max_distance", _t_.max_distance...);
		
		f("wall_constant", _t_.wall_constant...);
		f("wall_linear", _t_.wall_linear...);
		f("wall_quadratic", _t_.wall_quadratic...);
		f("wall_max_distance", _t_.wall_max_distance...);

		f("position_variations", _t_.position_variations...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::melee* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("primary_move_flag", _t_.primary_move_flag...);
		f("secondary_move_flag", _t_.secondary_move_flag...);
		f("tertiary_move_flag", _t_.tertiary_move_flag...);

		f("current_state", _t_.current_state...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const movement_subscribtion* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("target", _t_.target...);
		f("stop_response_at_zero_speed", _t_.stop_response_at_zero_speed...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::movement* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("response_receivers", _t_.response_receivers...);
		
		f("moving_left", _t_.moving_left...);
		f("moving_right", _t_.moving_right...);
		f("moving_forward", _t_.moving_forward...);
		f("moving_backward", _t_.moving_backward...);

		f("walking_enabled", _t_.walking_enabled...);
		f("enable_braking_damping", _t_.enable_braking_damping...);
		f("enable_animation", _t_.enable_animation...);
		f("sprint_enabled", _t_.sprint_enabled...);

		f("input_acceleration_axes", _t_.input_acceleration_axes...);
		f("acceleration_length", _t_.acceleration_length...);

		f("applied_force_offset", _t_.applied_force_offset...);

		f("non_braking_damping", _t_.non_braking_damping...);
		f("braking_damping", _t_.braking_damping...);

		f("standard_linear_damping", _t_.standard_linear_damping...);

		f("make_inert_for_ms", _t_.make_inert_for_ms...);
		f("max_speed_for_movement_response", _t_.max_speed_for_movement_response...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::name* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("id", _t_.id...);

		f("custom_nickname", _t_.custom_nickname...);
		f("nickname", _t_.nickname...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const particles_effect_input* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("effect", _t_.effect...);
		f("delete_entity_after_effect_lifetime", _t_.delete_entity_after_effect_lifetime...);

		f("modifier", _t_.modifier...);

		f("displace_source_position_within_radius", _t_.displace_source_position_within_radius...);
		f("single_displacement_duration_ms", _t_.single_displacement_duration_ms...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::particles_existence* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("input", _t_.input...);

		f("current_displacement", _t_.current_displacement...);
		f("time_of_last_displacement", _t_.time_of_last_displacement...);
		f("current_displacement_duration_bound_ms", _t_.current_displacement_duration_bound_ms...);

		f("time_of_birth", _t_.time_of_birth...);
		f("max_lifetime_in_steps", _t_.max_lifetime_in_steps...);

		f("distribute_within_segment_of_length", _t_.distribute_within_segment_of_length...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::particle_effect_response* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("response", _t_.response...);
		f("modifier", _t_.modifier...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::physical_relations* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("owner_body", _t_.owner_body...);
		f("fixture_entities", _t_.fixture_entities...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::physics* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("fixed_rotation", _t_.fixed_rotation...);
		f("bullet", _t_.bullet...);
		f("angled_damping", _t_.angled_damping...);
		f("activated", _t_.activated...);

		f("body_type", _t_.body_type...);

		f("angular_damping", _t_.angular_damping...);
		f("linear_damping", _t_.linear_damping...);
		f("linear_damping_vec", _t_.linear_damping_vec...);
		f("gravity_scale", _t_.gravity_scale...);

		f("transform", _t_.transform...);
		f("sweep", _t_.sweep...);

		f("velocity", _t_.velocity...);
		f("angular_velocity", _t_.angular_velocity...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::polygon* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("center_neon_map", _t_.center_neon_map...);
		f("vertices", _t_.vertices...);
		f("triangulation_indices", _t_.triangulation_indices...);

	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::position_copying* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("target", _t_.target...);

		f("offset", _t_.offset...);
		f("rotation_orbit_offset", _t_.rotation_orbit_offset...);
		
		f("reference_position", _t_.reference_position...);
		f("target_reference_position", _t_.target_reference_position...);
		
		f("scrolling_speed", _t_.scrolling_speed...);

		f("rotation_offset", _t_.rotation_offset...);
		f("rotation_multiplier", _t_.rotation_multiplier...);

		f("position_copying_mode", _t_.position_copying_mode...);
		f("position_copying_rotation", _t_.position_copying_rotation...);
		f("track_origin", _t_.track_origin...);
		f("target_newly_set", _t_.target_newly_set...);
		f("previous", _t_.previous...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::processing* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("activated", _t_.activated...);

		f("processing_subject_categories", _t_.processing_subject_categories...);
		f("disabled_categories", _t_.disabled_categories...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::render* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("screen_space_transform", _t_.screen_space_transform...);
		f("draw_border", _t_.draw_border...);
		f("layer", _t_.layer...);

		f("border_color", _t_.border_color...);

		f("last_step_when_visible", _t_.last_step_when_visible...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::rotation_copying* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("target", _t_.target...);
		f("stashed_target", _t_.stashed_target...);

		f("easing_mode", _t_.easing_mode...);

		f("colinearize_item_in_hand", _t_.colinearize_item_in_hand...);
		f("update_value", _t_.update_value...);
		
		f("smoothing_average_factor", _t_.smoothing_average_factor...);
		f("averages_per_sec", _t_.averages_per_sec...);
		
		f("last_rotation_interpolant", _t_.last_rotation_interpolant...);

		f("look_mode", _t_.look_mode...);
		f("stashed_look_mode", _t_.stashed_look_mode...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const sentience_meter* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("enabled", _t_.enabled...);

		f("value", _t_.value...);
		f("maximum", _t_.maximum...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::sentience* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("time_of_last_received_damage", _t_.time_of_last_received_damage...);
		f("time_of_last_exertion", _t_.time_of_last_exertion...);

		f("cast_cooldown_for_all_spells", _t_.cast_cooldown_for_all_spells...);

		f("health", _t_.health...);
		f("personal_electricity", _t_.personal_electricity...);
		f("consciousness", _t_.consciousness...);

		f("haste", _t_.haste...);
		f("electric_shield", _t_.electric_shield...);

		f("spells", _t_.spells...);

		f("currently_casted_spell", _t_.currently_casted_spell...);
		f("transform_when_spell_casted", _t_.transform_when_spell_casted...);
		f("time_of_last_spell_cast", _t_.time_of_last_spell_cast...);
		f("time_of_last_exhausted_cast", _t_.time_of_last_exhausted_cast...);

		f("time_of_last_shake", _t_.time_of_last_shake...);
		f("shake_for_ms", _t_.shake_for_ms...);

		f("comfort_zone", _t_.comfort_zone...);
		f("minimum_danger_amount_to_evade", _t_.minimum_danger_amount_to_evade...);
		f("danger_amount_from_hostile_attitude", _t_.danger_amount_from_hostile_attitude...);

		f("aimpunch", _t_.aimpunch...);
		f("health_damage_particles", _t_.health_damage_particles...);
		f("character_crosshair", _t_.character_crosshair...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const sound_effect_input* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("effect", _t_.effect...);
		f("delete_entity_after_effect_lifetime", _t_.delete_entity_after_effect_lifetime...);
		f("variation_number", _t_.variation_number...);
		f("direct_listener", _t_.direct_listener...);
		f("modifier", _t_.modifier...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::sound_existence* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("input", _t_.input...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::sound_response* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("response", _t_.response...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const friction_connection* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("target", _t_.target...);
		f("fixtures_connected", _t_.fixtures_connected...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::special_physics* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("dropped_collision_cooldown", _t_.dropped_collision_cooldown...);
		f("owner_friction_ground", _t_.owner_friction_ground...);
		f("owner_friction_grounds", _t_.owner_friction_grounds...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::sprite* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("tex", _t_.tex...);
		f("color", _t_.color...);
		f("size", _t_.size...);
		f("size_multiplier", _t_.size_multiplier...);
		f("center_offset", _t_.center_offset...);
		f("rotation_offset", _t_.rotation_offset...);

		f("flip_horizontally", _t_.flip_horizontally...);
		f("flip_vertically", _t_.flip_vertically...);
		
		f("effect", _t_.effect...);
		f("has_neon_map", _t_.has_neon_map...);

		f("max_specular_blinks", _t_.max_specular_blinks...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::substance* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("dummy", _t_.dummy...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::tile_layer_instance* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("id", _t_.id...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::trace* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("max_multiplier_x", _t_.max_multiplier_x...);
		f("max_multiplier_y", _t_.max_multiplier_y...);

		f("chosen_multiplier", _t_.chosen_multiplier...);

		f("lengthening_duration_ms", _t_.lengthening_duration_ms...);
		f("chosen_lengthening_duration_ms", _t_.chosen_lengthening_duration_ms...);
		f("lengthening_time_passed_ms", _t_.lengthening_time_passed_ms...);

		f("is_it_finishing_trace", _t_.is_it_finishing_trace...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::transform* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("pos", _t_.pos...);
		f("rotation", _t_.rotation...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::trigger_collision_detector* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("detection_intent_enabled", _t_.detection_intent_enabled...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::trigger* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("entity_to_be_notified", _t_.entity_to_be_notified...);
		f("react_to_collision_detectors", _t_.react_to_collision_detectors...);
		f("react_to_query_detectors", _t_.react_to_query_detectors...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::trigger_query_detector* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("detection_intent_enabled", _t_.detection_intent_enabled...);
		f("spam_trigger_requests_when_detection_intented", _t_.spam_trigger_requests_when_detection_intented...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const components::wandering_pixels* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("reach", _t_.reach...);
		f("face", _t_.face...);
		f("count", _t_.count...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const convex_poly_destruction_scar* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("first_impact", _t_.first_impact...);
		f("depth_point", _t_.depth_point...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const convex_poly_destruction_data* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("scars", _t_.scars...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const convex_poly* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("vertices", _t_.vertices...);

		f("destruction", _t_.destruction...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const convex_partitioned_shape* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("convex_polys", _t_.convex_polys...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const inventory_slot* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("category_allowed", _t_.category_allowed...);

		f("items_need_mounting", _t_.items_need_mounting...);
		f("only_last_inserted_is_movable", _t_.only_last_inserted_is_movable...);

		f("for_categorized_items_only", _t_.for_categorized_items_only...);

		f("is_physical_attachment_slot", _t_.is_physical_attachment_slot...);
		f("always_allow_exactly_one_item", _t_.always_allow_exactly_one_item...);


		f("montage_time_multiplier", _t_.montage_time_multiplier...);

		f("space_available", _t_.space_available...);

		f("attachment_density_multiplier", _t_.attachment_density_multiplier...);

		f("attachment_sticking_mode", _t_.attachment_sticking_mode...);
		f("attachment_offset", _t_.attachment_offset...);

		f("items_inside", _t_.items_inside...);
	}

	template <class F, class id_type, class... MemberInstances>
	void introspect_body(
		const basic_inventory_slot_id<id_type>* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("type", _t_.type...);
		f("container_entity", _t_.container_entity...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const inventory_item_address* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("root_container", _t_.root_container...);
		f("directions", _t_.directions...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const inventory_traversal* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("parent_slot", _t_.parent_slot...);
		f("current_address", _t_.current_address...);
		f("attachment_offset", _t_.attachment_offset...);
		f("item_remains_physical", _t_.item_remains_physical...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const electric_shield_perk* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("timing", _t_.timing...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const haste_perk* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("timing", _t_.timing...);
		f("is_greater", _t_.is_greater...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const perk_timing* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("duration", _t_.duration...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const spell_instance_data* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("cast_cooldown", _t_.cast_cooldown...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const resources::state_of_behaviour_tree_instance* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("previously_executed_leaf_id", _t_.previously_executed_leaf_id...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const resources::particle_effect_modifier* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("colorize", _t_.colorize...);
		f("scale_amounts", _t_.scale_amounts...);
		f("scale_lifetimes", _t_.scale_lifetimes...);
		f("homing_target", _t_.homing_target...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const resources::emission* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("spread_degrees", _t_.spread_degrees...);
		f("base_speed", _t_.base_speed...);
		f("base_speed_variation", _t_.base_speed_variation...);
		f("rotation_speed", _t_.rotation_speed...);
		f("particles_per_sec", _t_.particles_per_sec...);
		f("stream_lifetime_ms", _t_.stream_lifetime_ms...);
		f("particle_lifetime_ms", _t_.particle_lifetime_ms...);
		f("size_multiplier", _t_.size_multiplier...);
		f("acceleration", _t_.acceleration...);
		f("angular_offset", _t_.angular_offset...);
		f("swing_spread", _t_.swing_spread...);
		f("swings_per_sec", _t_.swings_per_sec...);
		f("min_swing_spread", _t_.min_swing_spread...);
		f("max_swing_spread", _t_.max_swing_spread...);
		f("min_swings_per_sec", _t_.min_swings_per_sec...);
		f("max_swings_per_sec", _t_.max_swings_per_sec...);
		f("swing_spread_change_rate", _t_.swing_spread_change_rate...);
		f("swing_speed_change_rate", _t_.swing_speed_change_rate...);
		f("fade_when_ms_remaining", _t_.fade_when_ms_remaining...);
		f("num_of_particles_to_spawn_initially", _t_.num_of_particles_to_spawn_initially...);

		f("randomize_spawn_point_within_circle_of_outer_radius", _t_.randomize_spawn_point_within_circle_of_outer_radius...);
		f("randomize_spawn_point_within_circle_of_inner_radius", _t_.randomize_spawn_point_within_circle_of_inner_radius...);

		f("starting_spawn_circle_size_multiplier", _t_.starting_spawn_circle_size_multiplier...);
		f("ending_spawn_circle_size_multiplier", _t_.ending_spawn_circle_size_multiplier...);

		f("starting_homing_force", _t_.starting_homing_force...);
		f("ending_homing_force", _t_.ending_homing_force...);

		f("homing_target", _t_.homing_target...);

		f("initial_rotation_variation", _t_.initial_rotation_variation...);
		f("randomize_acceleration", _t_.randomize_acceleration...);
		f("should_particles_look_towards_velocity", _t_.should_particles_look_towards_velocity...);

		f("particle_templates", _t_.particle_templates...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const all_simulation_settings* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("visibility", _t_.visibility...);
		f("pathfinding", _t_.pathfinding...);
		f("si", _t_.si...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const pathfinding_settings* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("epsilon_distance_visible_point", _t_.epsilon_distance_visible_point...);
		f("epsilon_distance_the_same_vertex", _t_.epsilon_distance_the_same_vertex...);

		f("draw_memorised_walls", _t_.draw_memorised_walls...);
		f("draw_undiscovered", _t_.draw_undiscovered...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const si_scaling* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("to_meters_multiplier", _t_.to_meters_multiplier...);
		f("to_pixels_multiplier", _t_.to_pixels_multiplier...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const visibility_settings* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("draw_triangle_edges", _t_.draw_triangle_edges...);
		f("draw_cast_rays", _t_.draw_cast_rays...);
		f("draw_discontinuities", _t_.draw_discontinuities...);
		f("draw_visible_walls", _t_.draw_visible_walls...);

		f("epsilon_ray_distance_variation", _t_.epsilon_ray_distance_variation...);
		f("epsilon_distance_vertex_hit", _t_.epsilon_distance_vertex_hit...);
		f("epsilon_threshold_obstacle_hit", _t_.epsilon_threshold_obstacle_hit...);
	}

	template <class F, class key, class... MemberInstances>
	void introspect_body(
		const basic_cosmic_entropy<key>* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("cast_spells", _t_.cast_spells...);
		f("intents_per_entity", _t_.intents_per_entity...);
		f("transfer_requests", _t_.transfer_requests...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const guid_mapped_entropy* const,
		F f,
		MemberInstances&&... _t_
	) {
		introspect_body(static_cast<basic_cosmic_entropy<entity_guid>*>(nullptr), f, std::forward<MemberInstances>(_t_)...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const cosmic_entropy* const,
		F f,
		MemberInstances&&... _t_
	) {
		introspect_body(static_cast<basic_cosmic_entropy<entity_id>*>(nullptr), f, std::forward<MemberInstances>(_t_)...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const cosmos_flyweights_state* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("spells", _t_.spells...);
		f("collision_sound_matrix", _t_.collision_sound_matrix...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const cosmos_metadata* const,
		F f,
		MemberInstances&&... _t_
	) {

		f("delta", _t_.delta...);
		f("total_steps_passed", _t_.total_steps_passed...);

#if COSMOS_TRACKS_GUIDS
		f("next_entity_guid", _t_.next_entity_guid...);
#endif
		f("settings", _t_.settings...);

		f("flyweights", _t_.flyweights...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const cosmos_significant_state* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("meta", _t_.meta...);

		f("pool_for_aggregates", _t_.pool_for_aggregates...);
		f("pools_for_components", _t_.pools_for_components...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const hotbar_settings* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("hotbar_increase_inside_alpha_when_selected", _t_.hotbar_increase_inside_alpha_when_selected...);
		f("hotbar_colorize_inside_when_selected", _t_.hotbar_colorize_inside_when_selected...);

		f("hotbar_primary_selected_color", _t_.hotbar_primary_selected_color...);
		f("hotbar_secondary_selected_color", _t_.hotbar_secondary_selected_color...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const config_lua_table* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("launch_mode", _t_.launch_mode...);
		f("input_recording_mode", _t_.input_recording_mode...);

		f("recording_replay_speed", _t_.recording_replay_speed...);

		f("determinism_test_cloned_cosmoi_count", _t_.determinism_test_cloned_cosmoi_count...);

		f("check_content_integrity_every_launch", _t_.check_content_integrity_every_launch...);
		f("save_regenerated_atlases_as_binary", _t_.save_regenerated_atlases_as_binary...);
		f("debug_regenerate_content_every_launch", _t_.debug_regenerate_content_every_launch...);

		f("enable_hrtf", _t_.enable_hrtf...);
		f("max_number_of_sound_sources", _t_.max_number_of_sound_sources...);

		f("audio_output_device", _t_.audio_output_device...);

		f("sound_effects_volume", _t_.sound_effects_volume...);
		f("music_volume", _t_.music_volume...);

		f("debug_disable_cursor_clipping", _t_.debug_disable_cursor_clipping...);

		f("connect_address", _t_.connect_address...);
		f("connect_port", _t_.connect_port...);
		f("server_port", _t_.server_port...);
		f("alternative_port", _t_.alternative_port...);

		f("nickname", _t_.nickname...);
		f("debug_second_nickname", _t_.debug_second_nickname...);

		f("mouse_sensitivity", _t_.mouse_sensitivity...);

		f("tickrate", _t_.tickrate...);

		f("jitter_buffer_ms", _t_.jitter_buffer_ms...);
		f("client_commands_jitter_buffer_ms", _t_.client_commands_jitter_buffer_ms...);

		f("interpolation_speed", _t_.interpolation_speed...);
		f("misprediction_smoothing_multiplier", _t_.misprediction_smoothing_multiplier...);

		f("debug_var", _t_.debug_var...);
		f("debug_randomize_entropies_in_client_setup", _t_.debug_randomize_entropies_in_client_setup...);
		f("debug_randomize_entropies_in_client_setup_once_every_steps", _t_.debug_randomize_entropies_in_client_setup_once_every_steps...);

		f("server_launch_http_daemon", _t_.server_launch_http_daemon...);
		f("server_http_daemon_port", _t_.server_http_daemon_port...);
		f("server_http_daemon_html_file_path", _t_.server_http_daemon_html_file_path...);

		f("db_path", _t_.db_path...);
		f("survey_num_file_path", _t_.survey_num_file_path...);
		f("post_data_file_path", _t_.post_data_file_path...);
		f("last_session_update_link", _t_.last_session_update_link...);

		f("director_scenario_filename", _t_.director_scenario_filename...);
		f("menu_intro_scenario_filename", _t_.menu_intro_scenario_filename...);

		f("menu_theme_filename", _t_.menu_theme_filename...);

		f("rewind_intro_scene_by_secs", _t_.rewind_intro_scene_by_secs...);
		f("start_menu_music_at_secs", _t_.start_menu_music_at_secs...);

		f("skip_credits", _t_.skip_credits...);
		f("latest_news_url", _t_.latest_news_url...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const neon_map_stamp* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("standard_deviation", _t_.standard_deviation...);
		f("radius_towards_x_axis", _t_.radius_towards_x_axis...);
		f("radius_towards_y_axis", _t_.radius_towards_y_axis...);
		f("amplification", _t_.amplification...);
		f("alpha_multiplier", _t_.alpha_multiplier...);
		f("last_write_time_of_source", _t_.last_write_time_of_source...);

		f("light_colors", _t_.light_colors...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const scripted_image_stamp* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("commands", _t_.commands...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const texture_atlas_stamp* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("image_stamps", _t_.image_stamps...);
		f("font_stamps", _t_.font_stamps...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const texture_atlas_metadata* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("atlas_image_size", _t_.atlas_image_size...);

		f("images", _t_.images...);
		f("fonts", _t_.fonts...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const b2Vec2* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("x", _t_.x...);
		f("y", _t_.y...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const b2Rot* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("s", _t_.s...);
		f("c", _t_.c...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const b2Transform* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("p", _t_.p...);
		f("q", _t_.q...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const b2Sweep* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("localCenter", _t_.localCenter...);
		f("c0", _t_.c0...);
		f("c", _t_.c...);
		f("a0", _t_.a0...);
		f("a", _t_.a...);
		f("alpha0", _t_.alpha0...);
	}

	template <class F, class... MemberInstances>
	void introspect_body(
		const b2Filter* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("categoryBits", _t_.categoryBits...);
		f("maskBits", _t_.maskBits...);
		f("groupIndex", _t_.groupIndex...);
	}

}