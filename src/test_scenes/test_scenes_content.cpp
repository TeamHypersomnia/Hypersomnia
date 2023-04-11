#include "augs/log_direct.h"
#include "game/assets/all_logical_assets.h"
#include "game/cosmos/cosmos_common_significant.h"
#include "view/viewables/all_viewables_defs.h"
#include "test_scenes/test_scenes_content.h"
#include "test_scenes/test_scene_flavours.h"
#include "test_scenes/test_scene_sounds.h"
#include "test_scenes/test_scene_particle_effects.h"
#include "view/viewables/image_in_atlas.h"
#include "view/viewables/image_cache.h"

#if BUILD_TEST_SCENES
loaded_image_caches_map populate_test_scene_images_and_sounds(
	sol::state& lua,
	all_viewables_defs& output_sources
) {
	auto& definitions = output_sources.image_definitions;

	try {
		load_test_scene_images(lua, definitions);
		load_test_scene_sounds(output_sources.sounds);
	}
	catch (const test_scene_asset_loading_error& err) {
		LOG_NOFORMAT(err.what());
	}

	loaded_image_caches_map out;

	for_each_id_and_object(definitions,
		[&](const auto id, const auto& object) {
			out.try_emplace(id, image_cache(image_definition_view({}, object)));
		}
	);

	return out;
}

void populate_test_scene_logical_assets(
	const image_definitions_map& images,
	all_logical_assets& output_logicals
) {
	load_test_scene_animations(images, output_logicals);
	load_test_scene_physical_materials(output_logicals.physical_materials);
	load_test_scene_recoil_players(output_logicals.recoils);
}

void populate_test_scene_viewables(
	const loaded_image_caches_map& caches,
	const plain_animations_pool& anims,
	all_viewables_defs& output_sources
) {
	try {
		load_test_scene_particle_effects(
			caches,
			anims,
			output_sources.particle_effects
		);
	}
	catch (const test_scene_asset_loading_error& err) {
		LOG_NOFORMAT(err.what());
	}
}

void populate_test_scene_common(const loaded_image_caches_map& caches, cosmos_common_significant& common) {
	populate_test_scene_flavours({ caches, common.logical_assets.plain_animations, common.flavours });

	common.light.ambient_color = rgba(53, 97, 102, 255); // Brighten it up a little

	{
		auto& defs = common.default_sound_properties;

		defs.max_distance = 4000.f;
		defs.reference_distance = 1000.f;
		defs.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
		defs.basic_nonlinear_rolloff = 1.f;
		defs.air_absorption = 2.f;
	}

	auto& common_assets = common.assets;
	common_assets.item_holster_sound.id = to_sound_id(test_scene_sound_id::STANDARD_HOLSTER);
	common_assets.item_pickup_to_deposit_sound.id = to_sound_id(test_scene_sound_id::BACKPACK_INSERT);
	common_assets.item_pickup_particles.id = to_particle_effect_id(test_scene_particle_effect_id::PICKUP_SPARKLES);

	common_assets.standard_footstep.sound.id = to_sound_id(test_scene_sound_id::STANDARD_FOOTSTEP);
	common_assets.standard_footstep.sound.modifier.pitch = 1.25f;
	common_assets.standard_footstep.sound.modifier.gain = 0.4;
	common_assets.standard_footstep.sound.modifier.max_distance = 2500.f;
	common_assets.standard_footstep.sound.modifier.reference_distance = 1000.f;
	common_assets.cast_unsuccessful_sound.id = to_sound_id(test_scene_sound_id::CAST_UNSUCCESSFUL);
	common_assets.ped_shield_impact_sound.id = to_sound_id(test_scene_sound_id::EXPLOSION);
	common_assets.ped_shield_destruction_sound.id = to_sound_id(test_scene_sound_id::GREAT_EXPLOSION);
	common_assets.item_throw_sound.id = to_sound_id(test_scene_sound_id::ITEM_THROW);
	common_assets.item_throw_sound.modifier.pitch = 1.15f;
	common_assets.item_throw_sound.modifier.gain = 0.8f;

	common_assets.standard_footstep.particles.id = to_particle_effect_id(test_scene_particle_effect_id::FOOTSTEP_SMOKE);
	common_assets.exhausted_smoke_particles.id = to_particle_effect_id(test_scene_particle_effect_id::EXHAUSTED_SMOKE);
	common_assets.exploding_ring_smoke = to_particle_effect_id(test_scene_particle_effect_id::EXPLODING_RING_SMOKE);
	common_assets.exploding_ring_sparkles = to_particle_effect_id(test_scene_particle_effect_id::EXPLODING_RING_SPARKLES);
	common_assets.thunder_remnants = to_particle_effect_id(test_scene_particle_effect_id::THUNDER_REMNANTS);

	common_assets.haste_footstep_particles.id = to_particle_effect_id(test_scene_particle_effect_id::HASTE_FOOTSTEP);
	common_assets.broken_shield_icon = to_image_id(test_scene_image_id::BROKEN_SHIELD_ICON);

	common_assets.head_icons[faction_type::RESISTANCE] = to_image_id(test_scene_image_id::RESISTANCE_HEAD_ICON);
	common_assets.head_icons[faction_type::METROPOLIS] = to_image_id(test_scene_image_id::METROPOLIS_HEAD_ICON);
	common_assets.broken_head_icons[faction_type::RESISTANCE] = to_image_id(test_scene_image_id::RESISTANCE_BROKEN_HEAD_ICON);
	common_assets.broken_head_icons[faction_type::METROPOLIS] = to_image_id(test_scene_image_id::METROPOLIS_BROKEN_HEAD_ICON);

	common_assets.standard_learnt_spell_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STANDARD_LEARNT_SPELL);
	common_assets.standard_learnt_spell_sound.id = to_sound_id(test_scene_sound_id::STANDARD_LEARNT_SPELL);

	common_assets.flash_noise_sound.id = to_sound_id(test_scene_sound_id::FLASH_NOISE);

	load_test_scene_sentience_properties(common);
	// _controlfp(0, _EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID | _EM_DENORMAL);
}

#endif
