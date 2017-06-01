#include "generated/setting_build_unit_tests.h"

#if BUILD_UNIT_TESTS
#include <tuple>
#include <catch.hpp>
#include "game/transcendental/cosmos.h"
#include "game/transcendental/cosmic_delta.h"
#include "augs/templates/introspection_utils/describe_fields.h"
#include "game/transcendental/types_specification/all_component_includes.h"
#include "augs/misc/delta_compression.h"
#include "augs/misc/templated_readwrite.h"
#include "generated/introspectors.h"

TEST_CASE("CosmicDelta PaddingSanityCheck1") {
	struct ok {
		bool a;
		int b;
		bool c;

		ok() : a(false), b(1), c(false) {

		}
	};

	typedef ok checked_type;
	constexpr size_t type_size = sizeof(checked_type);

	char buf1[type_size];
	char buf2[type_size];

	for (int i = 0; i < type_size; ++i) {
		buf1[i] = 3;
		buf2[i] = 4;
	}

	new (buf1) checked_type;
	new (buf2) checked_type;

	const bool are_different = std::memcmp(buf1, buf2, type_size);

	REQUIRE(are_different);
}

TEST_CASE("CosmicDelta PaddingSanityCheck2") {
	struct ok {
		bool a = false;
		int b = 1;
		bool c = false;
	};

	typedef ok checked_type;
	constexpr size_t type_size = sizeof(checked_type);

	char buf1[type_size];
	char buf2[type_size];

	for (int i = 0; i < type_size; ++i) {
		buf1[i] = 3;
		buf2[i] = 4;
	}

	new (buf1) checked_type;
	new (buf2) checked_type;

	const bool are_different = std::memcmp(buf1, buf2, type_size);

	REQUIRE(are_different);
}

#include "augs/filesystem/file.h"

TEST_CASE("CosmicDelta PaddingTest") {
	std::string component_size_information;
	std::size_t total_components_size = 0u;

	auto padding_checker = [&](auto c) {
		typedef decltype(c) checked_type;
		static_assert(std::is_same_v<std::decay_t<checked_type>, checked_type>, "Something's wrong with the types");
		static_assert(
			augs::is_byte_io_safe_v<augs::stream, checked_type> || allows_nontriviality_v<checked_type>,
			"Non-trivially copyable component detected! If you need a non-trivial component, explicitly define static constexpr bool allow_nontriviality = true; within the class"
		);

		total_components_size += sizeof checked_type;
		component_size_information += typesafe_sprintf("%x == sizeof %x\n", sizeof checked_type, typeid(checked_type).name());

		augs::constexpr_if<!allows_nontriviality_v<checked_type>>()([](auto...){
			constexpr size_t type_size = sizeof(checked_type);

			char buf1[type_size];
			char buf2[type_size];

			for (int i = 0; i < type_size; ++i) {
				buf1[i] = 3;
				buf2[i] = 4;
			}

			// it looks like the placement new may zero-out the memory before allocation.
			// we will leave this test as it is useful anyway.

			new (buf1) checked_type();
			new (buf2) checked_type();

			int iter = 0;
			bool same = true;

			for (; iter < type_size; ++iter) {
				if (buf1[iter] != buf2[iter]) {
					same = false;
					break;
				}
			}

			if(!same) {
				LOG("Object 1:\n%x\n Object 2:\n%x\n", describe_fields(*(checked_type*)buf1), describe_fields(*(checked_type*)buf2));

				FAIL(typesafe_sprintf(
					"Padding is wrong in %x\nsizeof: %x\nDivergence position: %x", 
					typeid(checked_type).name(),
					type_size,
					iter
				));
			}

			// test by delta
			{
				checked_type a;
				checked_type b;

				const auto dt = augs::object_delta<checked_type>(a, b);

				if (dt.has_changed()) {
					LOG("Object 1:\n%x\n Object 2:\n%x\n", describe_fields(a), describe_fields(b));

					FAIL(typesafe_sprintf(
						"Padding is wrong in %x\nsizeof: %x\nDivergence position: %x", 
						typeid(checked_type).name(),
						type_size,
						static_cast<int>(dt.get_first_divergence_pos())
					));
				}
			}

			// prove by introspection that all members are directly next to each other in memory
			const auto breaks = determine_breaks_in_fields_continuity_by_introspection(checked_type());

			if (breaks.size() > 0) {
				LOG(breaks);
				LOG(describe_fields(checked_type()));

				FAIL(typesafe_sprintf(
					"Padding is wrong in %x\nsizeof: %x\n", 
					typeid(checked_type).name(),
					type_size
				));
			}
		});
	};

	for_each_through_std_get(put_all_components_into_t<std::tuple>(), padding_checker);

	component_size_information += typesafe_sprintf("Total size in bytes: %x", total_components_size);
	augs::create_text_file("generated/logs/components.txt", component_size_information);
}

