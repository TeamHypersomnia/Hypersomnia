/*
	Disable float/int warnings, this is just a content script
*/
#if PLATFORM_WINDOWS
#pragma warning(disable : 4244)
#endif
#include "augs/templates/algorithm_templates.h"
#include "augs/math/cascade_aligner.h"
#include "game/assets/ids/asset_ids.h"
#include "game/assets/all_logical_assets.h"

#include "test_scenes/test_scene_flavours.h"
#include "test_scenes/scenes/testbed.h"
#include "test_scenes/ingredients/ingredients.h"
#include "test_scenes/test_scenes_content.h"

#include "game/transcendental/cosmos.h"
#include "game/organization/all_component_includes.h"
#include "game/organization/all_messages_includes.h"

#include "game/stateless_systems/input_system.h"
#include "game/stateless_systems/particles_existence_system.h"
#include "game/stateless_systems/car_system.h"
#include "game/stateless_systems/driver_system.h"

#include "game/enums/party_category.h"
#include "game/detail/describers.h"

#include "game/messages/intent_message.h"
#include "game/detail/inventory/perform_transfer.h"

#include "view/viewables/image_cache.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmic_delta.h"

class testbed_node {
	template <class F>
	decltype(auto) on_enum(F&& f) const {
		return std::visit(std::forward<F>(f), enum_id);
	}

	cosmos* cosm;

	std::variant<
		test_plain_sprited_bodys,
		test_complex_decorations,
		test_sprite_decorations,
		test_sound_decorations,
		test_particles_decorations
	> enum_id;
public:
	testbed_node() = default;
	testbed_node(const testbed_node&) = default;

	template <class E>
	testbed_node(cosmos& csm, const E e) : 
		cosm(&csm),
		enum_id(e)
	{}

	flip_flags flip;
	real32 rotation = 0.f;

	template <class E>
	auto next(const E e) const {
		auto cloned = *this;
		cloned.enum_id = e;
		return cloned;
	}

	auto get_size() const {
		return on_enum([&](const auto id) {
			const auto& f = ::get_test_flavour(cosm->get_common_significant().flavours, id);

			if constexpr(remove_cref<decltype(f)>::template has<invariants::sprite>()) {
				return f.template get<invariants::sprite>().size;
			}
			else {
				return vec2i(5, 5);
			}
		});
	}

	void create(const vec2 resolved_pos) const {
		on_enum([&](const auto id) {
			create_test_scene_entity(*cosm, id, transformr(resolved_pos, rotation)).do_flip(flip);
		});
	}
};

