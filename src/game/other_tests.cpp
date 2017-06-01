#include "generated/setting_build_unit_tests.h"

#if BUILD_UNIT_TESTS
#include <catch.hpp>
#include "game/transcendental/cosmos.h"
#include "augs/misc/trivially_copyable_tuple.h"

TEST_CASE("Ca TriviallyCopyableTuple") {
	struct a {
		int mem = 1;
	};
	struct b {
		int mem = 2;
	};
	struct c {
		int mem = 3;
		int mem2 = 66;
	};

	int f = 2556;
	augs::trivially_copyable_tuple<a, b, c> tt;
	int e = 65;

	REQUIRE(std::get<a>(tt).mem == 1);
	REQUIRE(std::get<b>(tt).mem == 2);
	REQUIRE(std::get<c>(tt).mem == 3);
	REQUIRE(std::get<c>(tt).mem2 == 66);
	REQUIRE(e == 65);
	REQUIRE(f == 2556);

	{

		std::vector<augs::trivially_copyable_tuple<a, b, c>> pooled;
		REQUIRE(pooled.size() == 0);
		pooled.emplace_back();
		REQUIRE(pooled.size() == 1);
		pooled.push_back(augs::trivially_copyable_tuple<a, b, c>());
		REQUIRE(pooled.size() == 2);
	}

	{
		using aggr = augs::trivially_copyable_tuple<
			augs::pooled_object_id<components::transform>,
			augs::pooled_object_id<components::rigid_body>, 
			augs::pooled_object_id<components::render>
		>;

		std::vector<aggr> pool;
		REQUIRE(pool.size() == 0);
		pool.push_back(aggr());
		REQUIRE(pool.size() == 1);
		pool.push_back(aggr());
		REQUIRE(pool.size() == 2);
	}

	{

		using aggr2 = augs::trivially_copyable_tuple<
		augs::pooled_object_id< components::tree_of_npo_node>,
		augs::pooled_object_id< components::special_physics>,
		augs::pooled_object_id< components::animation>,
		augs::pooled_object_id< components::behaviour_tree>,
		augs::pooled_object_id< components::position_copying>,
		augs::pooled_object_id< components::crosshair>,
		augs::pooled_object_id< components::missile>,
		augs::pooled_object_id< components::flags>,
		augs::pooled_object_id< components::gun>,
		augs::pooled_object_id< components::rotation_copying>,
		augs::pooled_object_id< components::movement>,
		augs::pooled_object_id< components::particles_existence>,
		augs::pooled_object_id< components::pathfinding>,
		augs::pooled_object_id< components::rigid_body>,
		augs::pooled_object_id< components::render>,
		augs::pooled_object_id< components::transform>,
		augs::pooled_object_id< components::sprite>,
		augs::pooled_object_id< components::polygon>,
		augs::pooled_object_id< components::tile_layer_instance>,
		augs::pooled_object_id< components::car>,
		augs::pooled_object_id< components::driver>,
		augs::pooled_object_id< components::fixtures>,
		augs::pooled_object_id< components::container>,
		augs::pooled_object_id< components::item>,
		augs::pooled_object_id< components::force_joint>,
		augs::pooled_object_id< components::item_slot_transfers>,
		augs::pooled_object_id< components::name>,
		augs::pooled_object_id< components::trace>,
		augs::pooled_object_id< components::melee>,
		augs::pooled_object_id< components::sentience>,
		augs::pooled_object_id< components::attitude>,
		augs::pooled_object_id< components::processing>,
		augs::pooled_object_id< components::guid>,
		augs::pooled_object_id< components::child>,
		augs::pooled_object_id< components::interpolation>,
		augs::pooled_object_id< components::light>,
		augs::pooled_object_id< components::wandering_pixels>,
		augs::pooled_object_id< components::sound_existence>,
		augs::pooled_object_id< components::explosive>,
		augs::pooled_object_id< components::catridge>,
		augs::pooled_object_id< components::shape_polygon>,
		augs::pooled_object_id< components::shape_circle>,
		augs::pooled_object_id< components::motor_joint>,
		augs::pooled_object_id< components::hand_fuse>,
		augs::pooled_object_id< components::sender>,
		augs::pooled_object_id< components::all_inferred_state>
		>;

		static_assert(std::is_same_v<aggr2, decltype(cosmos_base::aggregate_type::component_ids)>, "Trait failed");
		std::vector<aggr2> pool;
		REQUIRE(pool.size() == 0);
		pool.emplace_back();
		REQUIRE(pool.size() == 1);
		pool.push_back(aggr2());
		REQUIRE(pool.size() == 2);
		pool.push_back(aggr2());
		REQUIRE(pool.size() == 3);

		const augs::trivially_copyable_tuple<int, double, float> aaa;
		std::get<0>(aaa);
		std::get<1>(aaa);
		std::get<2>(aaa);
		//std::get<float&>(aaa);
		std::get<int>(aaa);
		std::get<float>(aaa);

		sizeof(aggr2);
		for_each_through_std_get(aggr2(), [](auto...) {});
		//std::get<components::sentience>(aaa);
		//std::get<3>(aaa); // error
	}

	{
		static_assert(alignof(cosmos_base::aggregate_pool_type) == 4, "Trait failed");
		cosmos_base::aggregate_pool_type pool;
		pool.initialize_space(2);
		REQUIRE(pool.size() == 0);
		pool.allocate();
		REQUIRE(pool.size() == 1);
		pool.allocate();
		REQUIRE(pool.size() == 2);
	}

	{
		cosmos_base::aggregate_type test;
		test.set_id(augs::pooled_object_id<components::sentience>());
	}
}
#endif