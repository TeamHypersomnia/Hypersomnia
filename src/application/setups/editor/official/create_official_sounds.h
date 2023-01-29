#pragma once
#include "test_scenes/test_scene_sounds.h"

void create_sounds(const intercosm& scene, editor_resource_pools& pools) {
	auto& sounds = scene.viewables.sounds;
	auto& pool = pools.template get_pool_for<editor_sound_resource>();

	{
		using test_id_type = test_sound_decorations;

		augs::for_each_enum_except_bounds([&](const test_id_type enum_id) {
			const auto flavour_id = to_entity_flavour_id(enum_id);
			const auto sound_id = scene.world.get_flavour(flavour_id).template get<invariants::continuous_sound>().effect.id;

			if (sound_id == to_sound_id(test_scene_sound_id::BLANK)) {
				return;
			}

			const auto path = sounds[sound_id].source_sound.path;
			
			auto res = editor_sound_resource(editor_pathed_resource(path, "", {}));
			res.official_tag = enum_id;
			res.scene_flavour_id = flavour_id;
			res.scene_asset_id = sound_id;
			res.overridden_official_name = to_lowercase(augs::enum_to_string(enum_id));
			pool.allocate(res);
		});
	}

#if CREATE_OFFICIAL_CONTENT_ON_EDITOR_LEVEL
	auto create_sound = [&](const official_sounds id) -> auto& {
		return create_official(id, pools).editable;
	};

	{
		auto& loudy_fan = create_sound(official_sounds::LOUDY_FAN);

		loudy_fan.distance_model = augs::distance_model::LINEAR_DISTANCE_CLAMPED;
		loudy_fan.reference_distance = 20.0f;
		loudy_fan.max_distance = 400.0f;
		loudy_fan.doppler_factor = 1.0f;
		loudy_fan.gain = 0.5f;
	}

	create_sound(official_sounds::AQUARIUM_AMBIENCE_LEFT);
	create_sound(official_sounds::AQUARIUM_AMBIENCE_RIGHT);

	create_sound(official_sounds::GLASS_DAMAGE);
	create_sound(official_sounds::COLLISION_GLASS);
#endif
}

