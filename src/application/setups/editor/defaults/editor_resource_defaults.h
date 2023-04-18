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