TEST_CASE("Cosmos", "GuidizeTests") {
	cosmos c1(2);

	const auto new_ent1 = c1.create_entity("e1");

	item_slot_transfer_request dt;
	dt.item = new_ent1;

	const auto guidized = c1.guidize(dt);

	REQUIRE(0 == guidized.target_slot.container_entity);
	REQUIRE(1 == guidized.item);

	const auto deguidized = c1.deguidize(guidized);
	REQUIRE(dt.item == deguidized.item);
	REQUIRE(dt.target_slot.container_entity == deguidized.target_slot.container_entity);
	entity_id dead;

	REQUIRE(dead == deguidized.target_slot.container_entity);
	// sanity check
	REQUIRE(dead == dt.target_slot.container_entity);
}

TEST_CASE("CosmicDelta EmptyAndTwoNew") {
	cosmos c1(2);
	cosmos c2(2);

	const auto new_ent1 = c2.create_entity("e1");
	const auto new_ent2 = c2.create_entity("e2");

	const auto first_guid = new_ent1.get_guid();
	const auto second_guid = new_ent2.get_guid();

	components::transform first_transform(21, 0, 12.4f);

	new_ent1 += first_transform;
	new_ent1 += components::rigid_body();
	new_ent1 += components::render();
	new_ent1 += components::sprite();

	new_ent2 += components::transform();
	new_ent2 += components::trace();
	new_ent2 += components::position_copying();

	{
		augs::stream s;

		cosmic_delta::encode(c1, c2, s);
		cosmic_delta::decode(c1, s);
	}

	const auto ent1 = c1.get_handle(first_guid);
	const auto ent2 = c1.get_handle(second_guid);

	// check if components are intact after encode/decode cycle


	REQUIRE(2 == c1.entities_count());
	REQUIRE(2 == c2.entities_count());
	REQUIRE(ent1.has<components::transform>());
	const bool transform_intact = ent1.get<components::transform>() == first_transform;
	REQUIRE(transform_intact);
	REQUIRE(ent1.has<components::rigid_body>());
	REQUIRE(ent1.has<components::render>());
	REQUIRE(ent1.has<components::sprite>());
	REQUIRE(!ent1.has<components::trace>());

	REQUIRE(ent2.has<components::transform>());
	const bool default_transform_intact = ent2.get<components::transform>() == components::transform();
	REQUIRE(default_transform_intact);
	REQUIRE(!ent2.has<components::rigid_body>());
	REQUIRE(!ent2.has<components::render>());
	REQUIRE(!ent2.has<components::sprite>());
	REQUIRE(ent2.has<components::trace>());

	{
		augs::stream comparatory;

		REQUIRE(!cosmic_delta::encode(c1, c2, comparatory));

		REQUIRE(1 == comparatory.size());
		REQUIRE(c1 == c2);
	}
}

TEST_CASE("CosmicDelta EmptyAndCreatedThreeEntitiesWithReferences") {
	{
		// (in)sanity check
		cosmos t(3);
		const auto id = t.create_entity("");
		t.delete_entity(id);
		const auto new_id = t.create_entity("");
		REQUIRE(id != new_id);
	}

	cosmos c1(3);
	// increment the local id counter in the base cosmos to have different ids 
	// once it allocates its own entities when decoding
	{
		entity_id ids[3] = {
			c1.create_entity("e1"),
			c1.create_entity("e2"),
			c1.create_entity("e3")
		};

		for(const auto i : ids) {
			c1.delete_entity(i);
		}
	}
	cosmos c2(3);

	const auto new_ent1 = c2.create_entity("e1");
	const auto new_ent2 = c2.create_entity("e2");
	const auto new_ent3 = c2.create_entity("e3");

	const auto first_guid = new_ent1.get_guid();
	const auto second_guid = new_ent2.get_guid();
	const auto third_guid = new_ent3.get_guid();

	new_ent1 += components::position_copying();
	new_ent2 += components::position_copying();
	new_ent3 += components::position_copying();

	new_ent1.get<components::position_copying>().set_target(new_ent2);
	new_ent2.get<components::position_copying>().set_target(new_ent3);
	new_ent3.get<components::position_copying>().set_target(new_ent1);

	new_ent1 += components::sentience();
	new_ent1.map_child_entity(child_entity_name::CHARACTER_CROSSHAIR, new_ent2);

	{
		augs::stream s;

		cosmic_delta::encode(c1, c2, s);
		cosmic_delta::decode(c1, s);
	}

	REQUIRE(3 == c1.entities_count());
	REQUIRE(3 == c2.entities_count());

	const auto deco_ent1 = c1.get_handle(first_guid);
	const auto deco_ent2 = c1.get_handle(second_guid);
	const auto deco_ent3 = c1.get_handle(third_guid);

	REQUIRE(deco_ent1.has<components::sentience>());
	REQUIRE(deco_ent1.has<components::position_copying>());
	const auto& s1 = new_ent1.get<components::sentience>();
	const auto& s2 = deco_ent1.get<components::sentience>();
	const bool pc1_intact = deco_ent1.get<components::position_copying>().target == deco_ent2.get_id();
	const bool pc1ch_intact = deco_ent1[child_entity_name::CHARACTER_CROSSHAIR] == deco_ent2.get_id();
	REQUIRE(pc1_intact);
	REQUIRE(pc1ch_intact);

	REQUIRE(deco_ent2.has<components::position_copying>());
	const bool pc2_intact = deco_ent2.get<components::position_copying>().target == deco_ent3.get_id();
	REQUIRE(pc2_intact);

	REQUIRE(deco_ent3.has<components::position_copying>());
	const bool pc3_intact = deco_ent3.get<components::position_copying>().target == deco_ent1.get_id();
	REQUIRE(pc3_intact);

	{
		augs::stream comparatory;

		REQUIRE(!cosmic_delta::encode(c1, c2, comparatory));

		REQUIRE(1 == comparatory.size());
	}
}


