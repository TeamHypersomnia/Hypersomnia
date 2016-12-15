#include "one_entity.h"
#include "game/ingredients/ingredients.h"
#include "game/transcendental/cosmos.h"
#include "game/assets/texture_id.h"

#include "game/systems_stateless/input_system.h"
#include "game/systems_stateless/render_system.h"
#include "game/systems_stateless/gui_system.h"
#include "game/components/sentience_component.h"
#include "game/components/attitude_component.h"
#include "game/components/name_component.h"
#include "game/enums/party_category.h"

#include "game/messages/intent_message.h"
#include "game/detail/inventory_utils.h"

#include "rendering_scripts/all.h"

#include "texture_baker/font.h"

#include "augs/misc/machine_entropy.h"
#include "game/transcendental/cosmic_entropy.h"
#include "game/transcendental/viewing_session.h"
#include "game/transcendental/step.h"

namespace scene_managers {
	void one_entity::populate_world_with_entities(logic_step& step) {
		auto& world = step.cosm;

		auto crate = prefabs::create_crate(world, vec2(300, 300), vec2i(100, 100));

		auto camera = world.create_entity("camera");
		camera.add_standard_components();
		world_camera = camera;

		auto bg_size = assets::get_size(assets::texture_id::TEST_BACKGROUND);

		for (int x = -4 * 10; x < 4 * 10; ++x)
			for (int y = -4 * 10; y < 4 * 10; ++y)
			{
				auto background = world.create_entity("bg[-]");
				ingredients::sprite(background, vec2(-1000, 0) + vec2(x, y) * (bg_size + vec2(1500, 550)), assets::texture_id::TEST_BACKGROUND, augs::white, render_layer::GROUND);
				//ingredients::standard_static_body(background);

				auto street = world.create_entity("street[-]");
				ingredients::sprite_scalled(street, vec2(-1000, 0) + vec2(x, y) * (bg_size + vec2(1500, 700)) - vec2(1500, 700),
					vec2(3000, 3000),
					assets::texture_id::TEST_BACKGROUND, augs::gray1, render_layer::UNDER_GROUND);

				background.add_standard_components();
				street.add_standard_components();
			}

		const int num_characters = 2;

		std::vector<entity_handle> new_characters;

		for (int i = 0; i < num_characters; ++i) {
			auto new_character = prefabs::create_character(world, vec2(i * 300 , 0), vec2i(1920, 1080), typesafe_sprintf("player%x", i));

			new_characters.push_back(new_character);

			if (i == 0) {
				new_character.get<components::sentience>().health.value = 800;
				new_character.get<components::sentience>().health.maximum = 800;
			}
		}
		
		auto car = prefabs::create_car(world, components::transform(-300, -600, -90));
		auto car2 = prefabs::create_car(world, components::transform(-800, -600, -90));
		auto car3 = prefabs::create_car(world, components::transform(-1300, -600, -90));
		auto motorcycle = prefabs::create_motorcycle(world, components::transform(0, -600, -90));

		name_entity(new_characters[0], entity_name::PERSON, L"Attacker");

		prefabs::create_sample_suppressor(world, vec2(300, -500));

		bool many_charges = false;

		auto rifle = prefabs::create_sample_rifle(step, vec2(100, -500),
			prefabs::create_sample_magazine(step, vec2(100, -650), many_charges ? "10" : "0.3",
				prefabs::create_cyan_charge(world, vec2(0, 0), many_charges ? 1000 : 30)));

		auto rifle2 = prefabs::create_sample_rifle(step, vec2(100, -500 + 50),
			prefabs::create_sample_magazine(step, vec2(100, -650), true ? "10" : "0.3",
				prefabs::create_cyan_charge(world, vec2(0, 0), true ? 1000 : 30)));

		prefabs::create_sample_rifle(step, vec2(100, -500 + 100));

		prefabs::create_pistol(step, vec2(300, -500 + 50));

		auto pis2 = prefabs::create_pistol(step, vec2(300, 50),
			prefabs::create_sample_magazine(step, vec2(100, -650), "0.4",
				prefabs::create_green_charge(world, vec2(0, 0), 40)));

		auto submachine = prefabs::create_submachine(step, vec2(500, -500 + 50),
			prefabs::create_sample_magazine(step, vec2(100 - 50, -650), many_charges ? "10" : "0.5", prefabs::create_pink_charge(world, vec2(0, 0), many_charges ? 500 : 50)));

		prefabs::create_submachine(step, vec2(0, -1000),
			prefabs::create_sample_magazine(step, vec2(100 - 50, -650), many_charges ? "10" : "0.5", prefabs::create_pink_charge(world, vec2(0, 0), many_charges ? 500 : 50)));

		prefabs::create_submachine(step, vec2(150, -1000 + 150),
			prefabs::create_sample_magazine(step, vec2(100 - 50, -650), many_charges ? "10" : "0.5", prefabs::create_pink_charge(world, vec2(0, 0), many_charges ? 500 : 50)));

		prefabs::create_submachine(step, vec2(300, -1000 + 300),
			prefabs::create_sample_magazine(step, vec2(100 - 50, -650), many_charges ? "10" : "0.5", prefabs::create_pink_charge(world, vec2(0, 0), many_charges ? 500 : 50)));

		prefabs::create_submachine(step, vec2(450, -1000 + 450),
			prefabs::create_sample_magazine(step, vec2(100 - 50, -650), many_charges ? "10" : "0.5", prefabs::create_pink_charge(world, vec2(0, 0), many_charges ? 500 : 50)));


		prefabs::create_sample_magazine(step, vec2(100 - 50, -650));
		prefabs::create_sample_magazine(step, vec2(100 - 100, -650), "0.30");
		//prefabs::create_pink_charge(world, vec2(100, 100));
		//prefabs::create_pink_charge(world, vec2(100, -400));
		//prefabs::create_pink_charge(world, vec2(150, -400));
		//prefabs::create_pink_charge(world, vec2(200, -400));
		prefabs::create_cyan_charge(world, vec2(150, -500));
		prefabs::create_cyan_charge(world, vec2(200, -500));

		prefabs::create_cyan_urban_machete(world, vec2(100, 100));
		auto second_machete = prefabs::create_cyan_urban_machete(world, vec2(0, 0));

		auto backpack = prefabs::create_sample_backpack(world, vec2(200, -650));
		prefabs::create_sample_backpack(world, vec2(200, -750));

		world.significant.meta.settings.visibility.epsilon_ray_distance_variation = 0.001;
		world.significant.meta.settings.visibility.epsilon_threshold_obstacle_hit = 10;
		world.significant.meta.settings.visibility.epsilon_distance_vertex_hit = 1;

		world.significant.meta.settings.pathfinding.draw_memorised_walls = 1;
		world.significant.meta.settings.pathfinding.draw_undiscovered = 1;

		// _controlfp(0, _EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID | _EM_DENORMAL);
		characters = to_id_vector(new_characters);
	}