namespace test_scenes {
	entity_id testbed::populate(const loaded_image_caches_map& caches, const logic_step step) const {
		auto& world = step.get_cosmos();
		
#if TODO
		const auto car = prefabs::create_car(step, transformr( { 1490, 340 }, -180));
		const auto car2 = prefabs::create_car(step, transformr({ 1490, 340 + 400 }, -180));
		const auto car3 = prefabs::create_car(step, transformr({ 1490, 340 + 800 }, -180));

		const auto riding_car = prefabs::create_car(step, transformr({ 850, 1000 }, -90));

		const auto riding_car2 = prefabs::create_car(step, transformr({ -850 + 1000, -8200 }, -90 + 180));
#endif

		const int num_characters = 10;

		for (int i = 0; i < 10; ++i) {
			prefabs::create_force_grenade(step, { 254, 611 + i *100.f });
			prefabs::create_ped_grenade(step, { 204, 611 + i * 100.f });
			prefabs::create_interference_grenade(step, { 154, 611 + i * 100.f });
		}

		for (int i = 0; i < 10; ++i) {
			prefabs::create_force_grenade(step, { 654, -811 + i *100.f });
			prefabs::create_ped_grenade(step, { 604, -811 + i * 100.f });
			prefabs::create_interference_grenade(step, { 554, -811 + i * 100.f });
		}

		std::vector<transformr> character_transforms = {
			{ { 0, 300 }, 0 },
			{ { 254, 211 }, 68 },
			{ { 1102, 213 }, 110 },
			{ { 1102, 413 }, 110 },
			{ { -100, 20000 }, 0 },
			{ { 1200, 15000 }, 0 },
			{ { -300, 20000 }, 0 },
			{ { -300, -2000 }, 0 },
			{ { -400, -2000 }, 0 },
			{ { -500, -2000 }, 0 }
		};

		auto get_size_of = [&caches](const auto id) {
			return vec2i(caches.at(to_image_id(id)).get_original_size());
		};

		std::vector<entity_id> new_characters;
		new_characters.resize(num_characters);

		auto character = [&](const size_t i) {
			return i < new_characters.size() ? world[new_characters.at(i)] : world[entity_id()];
		};

		for (int i = 0; i < num_characters; ++i) {
			auto transform = character_transforms[i];

			const bool is_metropolis = i % 2 == 0;
			const auto new_character = (is_metropolis ? prefabs::create_metropolis_soldier : prefabs::create_resistance_soldier)(step, transform, typesafe_sprintf("player%x", i));

			new_characters[i] = new_character;

			auto& sentience = new_character.get<components::sentience>();

			sentience.get<consciousness_meter_instance>().set_maximum_value(400);
			sentience.get<consciousness_meter_instance>().set_value(400);

			sentience.get<personal_electricity_meter_instance>().set_maximum_value(400);
			sentience.get<personal_electricity_meter_instance>().set_value(400);

			new_character.get<components::attitude>().maximum_divergence_angle_before_shooting = 25;

			if (i == 0 || i == 1) {
				/* Let's have two OP characters */
				sentience.get<health_meter_instance>().set_value(10000);
				sentience.get<health_meter_instance>().set_maximum_value(10000);

				sentience.get<personal_electricity_meter_instance>().set_value(10000);
				sentience.get<personal_electricity_meter_instance>().set_maximum_value(10000);

				const auto rifle = (is_metropolis ? prefabs::create_sample_rifle : prefabs::create_vindicator)(
					step, vec2(100, -500), prefabs::create_sample_magazine(step, vec2(100, -650), prefabs::create_steel_charge(step, vec2(0, 0)), 30)
				);

				if (i == 1) {
					new_character.get<components::crosshair>().base_offset.x = -500;
				}

				perform_transfer(item_slot_transfer_request::standard(rifle, new_character.get_primary_hand()), step);
			}

			if (i == 7 || i == 8 || i == 9) {
				/* Give some stuff to three test characters */
				if (i == 9) {
					const auto rifle = prefabs::create_sample_rifle(step, vec2(100, -500),
						prefabs::create_sample_magazine(step, vec2(100, -650),
							prefabs::create_cyan_charge(step, vec2(0, 0))));

					perform_transfer(item_slot_transfer_request::standard(rifle, new_character.get_primary_hand()), step);
				}
				else {
					const auto rifle = (i == 7 ? prefabs::create_sample_rifle : prefabs::create_sample_rifle)(step, vec2(100, -500),
						prefabs::create_sample_magazine(step, vec2(100, -650),
							prefabs::create_cyan_charge(step, vec2(0, 0))));

					perform_transfer(item_slot_transfer_request::standard(rifle, new_character.get_primary_hand()), step);
				}

				const auto backpack = (is_metropolis ? prefabs::create_sample_backpack : prefabs::create_brown_backpack)(step, vec2(200, -650));
				perform_transfer(item_slot_transfer_request::standard(backpack, new_character[slot_function::SHOULDER]), step);
			}

			fill_range(sentience.learned_spells, true);
		}

		{
			const vec2 coords[] = {
				{ 1200, 5400 },
				{ 1200, 10400 },
				{ 1200, 15400 },
				{ 1200, 20400 },
				{ -1, 25400 },
				{ 1200, 30400 },
			};

			for (const auto c : coords) {
				prefabs::create_crate(step, c + vec2(-100, 400) );
				prefabs::create_crate(step, c + vec2(300, 300) );
				prefabs::create_crate(step, c + vec2(100, -200) );

				const auto light_pos = c + vec2(0, 100);
				const auto light_cyan = c.x < 0 ? orange : rgba(30, 255, 255, 255);

				{
					const auto l = create_test_scene_entity(world, test_static_lights::STRONG_LAMP);
					l.set_logic_transform(transformr(light_pos));

					auto& light = l.get<components::light>();

					light.color = light_cyan;
				}

				{
					create_test_scene_entity(world, test_wandering_pixels_decorations::WANDERING_PIXELS, [&](const auto e){
						auto& w = e.template get<components::wandering_pixels>();

						w.colorize = light_cyan;
						w.particles_count = 150;
						const auto reach = xywh(light_pos.x- 350, light_pos.y-350, 500, 500);
						w.size = reach.get_size();
						e.set_logic_transform(reach.get_center());
					});
				}
			}
		}

		{
			{
				const auto l = create_test_scene_entity(world, test_static_lights::STRONG_LAMP);
				l.set_logic_transform(transformr(vec2(-44, 270)));
				auto& light = l.get<components::light>();
				light.color = cyan;
			}
			{
				const auto l = create_test_scene_entity(world, test_static_lights::STRONG_LAMP);
				l.set_logic_transform(transformr(vec2(1098, 220)));
				auto& light = l.get<components::light>();
				light.color = orange;
			}
			{
				const auto l = create_test_scene_entity(world, test_static_lights::STRONG_LAMP);
				l.set_logic_transform(transformr(vec2(223, -47)));
				auto& light = l.get<components::light>();
				light.color = cyan;
			}

			{
				const auto left_reach = xywh(164.f - 8.f + 90.f - 550, 220 - 250, 1000, 600);
				const auto right_reach = xywh(1164.f - 8.f + 90.f - 600, 220 - 250, 1000, 600);

				{
					create_test_scene_entity(world, test_wandering_pixels_decorations::WANDERING_PIXELS, [&](const auto e){

						auto& w = e.template get<components::wandering_pixels>();

						w.colorize = cyan;
						w.particles_count = 50;
						w.size = left_reach.get_size();
						e.set_logic_transform(left_reach.get_center());
					});
				}

				{
					create_test_scene_entity(world, test_wandering_pixels_decorations::WANDERING_PIXELS, [&](const auto e){

						auto& w = e.template get<components::wandering_pixels>();

						w.colorize = orange;
						w.particles_count = 50;
						w.size = right_reach.get_size();
						e.set_logic_transform(right_reach.get_center());
					});
				}
			}


			{
				create_test_scene_entity(world, test_sprite_decorations::HAVE_A_PLEASANT, transformr(vec2(-42, -32)));
				create_test_scene_entity(world, test_sprite_decorations::AWAKENING, transformr(vec2(-42, 8)));
				create_test_scene_entity(world, test_sprite_decorations::METROPOLIS, transformr(vec2(1106, 3)));

				auto half = vec2(-64, -64);
				prefabs::create_brick_wall(step, half + vec2(0, 0));
				prefabs::create_brick_wall(step, half +vec2(128, 128));
				prefabs::create_brick_wall(step, half +vec2(128, 0));
				prefabs::create_brick_wall(step, half +vec2(0, 128));

				const auto horioff = 8 * 128;

				prefabs::create_brick_wall(step, half +vec2(horioff, 0) + vec2(0, 0));
				prefabs::create_brick_wall(step, half +vec2(horioff, 0) + vec2(128, 128));
				prefabs::create_brick_wall(step, half +vec2(horioff, 0) + vec2(128, 0));
				prefabs::create_brick_wall(step, half +vec2(horioff, 0) + vec2(0, 128));

				for (int b = 0; b < 10; ++b) {
					prefabs::create_brick_wall(step, transformr(half +vec2(-128, -128 - b*128) + vec2(0, 256), 90));
					prefabs::create_brick_wall(step, transformr(half +vec2(horioff + 256, -128 - b*128) + vec2(0, 256), 90));
				}

				{
					const vec2 bg_size = get_size_of(test_scene_image_id::SOIL);

					const auto num_roads = 10 * 10;
					const auto side = static_cast<int>(sqrt(num_roads) / 2);

					for (int x = -side; x < side; ++x) {
						for (int y = -side; y < side * 16; ++y) {
							create_test_scene_entity(world, test_sprite_decorations::SOIL, transformr{ bg_size * vec2i(x, y) });
						}
					}
				}

				create_test_scene_entity(world, test_sprite_decorations::ROAD_DIRT, transformr(vec2(468, 112)));

				for (int r = 0; r < 38; ++r) {
					const vec2 size = get_size_of(test_scene_image_id::ROAD);

					create_test_scene_entity(world, test_sprite_decorations::ROAD, transformr{ vec2(468, 832+ size.y * r ) });
				}

				{
					const vec2 size = get_size_of(test_scene_image_id::FLOOR);

					for (int x = 0; x < 10; ++x) {
						for (int y = 0; y < 10; ++y) {
							create_test_scene_entity(world, test_sprite_decorations::FLOOR, transformr(vec2(-64, -192) + size * vec2i(x, -y)));
						}
					}
				}

			}
		}

		prefabs::create_kek9(step, vec2(-800, -200),
			prefabs::create_sample_magazine(step, vec2(100, -650),
				prefabs::create_cyan_charge(step, vec2(0, 0))));

		prefabs::create_amplifier_arm(step, vec2(-300, -500 + 50));
		prefabs::create_cyan_urban_machete(step, vec2(100, 100));

		prefabs::create_sample_backpack(step, vec2(200, -750));
		prefabs::create_brown_backpack(step, vec2(280, -750));

		prefabs::create_rotating_fan(step, vec2(380, -750));

		prefabs::create_rifle(step, vec2(280, -750), test_shootable_weapons::DATUM_GUN, prefabs::create_sample_magazine(step, vec2(100, -650), prefabs::create_cyan_charge(step, vec2(0, 0)), 25));

		prefabs::create_rifle(step, vec2(300, -100), test_shootable_weapons::LEWSII, prefabs::create_magazine(step, vec2(100, -650), test_container_items::LEWSII_MAG, prefabs::create_steel_charge(step, vec2(0, 0)), 100));
		prefabs::create_rifle(step, vec2(400, -100), test_shootable_weapons::LEWSII, prefabs::create_magazine(step, vec2(100, -650), test_container_items::LEWSII_MAG, prefabs::create_steel_charge(step, vec2(0, 0)), 100));

		const auto aquarium_size = get_size_of(test_scene_image_id::AQUARIUM_SAND_1);
		const auto whole_aquarium_size = aquarium_size * 2;

		auto create_aquarium = [&](const transformr aquarium_tr) {
			const auto aquarium_origin = aquarium_tr + transformr(aquarium_size / 2);

			auto aquarium_align = [&](const auto flavour_id) {
				return make_cascade_aligner(
					aquarium_origin.pos, 
					whole_aquarium_size, 
					testbed_node { world, flavour_id }
				);
			};

			{
				{
					const auto bub = test_particles_decorations::FLOWER_BUBBLES;

					aquarium_align(test_complex_decorations::FLOWER_PINK)
						.li().ti().nr().nr().nd().prepend(bub).dup()
						.nd().nd().prepend(bub).dup()
						.nl().nd().prepend(bub)
						.again(test_complex_decorations::FLOWER_CYAN)
						.ri().ti().nl().nd().nd().nd().prepend(bub).dup()
						.nl().nd().prepend(bub).dup()
						.nd().prepend(bub)
					;
				}

				aquarium_align(test_sound_decorations::AQUARIUM_AMBIENCE_LEFT)
					.lo().bo()
					.again(test_sound_decorations::AQUARIUM_AMBIENCE_RIGHT)
					.ro().bo()
					.again(test_sprite_decorations::LAB_WALL_A2).flip_v()
					.to()
				;

				aquarium_align(test_particles_decorations::AQUARIUM_BUBBLES).ti().rot_90().mv(0, 15);

				aquarium_align(test_plain_sprited_bodys::AQUARIUM_GLASS)
					.li().bo().nr()
					.next(test_plain_sprited_bodys::AQUARIUM_GLASS_START).lo().create_pop()
					.fill_ri(-1)
					.next(test_plain_sprited_bodys::AQUARIUM_GLASS_START).ro().flip_h()
				;

				aquarium_align(test_plain_sprited_bodys::LAB_WALL_SMOOTH_END)
					.li().bo().again()
					.ri().bo().flip_h()
				;

				aquarium_align(test_plain_sprited_bodys::LAB_WALL_CORNER_SQUARE)
					.lo().bo().again()
					.ro().bo().flip_h().again()

					.to().lo().flip_v().again()
					.to().ro().flip_v().flip_h()
				;

				aquarium_align(test_plain_sprited_bodys::LAB_WALL)
					.rot_90().lo().bi().fill_ti().again()
					.flip_v().rot_90().ro().bi().fill_ti().again()
					.flip_v().to().li().fill_ri()
				;
			}

			{
				vec2 lights[3] = {
					{ -145, 417 },
					vec2(193, 161) - vec2(160, 206),
					{ 463, -145 }
				};

				rgba light_cols[3] = { 
					rgba(0, 132, 190, 255),
					rgba(0, 99, 126, 255),
					rgba(0, 180, 59, 255)
				};

				for (int i = 0; i < 3; ++i) {
					const auto l = create_test_scene_entity(world, test_static_lights::AQUARIUM_LAMP);
					l.set_logic_transform(aquarium_tr + transformr(lights[i]));
					auto& light = l.get<components::light>();
					light.color = light_cols[i];
				}
			}

			{
				constexpr int N = 9;
				constexpr int DN = 4;

				transformr caustics[N] = {
					{ { 16, 496 }, 0 },
					{ { 16, 369 }, -75 },
					{ { -26, 250 }, 45 },

					{ { 2, 98 }, 75 },
					{ { 96, 74 }, 0 },
					{ { 151, -88 }, 120 },

					{ { 296, -104 }, 150 },
					{ { 357, -26 }, 45 },
					{ { 513, -1 }, 195 }
				};

				int caustics_offsets [N] =  {
					28, 2, 11,
					28, 2, 11,
					28, 2, 11
				};

				transformr dim_caustics[DN] = {
					{ { 225, 446 }, -75 },
					{ { 241, 238 }, -15 },
					{ { 449, 414 }, -75 },
					{ { 465, 238 }, -135 }
				};

				int dim_caustics_offsets [DN] = {
					31, 20, 10, 13
				};

				for (int i = 0; i < N; ++i) {
					const auto target = caustics[i] + aquarium_tr;
					auto ent = create_test_scene_entity(world, test_complex_decorations::WATER_SURFACE, target);
					ent.get<components::animation>().state.frame_num = caustics_offsets[i];
				}

				for (int i = 0; i < DN; ++i) {
					const auto target = dim_caustics[i] + aquarium_tr;
					auto ent = create_test_scene_entity(world, test_complex_decorations::WATER_SURFACE, target);
					ent.get<components::animation>().state.frame_num = dim_caustics_offsets[i];
					ent.get<components::sprite>().colorize.a = 79;
				}
			}

			{
				constexpr int N = 2;

				transformr halogens[N] = {
					{ { -174, 417 }, 90 },
					{ { 463, -174 }, 180 }
				};

				rgba halogens_light_cols[N] = {
					rgba(96, 255, 255, 255),
					rgba(103, 255, 69, 255)
				};

				rgba halogens_bodies_cols[N] = {
					rgba(0, 122, 255, 255),
					rgba(0, 180, 59, 255)
				};

				for (int i = 0; i < N; ++i) {
					const auto target = halogens[i] + aquarium_tr;
					
					{
						auto ent = create_test_scene_entity(world, test_sprite_decorations::AQUARIUM_HALOGEN_1_BODY, target);
						ent.get<components::sprite>().colorize = halogens_bodies_cols[i];
					}

					{
						auto ent = create_test_scene_entity(world, test_sprite_decorations::AQUARIUM_HALOGEN_1_LIGHT, target);
						ent.get<components::sprite>().colorize = halogens_light_cols[i];
					}
				}
			}

			{
				const auto bottom_lamp_tr = transformr(vec2(193, 161) - vec2(160, 206), -45.f);

				const auto target = bottom_lamp_tr + aquarium_tr;

				{
					auto ent = create_test_scene_entity(world, test_sprite_decorations::AQUARIUM_BOTTOM_LAMP_BODY, target);
					ent.get<components::sprite>().colorize = rgba(0, 122, 255, 255);
				}

				{
					auto ent = create_test_scene_entity(world, test_sprite_decorations::AQUARIUM_BOTTOM_LAMP_LIGHT, target);
					ent.get<components::sprite>().colorize = rgba(96, 255, 255, 255);
				}
			}


			create_test_scene_entity(world, test_wandering_pixels_decorations::WANDERING_PIXELS, [&](const auto e){
				auto& w = e.template get<components::wandering_pixels>();

				w.colorize = cyan;
				w.particles_count = 40;
				w.size = { 750, 750 };
				w.keep_particles_within_bounds = true;
				e.set_logic_transform(aquarium_origin);
			});

			create_test_scene_entity(world, test_wandering_pixels_decorations::AQUARIUM_PIXELS_LIGHT, [&](const auto e){
				e.set_logic_transform(aquarium_origin);
			});

			create_test_scene_entity(world, test_wandering_pixels_decorations::AQUARIUM_PIXELS_DIM, [&](const auto e){
				e.set_logic_transform(aquarium_origin);
			});

			{
				const auto edge_size = get_size_of(test_scene_image_id::AQUARIUM_SAND_EDGE);

				const auto s = edge_size;
				const auto h = edge_size / 2;

				for (int g = 0; g < (aquarium_size * 2).x / s.x; ++g) {
					create_test_scene_entity(world, test_sprite_decorations::AQUARIUM_SAND_EDGE, aquarium_origin.pos + aquarium_size + vec2(-h.x, -h.y) - vec2(s.x * g, 0));
				}
			}

			create_test_scene_entity(world, test_sprite_decorations::WATER_COLOR_OVERLAY, aquarium_origin);

			create_test_scene_entity(world, test_sprite_decorations::AQUARIUM_SAND_1, aquarium_tr);
			create_test_scene_entity(world, test_sprite_decorations::AQUARIUM_SAND_1, aquarium_tr + transformr(vec2(aquarium_size.x, 0)));
			create_test_scene_entity(world, test_sprite_decorations::AQUARIUM_SAND_2, aquarium_tr + transformr(vec2(aquarium_size.x, aquarium_size.y)));
			create_test_scene_entity(world, test_sprite_decorations::AQUARIUM_SAND_2, aquarium_tr + transformr(vec2(0, aquarium_size.y)));

			create_test_scene_entity(world, test_sprite_decorations::DUNE_SMALL, transformr(aquarium_origin.pos + vec2(-193, -193) + vec2(52, -22)));
			create_test_scene_entity(world, test_sprite_decorations::DUNE_SMALL, transformr(aquarium_origin.pos + vec2(-237, 255)));
			create_test_scene_entity(world, test_sprite_decorations::DUNE_BIG, transformr(aquarium_origin.pos + vec2(-74, -48)));
			create_test_scene_entity(world, test_sprite_decorations::DUNE_BIG, transformr(aquarium_origin.pos + vec2(161, 126)));

			const auto yellowfish = test_complex_decorations::YELLOW_FISH;
			const auto darkbluefish = test_complex_decorations::DARKBLUE_FISH;
			const auto cyanvioletfish = test_complex_decorations::CYANVIOLET_FISH;
			const auto jellyfish = test_complex_decorations::JELLYFISH;
			const auto dragon_fish = test_complex_decorations::DRAGON_FISH;
			const auto rainbow_dragon_fish = test_complex_decorations::RAINBOW_DRAGON_FISH;

			prefabs::create_fish(step, yellowfish, aquarium_tr - vec2(80, 10), aquarium_origin);
			prefabs::create_fish(step, yellowfish, aquarium_tr + components::transform(vec2(80, 10), -180), aquarium_origin);
			prefabs::create_fish(step, yellowfish, aquarium_tr - vec2(80, 30), aquarium_origin);
			prefabs::create_fish(step, yellowfish, aquarium_tr + components::transform(vec2(80, 50), -180), aquarium_origin);
			prefabs::create_fish(step, yellowfish, aquarium_tr - vec2(120, 30), aquarium_origin);
			prefabs::create_fish(step, yellowfish, aquarium_tr + components::transform(vec2(90, 40), -180), aquarium_origin);

			prefabs::create_fish(step, cyanvioletfish, aquarium_tr - vec2(40, 10), aquarium_origin);
			prefabs::create_fish(step, cyanvioletfish, aquarium_tr + components::transform(vec2(40, 10), -180), aquarium_origin);
			prefabs::create_fish(step, cyanvioletfish, aquarium_tr - vec2(40, 30), aquarium_origin);
			prefabs::create_fish(step, cyanvioletfish, aquarium_tr + components::transform(vec2(40, 50), -180), aquarium_origin);
			prefabs::create_fish(step, cyanvioletfish, aquarium_tr - vec2(70, 30), aquarium_origin);
			prefabs::create_fish(step, cyanvioletfish, aquarium_tr + components::transform(vec2(40, 40), -180), aquarium_origin);

			prefabs::create_fish(step, yellowfish, aquarium_tr + vec2(20, 20) - vec2(80, 10), aquarium_origin);
			prefabs::create_fish(step, yellowfish, aquarium_tr + vec2(20, 20) + components::transform(vec2(80, 10), -180), aquarium_origin);
			prefabs::create_fish(step, yellowfish, aquarium_tr + vec2(20, 20) - vec2(80, 30), aquarium_origin);
			prefabs::create_fish(step, yellowfish, aquarium_tr + vec2(20, 20) + components::transform(vec2(80, 50), -180), aquarium_origin);
			prefabs::create_fish(step, yellowfish, aquarium_tr + vec2(20, 20) - vec2(120, 30), aquarium_origin);
			prefabs::create_fish(step, yellowfish, aquarium_tr + vec2(20, 20) + components::transform(vec2(90, 40), -180), aquarium_origin);

			const auto jellyfishtr = aquarium_tr + components::transform(vec2(100, 100), -45);
			prefabs::create_fish(step, darkbluefish, jellyfishtr - vec2(80, 10), aquarium_origin);
			prefabs::create_fish(step, darkbluefish, jellyfishtr + components::transform(vec2(80, 10), -180), aquarium_origin);

			prefabs::create_fish(step, darkbluefish, jellyfishtr - vec2(80, 30), aquarium_origin);
			prefabs::create_fish(step, darkbluefish, jellyfishtr + components::transform(vec2(80, 50), -180), aquarium_origin);

			prefabs::create_fish(step, darkbluefish, jellyfishtr - vec2(120, 30), aquarium_origin);
			prefabs::create_fish(step, darkbluefish, jellyfishtr + components::transform(vec2(90, 40), -180), aquarium_origin);

			prefabs::create_fish(step, jellyfish, aquarium_tr - vec2(180, 30), aquarium_origin);
			prefabs::create_fish(step, jellyfish, aquarium_tr + components::transform(vec2(190, 40), -180), aquarium_origin);
			prefabs::create_fish(step, jellyfish, aquarium_tr - vec2(180, 50), aquarium_origin);
			prefabs::create_fish(step, jellyfish, aquarium_tr + components::transform(vec2(190, 60), -180), aquarium_origin);
			prefabs::create_fish(step, jellyfish, aquarium_tr - vec2(180, 70), aquarium_origin);
			prefabs::create_fish(step, jellyfish, aquarium_tr + components::transform(vec2(190, 80), -180), aquarium_origin);

			prefabs::create_fish(step, dragon_fish, aquarium_tr - vec2(280, 130), aquarium_origin);
			prefabs::create_fish(step, dragon_fish, aquarium_tr + components::transform(vec2(290, 40), -180), aquarium_origin);
			prefabs::create_fish(step, dragon_fish, aquarium_tr - vec2(280, 150), aquarium_origin);
			prefabs::create_fish(step, dragon_fish, aquarium_tr + components::transform(vec2(290, 60), -180), aquarium_origin);

			prefabs::create_fish(step, rainbow_dragon_fish, vec2(40, 40) + aquarium_tr.pos - vec2(280, 130), aquarium_origin);
			prefabs::create_fish(step, rainbow_dragon_fish, vec2(40, 40) + aquarium_tr.pos + vec2(290, 40), aquarium_origin);
			prefabs::create_fish(step, rainbow_dragon_fish, vec2(40, 40) + aquarium_tr.pos - vec2(280, 150), aquarium_origin);
			prefabs::create_fish(step, rainbow_dragon_fish, vec2(40, 40) + aquarium_tr.pos + vec2(290, 60), aquarium_origin);
		};

		const auto orig1 = vec2(-1024, 1024);
		create_aquarium(orig1);

		const auto lab_wall_size = get_size_of(test_scene_image_id::LAB_WALL);

		make_cascade_aligner(
			orig1 + aquarium_size / 2, 
			whole_aquarium_size + vec2i::square(2 * lab_wall_size.y),
			testbed_node { world, test_complex_decorations::CONSOLE_LIGHT }
		).ro()
		.next(test_sound_decorations::HUMMING_DISABLED);

		if (character(2).alive()) {
			const auto second_machete = prefabs::create_cyan_urban_machete(step, vec2(0, 300));
			perform_transfer(item_slot_transfer_request::standard(second_machete, character(2).get_primary_hand()), step);
		}

		return character(1);
	}
}
