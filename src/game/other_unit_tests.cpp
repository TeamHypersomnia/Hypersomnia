
#if BUILD_UNIT_TESTS
#include <catch.hpp>
#include "game/cosmos/cosmos.h"
#include "augs/misc/trivially_copyable_tuple.h"

#if !STATICALLY_ALLOCATE_ENTITIES

TEST_CASE("GetByDynamicId") {
	all_entity_types t;

	REQUIRE(20.0 == get_by_dynamic_index(t, std::size_t(0), [](auto){
		return 20.0;	
	}));

	REQUIRE(20.0 == get_by_dynamic_id(t, type_in_list_id<all_entity_types>(), [](auto){
		return 20.0;	
	}));

	using candidates = type_list<plain_missile, explosive_missile>;

	auto tester = [](auto a) -> decltype(auto) {
		using T = remove_cref<decltype(a)>;
		static_assert( || same<T, explosive_missile>);

		if constexpr(std::is_same_v<T, plain_missile>) {
			return 4;
		}
		else if constexpr(std::is_same_v<T, explosive_missile>) {
			return 8949;
		}
		else if constexpr(std::is_same_v<T, std::nullopt_t>) {
			return -1;
		}
		else {
			static_assert(always_false_v<T>, "Non-exhaustive tester");
			return 0;
		}
	};

	using id_type = type_in_list_id<all_entity_types>;

	REQUIRE(4 == conditional_get_by_dynamic_id<candidates>(t, id_type::of<plain_missile>(), tester));
	REQUIRE(8949 == conditional_get_by_dynamic_id<candidates>(t, id_type::of<explosive_missile>(), tester));

	REQUIRE(-1 == conditional_find_by_dynamic_id<candidates>(t, id_type::of<controlled_character>(), tester));
}

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
			cosmic_object_pool_id<components::transform>,
			cosmic_object_pool_id<components::rigid_body>, 
		>;

		std::vector<aggr> pool;
		REQUIRE(pool.size() == 0);
		pool.push_back(aggr());
		REQUIRE(pool.size() == 1);
		pool.push_back(aggr());
		REQUIRE(pool.size() == 2);
	}

	{

		using aggr2 = decltype(cosmic_entity::component_ids);

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
		entity_pool_type pool(2);
		REQUIRE(pool.size() == 0);
		pool.allocate();
		REQUIRE(pool.size() == 1);
		pool.allocate();
		REQUIRE(pool.size() == 2);
	}

	{
	}
}
#endif

#endif