	void one_entity::configure_view(viewing_session& session) const {
		auto& active_context = session.context;

		active_context.map_key_to_intent(window::event::keys::key::W, intent_type::MOVE_FORWARD);
		active_context.map_key_to_intent(window::event::keys::key::S, intent_type::MOVE_BACKWARD);
		active_context.map_key_to_intent(window::event::keys::key::A, intent_type::MOVE_LEFT);
		active_context.map_key_to_intent(window::event::keys::key::D, intent_type::MOVE_RIGHT);

		active_context.map_event_to_intent(window::event::message::mousemotion, intent_type::MOVE_CROSSHAIR);
		active_context.map_key_to_intent(window::event::keys::key::LMOUSE, intent_type::CROSSHAIR_PRIMARY_ACTION);
		active_context.map_key_to_intent(window::event::keys::key::RMOUSE, intent_type::CROSSHAIR_SECONDARY_ACTION);

		active_context.map_key_to_intent(window::event::keys::key::E, intent_type::USE_BUTTON);
		active_context.map_key_to_intent(window::event::keys::key::LSHIFT, intent_type::WALK);

		active_context.map_key_to_intent(window::event::keys::key::G, intent_type::THROW_PRIMARY_ITEM);
		active_context.map_key_to_intent(window::event::keys::key::H, intent_type::HOLSTER_PRIMARY_ITEM);

		active_context.map_key_to_intent(window::event::keys::key::BACKSPACE, intent_type::SWITCH_LOOK);

		active_context.map_key_to_intent(window::event::keys::key::LCTRL, intent_type::START_PICKING_UP_ITEMS);
		active_context.map_key_to_intent(window::event::keys::key::CAPSLOCK, intent_type::SWITCH_CHARACTER);

		active_context.map_key_to_intent(window::event::keys::key::SPACE, intent_type::SPACE_BUTTON);
		active_context.map_key_to_intent(window::event::keys::key::MOUSE4, intent_type::SWITCH_TO_GUI);
	}

	entity_id one_entity::get_selected_character() const {
		return characters[current_character_index];
	}
}
