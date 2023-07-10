#pragma once

template <class R>
void setup_resource_defaults(
	R& resource,
	const editor_official_resource_map& o
) {
	if constexpr(std::is_same_v<editor_sprite_resource_editable, R>) {
		resource.as_physical.material = o[test_scene_physical_material_id::METAL];
	}
}

template <class F>
void setup_resource_defaults_after_creating_officials(
	F find_resource,
	editor_official_resource_map& o
) {
	if (auto portal = find_resource(o[area_marker_type::PORTAL])) {
		portal->editable.node_defaults.shape = marker_shape_type::CIRCLE;
		portal->editable.node_defaults.size = vec2(768, 768);

		auto& to = portal->editable.node_defaults.as_portal;

		to.ambience_particles.id = o[test_particles_decorations::PORTAL_CIRCLE];
		to.ambience_sound.id = o[test_sound_decorations::PORTAL_AMBIENCE];

		to.begin_entering_sound.id = o[test_sound_decorations::PORTAL_BEGIN_ENTERING_SOUND];
		to.enter_sound.id = o[test_sound_decorations::PORTAL_ENTER_SOUND];
		to.exit_sound.id = o[test_sound_decorations::PORTAL_EXIT_SOUND];

		to.begin_entering_particles.id = o[test_particles_decorations::PORTAL_BEGIN_ENTERING_PARTICLES];
		to.enter_particles.id = o[test_particles_decorations::PORTAL_ENTER_PARTICLES];
		to.exit_particles.id = o[test_particles_decorations::PORTAL_EXIT_PARTICLES];
	}	

	if (auto portal = find_resource(o[area_marker_type::HAZARD])) {
		portal->editable.node_defaults.shape = marker_shape_type::BOX;
		portal->editable.node_defaults.size = vec2(128, 128);

		auto& to = portal->editable.node_defaults.as_portal;

		to.exit_particles.id = o[test_particles_decorations::PORTAL_EXIT_PARTICLES];
		to.exit_sound.id = o[test_sound_decorations::FIRE_DAMAGE];

		to.ambience_particles.id = o[test_particles_decorations::LAVA_CIRCLE];
		to.ambience_sound.id = o[test_sound_decorations::LAVA_AMBIENCE];
		to.ambience_sound_distance_mult = 3.0f;

		to.reacts_to.bullets = false;
		to.reacts_to.flying_explosives  = false;
		to.reacts_to.flying_melees = false;
		to.reacts_to.character_weapons = false;
		to.reacts_to.lying_items = false;
		to.reacts_to.shells = false;

		to.ignore_airborne_characters = true;

		to.trampoline_like = true;
		to.color_preset = editor_color_preset::LAVA;

		to.exit_impulses.character_exit_impulse.amount /= 3;
		to.exit_impulses.object_exit_impulse.amount /= 3;
		to.exit_direction = portal_exit_direction::REVERSE_ENTERING_VELOCITY;

		to.hazard.emplace(hazard_def());
		to.rings_effect.is_enabled = false;
	}
}
