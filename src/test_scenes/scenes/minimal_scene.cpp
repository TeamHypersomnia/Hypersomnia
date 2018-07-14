#include "minimal_scene.h"

#include "augs/templates/algorithm_templates.h"

#include "game/enums/party_category.h"

#include "test_scenes/test_scene_flavours.h"
#include "test_scenes/ingredients/ingredients.h"
#include "test_scenes/test_scenes_content.h"

#include "game/transcendental/cosmos.h"
#include "game/organization/all_component_includes.h"
#include "game/organization/all_messages_includes.h"
#include "game/transcendental/logic_step.h"

#include "game/detail/inventory/perform_transfer.h"
#include "view/viewables/image_cache.h"

#include "test_scenes/scenes/test_scene_node.h"
#include "augs/math/cascade_aligner.h"

namespace test_scenes {
	entity_id minimal_scene::populate(const loaded_image_caches_map& caches, const logic_step step) const {
		auto& world = step.get_cosmos();

#if 0
		auto create = [&](auto&&... args) {
			return create_test_scene_entity(world, std::forward<decltype(args)>(args)...);
		};
#endif

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

			const auto new_character = prefabs::create_metropolis_soldier(step, transform, typesafe_sprintf("player%x", i));

			new_characters[i] = new_character;

			if (i == 0) {
				new_character.get<components::sentience>().get<health_meter_instance>().set_value(100);
				new_character.get<components::sentience>().get<health_meter_instance>().set_maximum_value(100);
				new_character.get<components::attitude>().parties = party_category::RESISTANCE_CITIZEN;
				new_character.get<components::attitude>().hostile_parties = party_category::METROPOLIS_CITIZEN;
			}
			else if (i == 1) {
				new_character.get<components::sentience>().get<health_meter_instance>().set_value(100);
				new_character.get<components::sentience>().get<health_meter_instance>().set_maximum_value(100);
				new_character.get<components::attitude>().parties = party_category::METROPOLIS_CITIZEN;
				new_character.get<components::attitude>().hostile_parties = party_category::RESISTANCE_CITIZEN;
			}

			auto& sentience = new_character.get<components::sentience>();


			fill_range(sentience.learned_spells, true);
		}

#if 0
		prefabs::create_sample_rifle(step, vec2(100, -500 + 50),
			prefabs::create_sample_magazine(step, vec2(100, -650),
				prefabs::create_cyan_charge(step, vec2(0, 0))));

		prefabs::create_force_grenade(step, { 100, 100 });
		prefabs::create_force_grenade(step, { 200, 100 });
		prefabs::create_force_grenade(step, { 300, 100});

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

		cosmic::reinfer_all_entities(world);

		// _controlfp(0, _EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID | _EM_DENORMAL);
		return new_characters[0];
	}
}
