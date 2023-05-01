#if __FAST_MATH__
#error "Don't do this"
#endif

#if BUILD_UNIT_TESTS
#include <Catch/single_include/catch2/catch.hpp>
#include "game/cosmos/cosmos.h"
#include "augs/misc/pool/pool_allocate.h"
#include "augs/misc/trivially_copyable_tuple.h"
#include "augs/templates/logically_empty.h"
#include "game/cosmos/entity_id_declaration.h"

#include "game/components/transform_component.h"
#include "game/components/rigid_body_component.h"

#if !STATICALLY_ALLOCATE_ENTITIES

TEST_CASE("LogicallyEmpty") {
	{
		int a = 0;
		int b = 1;

		REQUIRE(logically_empty(a));
		REQUIRE(!logically_empty(b));
		REQUIRE(!logically_empty(a, b));
		REQUIRE(!logically_set(a, b));
		REQUIRE(logically_set(b, b));
		REQUIRE(logically_empty(a, a));
	}

	{
		std::vector<int> a;
		std::vector<int> b { 0 };

		REQUIRE(logically_empty(a));
		REQUIRE(!logically_empty(b));
		REQUIRE(!logically_empty(a, b));
		REQUIRE(!logically_set(a, b));
		REQUIRE(logically_set(b, b));
		REQUIRE(logically_empty(a, a));
	}
}

TEST_CASE("GetByDynamicId") {
	all_entity_types t;

	REQUIRE(20.0 == get_by_dynamic_index(t, std::size_t(0), [](auto){
		return 20.0;	
	}));

	REQUIRE(20.0 == get_by_dynamic_id(t, type_in_list_id<all_entity_types>(0), [](auto){
		return 20.0;	
	}));

	using candidates = type_list<plain_missile, plain_sprited_body>;

	auto tester = [](auto a) -> decltype(auto) {
		using T = remove_cref<decltype(a)>;

		if constexpr(std::is_same_v<T, plain_missile>) {
			return 4;
		}
		else if constexpr(std::is_same_v<T, plain_sprited_body>) {
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

	REQUIRE(4 == constrained_get_by_dynamic_id<candidates>(t, id_type::of<plain_missile>(), tester));
	REQUIRE(8949 == constrained_get_by_dynamic_id<candidates>(t, id_type::of<plain_sprited_body>(), tester));

	REQUIRE(-1 == constrained_find_by_dynamic_id<candidates>(t, id_type::of<controlled_character>(), tester));
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
		using T1 = components::transform;
		using T2 = components::rigid_body;
		using aggr = augs::trivially_copyable_tuple<T1, T2>;

		std::vector<aggr> pool;
		REQUIRE(pool.size() == 0);
		pool.push_back(aggr());
		REQUIRE(pool.size() == 1);
		pool.push_back(aggr());
		REQUIRE(pool.size() == 2);
	}
}
#endif

#endif