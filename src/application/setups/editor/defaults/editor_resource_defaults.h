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
		auto& to = portal->editable.node_defaults.as_portal;

		to.ambience_particles.id = o[test_particles_decorations::PORTAL_CIRCLE];
		to.ambience_sound.id = o[test_sound_decorations::PORTAL_AMBIENCE];

		to.begin_entering_sound.id = o[test_sound_decorations::PORTAL_BEGIN_ENTERING];
		to.enter_sound.id = o[test_sound_decorations::PORTAL_ENTER];
		to.exit_sound.id = o[test_sound_decorations::PORTAL_EXIT];

		to.enter_particles.id = o[test_particles_decorations::EXHAUSTED_SMOKE];
		to.exit_particles.id = o[test_particles_decorations::DASH_SMOKE];
	}	
}