TEST_CASE("CosmicDelta ThreeEntitiesWithReferencesAndDestroyedChild") {
	entity_guid c1_first_guid = 0;
	entity_guid c1_second_guid = 0;
	entity_guid c1_third_guid = 0;
	entity_guid c2_first_guid = 0;
	entity_guid c2_second_guid = 0;
	entity_guid c2_third_guid = 0;

	cosmos c1(3);
	{
		const auto new_ent1 = c1.create_entity("e1");
		const auto new_ent2 = c1.create_entity("e2");
		const auto new_ent3 = c1.create_entity("e3");

		c1_first_guid = new_ent1.get_guid();
		c1_second_guid = new_ent2.get_guid();
		c1_third_guid = new_ent3.get_guid();

		new_ent1 += components::position_copying();
		new_ent2 += components::position_copying();
		new_ent3 += components::position_copying();

		new_ent1.get<components::position_copying>().set_target(new_ent2);
		new_ent2.get<components::position_copying>().set_target(new_ent3);
		new_ent3.get<components::position_copying>().set_target(new_ent1);

		new_ent1 += components::sentience();
		new_ent1.map_child_entity(child_entity_name::CHARACTER_CROSSHAIR, new_ent2);
	}

	cosmos c2(3);
	{
		const auto new_ent1 = c2.create_entity("e1");
		const auto new_ent2 = c2.create_entity("e2");
		const auto new_ent3 = c2.create_entity("e3");

		c2_first_guid = new_ent1.get_guid();
		c2_second_guid = new_ent2.get_guid();
		c2_third_guid = new_ent3.get_guid();

		new_ent1 += components::position_copying();
		new_ent2 += components::position_copying();
		new_ent3 += components::position_copying();

		new_ent1.get<components::position_copying>().set_target(new_ent2);
		new_ent2.get<components::position_copying>().set_target(new_ent3);
		new_ent3.get<components::position_copying>().set_target(new_ent1);

		new_ent1 += components::sentience();
		new_ent1.map_child_entity(child_entity_name::CHARACTER_CROSSHAIR, new_ent2);
	}

	REQUIRE(3 == c1.entities_count());
	REQUIRE(3 == c2.entities_count());

	{
		augs::stream comparatory;

		REQUIRE(!cosmic_delta::encode(c1, c2, comparatory));

		REQUIRE(1 == comparatory.size());
	}

	c2.delete_entity(c2.get_handle(c2_second_guid));
	REQUIRE(2 == c2.entities_count());

	{
		augs::stream s;

		cosmic_delta::encode(c1, c2, s);
		cosmic_delta::decode(c1, s);
	}

	{
		augs::stream comparatory;

		REQUIRE(!cosmic_delta::encode(c1, c2, comparatory));

		REQUIRE(1 == comparatory.size());
	}

	REQUIRE(2 == c1.entities_count());

	const auto ent1 = c1.get_handle(c1_first_guid);
	REQUIRE(!c1.entity_exists_with_guid(c1_second_guid));
	const auto ent3 = c1.get_handle(c1_third_guid);

	REQUIRE(ent1.has<components::position_copying>());
	const bool pc1_dead = c1[ent1.get<components::position_copying>().target].dead();
	REQUIRE(pc1_dead);

	REQUIRE(ent3.has<components::position_copying>());
	const bool pc3_intact = ent3.get<components::position_copying>().target == ent1.get_id();
	REQUIRE(pc3_intact);

	LOG_NVPS(sizeof(assets_manager));
}

#endif
