#if BUILD_UNIT_TESTS
#include <tuple>
#include <catch.hpp>
#include <cstring>

#include "augs/filesystem/file.h"
#include "augs/templates/introspection_utils/describe_fields.h"
#include "augs/readwrite/delta_compression.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/cosmic_delta.h"
#include "game/organization/all_component_includes.h"

#include "augs/templates/introspection_utils/rewrite_members.h"

#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/lua_readwrite.h"

TEST_CASE("CosmicDelta0 PaddingSanityCheck1") {
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

TEST_CASE("CosmicDelta1 PaddingSanityCheck2") {
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


TEST_CASE("CosmicDelta2 PaddingTest") {
	std::string component_size_information;
	std::size_t total_components_size = 0u;

	auto assert_component_trivial = [](auto c) {
		using checked_type = decltype(c);
		
		static_assert(
			augs::is_byte_readwrite_safe_v<augs::stream, checked_type> || allows_nontriviality_v<checked_type>,
			"Non-trivially copyable component detected! If you need a non-trivial component, explicitly define static constexpr bool allow_nontriviality = true; within the class"
		);
	};

	auto padding_checker = [&](auto c, auto... args) {
		using checked_type = decltype(c);
		static_assert(std::is_same_v<std::decay_t<checked_type>, checked_type>, "Something's wrong with the types");

		total_components_size += sizeof(checked_type);
		component_size_information += typesafe_sprintf("%x == sizeof %x\n", sizeof(checked_type), typeid(checked_type).name());

		if constexpr(!allows_nontriviality_v<checked_type>) {
			constexpr size_t type_size = sizeof(checked_type);

			char buf1[type_size];
			char buf2[type_size];

			for (int i = 0; i < type_size; ++i) {
				buf1[i] = 3;
				buf2[i] = 4;
			}

			// it looks like the placement new may zero-out the memory before allocation.
			// we will leave this test as it is useful anyway.

			new (buf1) checked_type(args...);
			new (buf2) checked_type(args...);

			int iter = 0;
			bool same = true;

			for (; iter < type_size; ++iter) {
				if (buf1[iter] != buf2[iter]) {
					same = false;
					break;
				}
			}

			if(!same) {
				const auto log_contents = typesafe_sprintf(
					"Padding is wrong, or a variable is uninitialized in %x\nsizeof: %x\nDivergence position: %x",
					typeid(checked_type).name(),
					type_size,
					iter
				);

				augs::create_text_file(LOG_FILES_DIR "object1.txt", describe_fields(*(checked_type*)buf1));
				augs::create_text_file(LOG_FILES_DIR "object2.txt", describe_fields(*(checked_type*)buf2));

				LOG(log_contents);
				FAIL(log_contents);
			}

			// test by delta
			{
				auto a = checked_type(args...);
				auto b = checked_type(args...);

				const auto dt = augs::object_delta<checked_type>(a, b);

				if (dt.has_changed()) {
					const auto log_contents = typesafe_sprintf(
						"Padding is wrong, or a variable is uninitialized in %x\nsizeof: %x\nDivergence position: %x",
						typeid(checked_type).name(),
						type_size,
						static_cast<int>(dt.get_first_divergence_pos())
					);

					augs::create_text_file(LOG_FILES_DIR "object1.txt", describe_fields(a));
					augs::create_text_file(LOG_FILES_DIR "object2.txt", describe_fields(b));

					LOG(log_contents);
					FAIL(log_contents);
				}
			}

			// prove by introspection that all members are directly next to each other in memory
			const auto breaks = determine_breaks_in_fields_continuity_by_introspection(checked_type(args...));

			if (breaks.size() > 0) {
				LOG(breaks);
				LOG(describe_fields(checked_type(args...)));

				FAIL(typesafe_sprintf(
					"Padding is wrong, or a variable is uninitialized in %x\nsizeof: %x\n", 
					typeid(checked_type).name(),
					type_size
				));
			}
		}
	};

	for_each_through_std_get(component_list_t<std::tuple>(), assert_component_trivial);
	for_each_through_std_get(component_list_t<std::tuple>(), padding_checker);

	padding_checker(item_slot_transfer_request());

	component_size_information += typesafe_sprintf("Total size in bytes: %x", total_components_size);
	augs::create_text_file(LOG_FILES_DIR "components.txt", component_size_information);

	/* Validate cosmos_metadata. It will also be written and compared. */

	cosmos_metadata meta;

	augs::introspect(
		augs::recursive([&](auto&& self, auto, auto m) {
			using T = std::decay_t<decltype(m)>;

			if constexpr(std::is_same_v<T, augs::delta>) {
				padding_checker(m, augs::delta::zero);
			}
			else if constexpr(augs::is_byte_readwrite_safe_v<augs::stream, T> && !is_introspective_leaf_v<T>) {
				padding_checker(m);
			}
			else if constexpr(has_introspect_v<T>){
				augs::introspect(augs::recursive(self), m);
			}
		}),
		meta
	);
}

#if !STATICALLY_ALLOCATE_ENTITIES_NUM
/* Too much space would be wasted. */

TEST_CASE("CosmicDelta3 GuidizeTests") {
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

TEST_CASE("Cosmos ComparisonTest") {
	cosmos c1(2);
	cosmos c2(2);

	REQUIRE(c1 == c2);
	REQUIRE(c1 == c1);
	REQUIRE(c2 == c2);

	{
		const auto new_ent1 = c1.create_entity("e1");
		const auto new_ent2 = c1.create_entity("e2");
		
		components::transform first_transform(21, 0, 12.4f);
		
		new_ent1 += first_transform;
		new_ent1 += components::rigid_body();
		new_ent1 += components::render();
		new_ent1 += components::sprite();
		
		components::position_copying p;
		p.target = new_ent1;
		new_ent2 += components::transform();
		new_ent2 += components::trace();
		new_ent2 += p;
	}

	{
		const auto new_ent1 = c2.create_entity("e1");
		const auto new_ent2 = c2.create_entity("e2");
		
		components::transform first_transform(21, 0, 12.4f);
		
		new_ent1 += first_transform;
		new_ent1 += components::rigid_body();
		new_ent1 += components::render();
		new_ent1 += components::sprite();
		
		components::position_copying p;
		p.target = new_ent1;
		new_ent2 += components::transform();
		new_ent2 += components::trace();
		new_ent2 += p;
	}
	
	REQUIRE(c1 == c1);
	REQUIRE(c2 == c2);
	REQUIRE(c1 == c2);
}

TEST_CASE("CosmicDelta4 EmptyAndTwoNew") {
	cosmos c1(2);
	cosmos c2(2);
	
	REQUIRE(c1 == c2);

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
		REQUIRE(c1 == c2);
	}

	const auto ent1 = c1[first_guid];
	const auto ent2 = c1[second_guid];

	// check if components are intact after encode/decode cycle


	REQUIRE(2 == c1.get_entities_count());
	REQUIRE(2 == c2.get_entities_count());
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

	// test removals of components
	new_ent1.remove<components::rigid_body>();
	new_ent2.remove<components::trace>();
	new_ent2.remove<components::position_copying>();

	{
		augs::stream s;

		cosmic_delta::encode(c1, c2, s);
		cosmic_delta::decode(c1, s);
		REQUIRE(c1 == c2);
	}

	REQUIRE(!ent1.has<components::rigid_body>());
	REQUIRE(!ent2.has<components::trace>());
	REQUIRE(!ent2.has<components::position_copying>());

	// if both remove a transform, cosmoi shall be identical
	new_ent1.remove<components::transform>();
	ent1.remove<components::transform>();

	{
		augs::stream comparatory;

		REQUIRE(!cosmic_delta::encode(c1, c2, comparatory));

		REQUIRE(1 == comparatory.size());
		REQUIRE(c1 == c2);
	}
}

TEST_CASE("CosmicDelta5 EmptyAndCreatedThreeEntitiesWithReferences") {
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
		// should be byte-wise different due to different entity_id values
		REQUIRE(c1 != c2);
	}

	REQUIRE(3 == c1.get_entities_count());
	REQUIRE(3 == c2.get_entities_count());

	const auto deco_ent1 = c1[first_guid];
	const auto deco_ent2 = c1[second_guid];
	const auto deco_ent3 = c1[third_guid];

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


TEST_CASE("CosmicDelta6 ThreeEntitiesWithReferencesAndDestroyedChild") {
	// Insanity check
	REQUIRE(cosmos(1) == cosmos(1));
	REQUIRE(cosmos(3) == cosmos(3));

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

	REQUIRE(c1_first_guid == c2_first_guid);
	REQUIRE(c1_second_guid == c2_second_guid);
	REQUIRE(c1_third_guid == c2_third_guid);

	REQUIRE(3 == c1.get_entities_count());
	REQUIRE(3 == c2.get_entities_count());

	{
		augs::stream comparatory;

		REQUIRE(!cosmic_delta::encode(c1, c2, comparatory));

		REQUIRE(1 == comparatory.size());
	}

	c2.delete_entity(c2[c2_second_guid]);
	REQUIRE(2 == c2.get_entities_count());

	{
		augs::stream s;

		/*
			Note: when components with entity_ids are delta compared,
			firstly the fields with entity_id are replaced with entity_guid contents.
			This is because entity_ids are fields whose value we permit to be unique to the current machine,
			and the outputs of delta are supposed to be in a format correct for a network transfer.
			Thus, it is completely incorrect to compare entity_ids as they are,
			since the pool can assign them completely different values (despite them pointing to entities with identical guids), let alone if they are simply pointers.

			When the entity with c2_second_guid is deleted in c2, that guid can no longer be retrieved in c2 by the time we perform delta,
			making it impossible to detect that the position_copying components of ent1 of both c1 and c2 represent the same guid.
			Therefore, the cosmic delta has no other choice but to substitute the dead entity_id in c2->ent1->position_copying with zero-valued entity_guid which is defined to be "null guid".
			This in effect will detect a difference between the position_copying components of c1->ent1 and c2->ent1. 
			That is because we will substitute an alive guid for the former, and the null guid for the latter.

			This notice exists so that there is no more confusion when delta detects 1 changed entity and 1 deleted entity when the only thing we did was a deletion of a single entity.
		*/

		cosmic_delta::encode(c1, c2, s);
		cosmic_delta::decode(c1, s);
	}
	REQUIRE(2 == c1.get_entities_count());

	{
		augs::stream comparatory;

		REQUIRE(!cosmic_delta::encode(c1, c2, comparatory));
		
		/* 
			The following REQUIRE(c1 != c2) may seem counter-intuitive.
			This, however, is actually the expected behaviour if we use straight-serializable entity_ids that let us avoid checking entity_ids everywhere in the cosmos for resetting
			every time that an entity of a given entity_id is destroyed.

			Read the previous comment and consider what happens when we encode c2.
			Notice that we intend NO CHANGE to c2, (which, by the way, is taken by const&) since it is the cosmos we encode against.
			Delta will detect a difference between entity_ids in c1->ent1->position_copying and c2->ent1->position_copying, 
			supplying a default, unset entity_id for c1->ent1->position_copying.
			c2->ent1->position_copying however, will NOT change AT ALL (as it was const& all the time), 
			and despite it pointing to the already dead ent2, it will remain set to some value (that the pool correctly recognizes as dead), 
			and therefore byte-wisely different from the default (entity_id()) (which is now the value for c1->ent1->position_copying).

		*/
		
		// remove this REQUIRE if we switch entity_ids to types that need manual resetting upon deletion of an object,
		// for example pointer types that are set to null every time that the object pointed to is deleted.
		// In such case, this should become REQUIRE(c1 == c2).
		REQUIRE(c1 != c2);
		c2[c2_first_guid].get<components::position_copying>().target = entity_id();
		// note: we were also setting this child to ent2, so we need to nullify it too before ensuring equality
		c2[c2_first_guid].map_child_entity(child_entity_name::CHARACTER_CROSSHAIR, entity_id());
		
		// Only now shall the two cosmoi be equal. (the following expression is  equivalent to c1 == c2)
		REQUIRE(c1.significant.get_first_mismatch_pos(c2.significant) == -1);

		REQUIRE(1 == comparatory.size());
	}

	const auto ent1 = c1[c1_first_guid];
	REQUIRE(!c1.entity_exists_by(c1_second_guid));
	const auto ent3 = c1[c1_third_guid];

	REQUIRE(ent1.has<components::position_copying>());
	const bool pc1_dead = c1[ent1.get<components::position_copying>().target].dead();
	REQUIRE(entity_id() == ent1.get<components::position_copying>().target);
	REQUIRE(pc1_dead);

	REQUIRE(ent3.has<components::position_copying>());
	const bool pc3_intact = ent3.get<components::position_copying>().target == ent1.get_id();
	REQUIRE(pc3_intact);
}
#endif

#endif
