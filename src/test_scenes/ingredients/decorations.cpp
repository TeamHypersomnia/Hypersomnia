#include "test_scenes/ingredients/ingredients.h"
#include "game/assets/all_logical_assets.h"
#include "game/transcendental/cosmos.h"
#include "game/components/fixtures_component.h"
#include "test_scenes/test_scene_animations.h"

namespace test_flavours {
	void populate_decoration_flavours(const populate_flavours_input in) {
		auto& caches = in.caches;
		auto& flavours = in.flavours;

		{
			auto& meta = get_test_flavour(flavours, test_complex_decorations::ROTATING_FAN);

			invariants::render render_def;
			render_def.layer = render_layer::ON_FLOOR;

			meta.set(render_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::FAN, white);

			{
				invariants::movement_path movement_path_def;
				movement_path_def.continuous_rotation_speed = 720.f;
				meta.set(movement_path_def);
			}

			{
				invariants::ground ground_def;

				footstep_effect_input dirt;
				dirt.sound.id = to_sound_id(test_scene_sound_id::FOOTSTEP_FLOOR);
				dirt.sound.modifier.gain = .6f;
				dirt.sound.modifier.pitch = .9f;
				dirt.particles.id = to_particle_effect_id(test_scene_particle_effect_id::FOOTSTEP_SMOKE);

				ground_def.footstep_effect.emplace(dirt);

				meta.set(ground_def);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_complex_decorations::YELLOW_FISH);

			invariants::render render_def;
			render_def.layer = render_layer::ON_ON_FLOOR;

			meta.set(render_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::YELLOW_FISH_1, white);

			{
				invariants::animation anim_def;
				anim_def.id = to_animation_id(test_scene_plain_animation_id::YELLOW_FISH);
				meta.set(anim_def);

				components::animation anim;
				anim.speed_factor = 0.2f;

				meta.set(anim);
			}

			{
				invariants::movement_path movement_path_def;
				auto& square = movement_path_def.rect_bounded;
				square.is_enabled = true;
				square.value.rect_size = vec2i(368, 403) * 2;
				meta.set(movement_path_def);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_complex_decorations::DARKBLUE_FISH);

			invariants::render render_def;
			render_def.layer = render_layer::ON_ON_FLOOR;

			meta.set(render_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::DARKBLUE_FISH_1, white);

			{
				invariants::animation anim_def;
				anim_def.id = to_animation_id(test_scene_plain_animation_id::DARKBLUE_FISH);
				meta.set(anim_def);

				components::animation anim;
				anim.speed_factor = 0.2f;

				meta.set(anim);
			}

			{
				invariants::movement_path movement_path_def;
				auto& square = movement_path_def.rect_bounded;
				square.is_enabled = true;
				square.value.rect_size = vec2i(368, 403) * 2;
				meta.set(movement_path_def);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_complex_decorations::JELLYFISH);

			invariants::render render_def;
			render_def.layer = render_layer::ON_ON_FLOOR;

			meta.set(render_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::JELLYFISH_1, white);

			{
				invariants::animation anim_def;
				anim_def.id = to_animation_id(test_scene_plain_animation_id::JELLYFISH);
				meta.set(anim_def);

				components::animation anim;
				anim.speed_factor = 0.2f;

				meta.set(anim);
			}

			{
				invariants::movement_path movement_path_def;
				auto& square = movement_path_def.rect_bounded;
				square.is_enabled = true;
				square.value.rect_size = vec2i(368, 403) * 2;
				meta.set(movement_path_def);
			}
		}
	}
}

namespace prefabs {
	entity_handle create_rotating_fan(const logic_step step, const transformr& pos) {
		const auto decor = create_test_scene_entity(step.get_cosmos(), test_complex_decorations::ROTATING_FAN, pos);
		return decor;
	}

	entity_handle create_fish(const logic_step step, const test_complex_decorations t, const transformr& pos, const transformr& origin, const unsigned frame_offset) {
		const auto decor = create_test_scene_entity(step.get_cosmos(), t, pos);
		decor.get<components::animation>().state.frame_num = frame_offset;
		decor.get<components::movement_path>().origin = origin;
		return decor;
	}
}
