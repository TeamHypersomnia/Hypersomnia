#include "minimal_scene.h"

#include "augs/templates/algorithm_templates.h"

#include "game/enums/faction_type.h"

#include "test_scenes/test_scene_flavours.h"
#include "test_scenes/ingredients/ingredients.h"
#include "test_scenes/test_scenes_content.h"

#include "game/cosmos/cosmos.h"
#include "game/organization/all_component_includes.h"
#include "game/organization/all_messages_includes.h"
#include "game/cosmos/logic_step.h"

#include "game/detail/inventory/perform_transfer.h"
#include "view/viewables/image_cache.h"

#include "test_scenes/scenes/test_scene_node.h"
#include "game/modes/bomb_mode.h"
#include "augs/math/cascade_aligner.h"
#include "game/modes/test_scene_mode.h"

namespace test_scenes {
	void minimal_scene::setup(test_scene_mode_vars& vars) {
		vars.spawned_faction = faction_type::METROPOLIS;
		vars.name = "Minimal scene vars";
	}

	void minimal_scene::setup(bomb_mode_vars& vars) {
		vars.name = "Minimal scene bomb vars";
	}

	void minimal_scene::populate(const loaded_image_caches_map& caches, const logic_step step) const {
		auto& world = step.get_cosmos();

		auto create = [&](auto&&... args) {
			return create_test_scene_entity(world, std::forward<decltype(args)>(args)...);
		};

		auto get_size_of = [&caches](const auto id) {
			return vec2i(caches.at(to_image_id(id)).get_original_size());
		};

		const int num_characters = 1;

		std::vector<entity_id> new_characters;
		new_characters.resize(num_characters);

		for (int i = 0; i < num_characters; ++i) {
			transformr transform;

			if (i == 0) {
			}
			else if (i == 1) {
				transform.pos.x += 200;
			}

			const auto metropolis_type = test_controlled_characters::METROPOLIS_SOLDIER;
			const auto new_character = create(metropolis_type, transform);

			new_characters[i] = new_character;

			if (i == 0) {
				new_character.get<components::sentience>().get<health_meter_instance>().set_value(100);
				new_character.get<components::sentience>().get<health_meter_instance>().set_maximum_value(100);
				new_character.get<components::attitude>().official_faction = faction_type::RESISTANCE;
			}
			else if (i == 1) {
				new_character.get<components::sentience>().get<health_meter_instance>().set_value(100);
				new_character.get<components::sentience>().get<health_meter_instance>().set_maximum_value(100);
				new_character.get<components::attitude>().official_faction = faction_type::METROPOLIS;
			}

			auto& sentience = new_character.get<components::sentience>();


			fill_range(sentience.learned_spells, true);
		}

#if 0
		prefabs::create_sample_rifle(step, vec2(100, -500 + 50),
		prefabs::create_sample_magazine(step, vec2(100, -650),
		prefabs::create_cyan_charge(step, vec2(0, 0))));


		const auto force_type = test_hand_explosives::FORCE_GRENADE;
		const auto ped_type = test_hand_explosives::PED_GRENADE;
		const auto interference_type = test_hand_explosives::INTERFERENCE_GRENADE;

		create(force_type, { 100, 100 });
		create(force_type, { 200, 100 });
		create(force_type, { 300, 100});

		/* Test: create cyan charges first, only then magazine, and reinfer. */
		const auto charge = prefabs::create_cyan_charge(step, vec2(0, 0));
		prefabs::create_sample_magazine(step, vec2(100, -650), charge);
#endif

		{
			const vec2 floor_size = get_size_of(test_scene_image_id::FLOOR);
			const auto total_floor_size = floor_size * 10;
			const auto floor_origin = vec2(512, -768);

			auto floor_align = [&](const auto flavour_id) {
				return make_cascade_aligner(
					floor_origin,
					total_floor_size, 
					test_scene_node { world, flavour_id }
				);
			};

			floor_align(test_sprite_decorations::FLOOR).set_size(total_floor_size);
		}

		create(test_hand_explosives::BOMB, vec2(580, 200));
		cosmic::reinfer_all_entities(world);

		// _controlfp(0, _EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID | _EM_DENORMAL);
	}
}
