#if !IS_PRODUCTION_BUILD
#if BUILD_UNIT_TESTS
#include "augs/log_direct.h"
#include <Catch/single_include/catch2/catch.hpp>
#include <cstring>
#include <array>

#include "augs/filesystem/file.h"
#include "augs/templates/introspection_utils/describe_fields.h"
#include "augs/string/get_type_name.h"
#include "augs/readwrite/delta_compression.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/cosmic_delta.h"
#include "game/organization/all_component_includes.h"
#include "game/organization/for_each_component_type.h"

#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/lua_readwrite.h"
#include "augs/log_path_getters.h"

TEST_CASE("StateTest0 PaddingSanityCheck1") {
	struct ok {
		bool a;
		int b;
		bool c;

		bool operator==(const ok&) const = default;

		ok() : a(false), b(1), c(false) {

		}
	};

	using checked_type = ok;
	constexpr size_t type_size = sizeof(checked_type);

	std::aligned_storage_t<type_size, alignof(checked_type)> buf1{};
	std::aligned_storage_t<type_size, alignof(checked_type)> buf2{};

	std::memset(&buf1, 3, type_size);
	std::memset(&buf2, 4, type_size);

	new (&buf1) checked_type;
	new (&buf2) checked_type;

	const bool are_different = std::memcmp(&buf1, &buf2, type_size);

	REQUIRE(are_different);
}

TEST_CASE("StateTest1 PaddingSanityCheck2") {
	struct ok {
		bool a = false;
		int b = 1;
		bool c = false;

		bool operator==(const ok&) const = default;
	};

	typedef ok checked_type;
	constexpr size_t type_size = sizeof(checked_type);

	std::aligned_storage_t<type_size, alignof(checked_type)> buf1{};
	std::aligned_storage_t<type_size, alignof(checked_type)> buf2{};

	std::memset(&buf1, 3, type_size);
	std::memset(&buf2, 4, type_size);

	new (&buf1) checked_type;
	new (&buf2) checked_type;

	const bool are_different = std::memcmp(&buf1, &buf2, type_size);

	REQUIRE(are_different);
}


TEST_CASE("StateTest2 PaddingTest") {
	auto assert_component_trivial = [](auto c) {
		using checked_type = decltype(c);
		
		static_assert(
			augs::is_byte_readwrite_safe_v<augs::memory_stream, checked_type> || allows_nontriviality_v<checked_type>,
			"Non-trivially copyable component/invariant detected! If you need a non-trivial component/invariant"
			", explicitly define static constexpr bool allow_nontriviality = true; within the class"
		);
	};

	auto padding_checker = [&](auto c, auto... args) {
		using checked_type = decltype(c);
		static_assert(std::is_same_v<remove_cref<checked_type>, checked_type>, "Something's wrong with the types");

		if constexpr(!allows_nontriviality_v<checked_type>) {
			constexpr std::size_t type_size = sizeof(checked_type);

			std::aligned_storage_t<type_size, alignof(checked_type)> buf1{};
			std::aligned_storage_t<type_size, alignof(checked_type)> buf2{};

			std::memset(&buf1, 3, type_size);
			std::memset(&buf2, 4, type_size);

			// it looks like the placement new may zero-out the memory before allocation.
			// we will leave this test as it is useful anyway.

			new (&buf1) checked_type(args...);
			new (&buf2) checked_type(args...);

			std::size_t iter = 0;
			bool same = true;

			for (; iter < type_size; ++iter) {
				if (reinterpret_cast<const char*>(&buf1)[iter] != reinterpret_cast<const char*>(&buf2)[iter]) {
					same = false;
					break;
				}
			}

			if (!same) {
				const auto log_contents = typesafe_sprintf(
					"Padding is wrong, or a variable is uninitialized in %x\nsizeof: %x\nDivergence position: %x",
					get_type_name<checked_type>(),
					type_size,
					iter
				);

				augs::save_as_text(get_path_in_log_files("object1.txt"), describe_fields(*reinterpret_cast<checked_type*>(&buf1)));
				augs::save_as_text(get_path_in_log_files("object2.txt"), describe_fields(*reinterpret_cast<checked_type*>(&buf2)));

				LOG_NOFORMAT(log_contents);
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
						get_type_name<checked_type>(),
						type_size,
						static_cast<int>(dt.get_first_divergence_pos())
					);

					augs::save_as_text(get_path_in_log_files("object1.txt"), describe_fields(a));
					augs::save_as_text(get_path_in_log_files("object2.txt"), describe_fields(b));

					LOG_NOFORMAT(log_contents);
					FAIL(log_contents);
				}
			}

			// prove by introspection that all members are directly next to each other in memory
			const auto breaks = determine_breaks_in_fields_continuity_by_introspection(checked_type(args...));

			if (breaks.size() > 0) {
				LOG_NOFORMAT(breaks);
				LOG_NOFORMAT(describe_fields(checked_type(args...)));

				FAIL(typesafe_sprintf(
					"Padding is wrong, or a variable is uninitialized in %x\nsizeof: %x\n", 
					get_type_name<checked_type>(),
					type_size
				));
			}
		}
	};

	for_each_component_type(assert_component_trivial);
	for_each_component_type(padding_checker);

	for_each_invariant_type(assert_component_trivial);
	for_each_invariant_type(padding_checker);

	padding_checker(item_slot_transfer_request());

	padding_checker(augs::pool_indirector<unsigned short>());
	padding_checker(augs::pool_slot<unsigned short>());

	padding_checker(augs::pool_indirector<unsigned>());
	padding_checker(augs::pool_slot<unsigned>());
	
#if 0
#if !STATICALLY_ALLOCATE_ENTITIES
	/* TODO: Fix this actually */
	/* Too much space would be wasted and stack overflows would occur. */

	cosmos_common_significant common;

	augs::introspect(
		augs::recursive([padding_checker](auto self, auto, auto m) {
			using T = remove_cref<decltype(m)>;

			if constexpr(std::is_same_v<T, augs::delta>) {
				padding_checker(m, augs::delta::zero);
			}
			else if constexpr(augs::is_byte_readwrite_safe_v<augs::memory_stream, T> && !is_introspective_leaf_v<T>) {
				padding_checker(m);
			}
			else if constexpr(is_container_v<T>){
				auto t = typename T::value_type();
				self(self, t, t);
			}
			else if constexpr(has_introspect_v<T>){
				augs::introspect(augs::recursive(self), m);
			}
		}),
		common
	);
#endif
#endif
}
#endif
#endif
