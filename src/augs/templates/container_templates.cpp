#if BUILD_UNIT_TESTS
#include <vector>
#include <catch.hpp>
#include "augs/templates/container_templates.h"

TEST_CASE("Templates ContainerTemplates") {
	using v_t = std::vector<int>;
	v_t v = { 0, 1, 2, 3, 4 };
	erase_from_to(v, 0, 2);
	REQUIRE(v == v_t { 2, 3, 4 } );

	erase_from_to(v, 0);
	REQUIRE(v.empty());
	erase_from_to(v, 0);
	REQUIRE(v.empty());
}
#endif
