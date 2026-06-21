#pragma once
#include "test_scenes/test_scene_sounds.h"

void create_sounds(const intercosm& scene, editor_resource_pools& pools) {
	auto& sounds = scene.viewables.sounds;
	auto& pool = pools.template get_pool_for<editor_sound_resource>();

	{
		using test_id_type = test_sound_decorations;

		augs::for_each_enum_except_bounds([&](const test_id_type enum_id) {
			const auto flavour_id = to_entity_flavour_id(enum_id);
			const auto& sound = scene.world.get_flavour(flavour_id).template get<invariants::continuous_sound>();
			const auto sound_id = sound.effect.id;

			if (sound_id == to_sound_id(test_scene_sound_id::BLANK)) {
				return;
			}

			const auto path = sounds[sound_id].source_sound.path;
			
			auto res = editor_sound_resource(editor_pathed_resource(path, "", {}));
			res.official_tag = enum_id;
			res.scene_flavour_id = flavour_id;
			res.scene_asset_id = sound_id;
			res.cached_official_name = to_lowercase(augs::enum_to_string(enum_id));

			/* Might come in handy */

			if (sound.effect.modifier.max_distance >= 0) {
				res.editable.max_distance = sound.effect.modifier.max_distance;
			}

			if (sound.effect.modifier.reference_distance >= 0) {
				res.editable.reference_distance = sound.effect.modifier.reference_distance;
			}

			res.editable.distance_model = sound.effect.modifier.distance_model;

			res.editable.gain = sound.effect.modifier.gain;
			res.editable.pitch = sound.effect.modifier.pitch;

			pool.allocate(res);
		});
	}
}

