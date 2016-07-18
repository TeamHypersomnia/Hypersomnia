#include "one_entity.h"
#include "game/ingredients/ingredients.h"
#include "game/cosmos.h"
#include "game/assets/texture_id.h"

#include "game/systems/input_system.h"
#include "game/systems/render_system.h"
#include "game/stateful_systems/gui_system.h"
#include "game/components/camera_component.h"
#include "game/components/sentience_component.h"
#include "game/components/attitude_component.h"
#include "game/components/name_component.h"
#include "game/enums/party_category.h"

#include "game/messages/intent_message.h"
#include "game/detail/inventory_utils.h"

#include "rendering_scripts/all.h"

#include "texture_baker/font.h"

#include "misc/machine_entropy.h"
#include "game/cosmic_entropy.h"
#include "game/step.h"

namespace scene_managers {
	void one_entity::populate_world_with_entities(fixed_step& step) {
		auto& world = step.cosm;
		vec2i size = step.cosm.settings.screen_size;

		auto crate = prefabs::create_crate(world, vec2(300, 300), vec2i(100, 100));

		auto camera = world.create_entity("camera");
		ingredients::camera(camera, size.x, size.y);
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

		const int num_characters = 1;

		std::vector<entity_handle> new_characters;

		for (int i = 0; i < num_characters; ++i) {
			auto new_character = prefabs::create_character(world, vec2(i * 300 + 200, 200));
			new_character.set_debug_name(typesafe_sprintf("player%x", i));

			new_characters.push_back(new_character);

			if (i == 0) {
				new_character.get<components::sentience>().health.value = 800;
				new_character.get<components::sentience>().health.maximum = 800;
			}
		}

		name_entity(world[new_characters[0]], entity_name::PERSON, L"Attacker");

		ingredients::inject_window_input_to_character(world[new_characters[current_character]], camera);

		auto& active_context = world.settings.input;

		active_context.map_key_to_intent(window::event::keys::W, intent_type::MOVE_FORWARD);
		active_context.map_key_to_intent(window::event::keys::S, intent_type::MOVE_BACKWARD);
		active_context.map_key_to_intent(window::event::keys::A, intent_type::MOVE_LEFT);
		active_context.map_key_to_intent(window::event::keys::D, intent_type::MOVE_RIGHT);

		active_context.map_event_to_intent(window::event::mousemotion, intent_type::MOVE_CROSSHAIR);
		active_context.map_key_to_intent(window::event::keys::LMOUSE, intent_type::CROSSHAIR_PRIMARY_ACTION);
		active_context.map_key_to_intent(window::event::keys::RMOUSE, intent_type::CROSSHAIR_SECONDARY_ACTION);

		active_context.map_key_to_intent(window::event::keys::E, intent_type::USE_BUTTON);
		active_context.map_key_to_intent(window::event::keys::LSHIFT, intent_type::WALK);

		active_context.map_key_to_intent(window::event::keys::G, intent_type::THROW_PRIMARY_ITEM);
		active_context.map_key_to_intent(window::event::keys::H, intent_type::HOLSTER_PRIMARY_ITEM);

		active_context.map_key_to_intent(window::event::keys::BACKSPACE, intent_type::SWITCH_LOOK);

		active_context.map_key_to_intent(window::event::keys::LCTRL, intent_type::START_PICKING_UP_ITEMS);
		active_context.map_key_to_intent(window::event::keys::CAPSLOCK, intent_type::SWITCH_CHARACTER);

		active_context.map_key_to_intent(window::event::keys::SPACE, intent_type::SPACE_BUTTON);
		active_context.map_key_to_intent(window::event::keys::MOUSE4, intent_type::SWITCH_TO_GUI);

		world.settings.visibility.epsilon_ray_distance_variation = 0.001;
		world.settings.visibility.epsilon_threshold_obstacle_hit = 10;
		world.settings.visibility.epsilon_distance_vertex_hit = 1;

		world.settings.pathfinding.draw_memorised_walls = 1;
		world.settings.pathfinding.draw_undiscovered = 1;

		characters = to_id_vector(new_characters);
	}


	entity_id one_entity::get_controlled_entity() const {
		return characters[current_character];
	}

	cosmic_entropy one_entity::make_cosmic_entropy(augs::machine_entropy machine, cosmos& cosm) {
		cosmic_entropy result;
		result.from_input_receivers_distribution(machine, cosm);

		return result;
	}

	void one_entity::pre_solve(fixed_step& step) {

	}

	void one_entity::post_solve(fixed_step& step) {
		auto& cosmos = step.cosm;

		for (auto& it : step.messages.get_queue<messages::intent_message>()) {
			if (it.subject == characters[current_character] && it.intent == intent_type::SWITCH_CHARACTER && it.pressed_flag) {
				++current_character;
				current_character %= characters.size();

				ingredients::inject_window_input_to_character(cosmos[characters[current_character]], cosmos[world_camera]);
			}
		}
	}


	void one_entity::view_cosmos(basic_viewing_step& step) const {
		auto& cosmos = step.cosm;

		viewing_step viewing(step, cosmos[world_camera].get<components::camera>().how_camera_will_render);
		rendering_scripts::standard_rendering(viewing);
	}
}