#if BUILD_UNIT_TESTS
#include "augs/misc/children_vector_tracker.h"
#include <Catch/single_include/catch2/catch.hpp>

TEST_CASE("ChildrenVectorTracker Tests") {
	augs::children_vector_tracker<int, int> abc;

	abc.assign_parenthood(1, 10);
	abc.assign_parenthood(2, 10);
	abc.assign_parenthood(3, 10);
	abc.assign_parenthood(4, 10);

	REQUIRE(abc.get_children_of(10) == std::vector<int>({ 1, 2, 3, 4 }));

	abc.unset_parenthood(4, 10);

	REQUIRE(abc.get_children_of(10) == std::vector<int>({ 1, 2, 3 }));

	abc.unset_parenthood(1, 10);

	REQUIRE(abc.get_children_of(10) == std::vector<int>({ 2, 3 }));

	abc.reassign_parenthood(2, 10, 20);
	abc.reassign_parenthood(3, 10, 20);

	REQUIRE(abc.get_children_of(20) == std::vector<int>({ 2, 3 }));
	REQUIRE(abc.get_children_of(10).empty());
}
#endif
