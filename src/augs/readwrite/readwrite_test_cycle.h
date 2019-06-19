#pragma once
#include "augs/templates/type_mod_templates.h"
#include "augs/misc/pool/pool_declaration.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_file.h"
#include "augs/readwrite/lua_file.h"
#include "augs/templates/can_stream.h"
#include "augs/readwrite/to_bytes.h"

const auto test_file_path = GENERATED_FILES_DIR "/test_byte_readwrite.bin";
const auto test_lua_file_path = GENERATED_FILES_DIR "/test_lua_readwrite.lua";

namespace detail {
	struct dummy_A {
		// GEN INTROSPECTOR struct detail::A
		int a = 1;
		// END GEN INTROSPECTOR

		bool operator==(const dummy_A& r) const {
			return a == r.a;
		}
	};

	struct dummy_B : dummy_A {
		using introspect_base = dummy_A;
		// GEN INTROSPECTOR struct detail::B
		std::optional<int> b;
		// END GEN INTROSPECTOR

		bool operator==(const dummy_B& r) const {
			return b == r.b && dummy_A::operator==(r);
		}
	};

	struct dummy_C : dummy_B {
		using introspect_base = dummy_B;
		// GEN INTROSPECTOR struct detail::C
		int c = 3;
		// END GEN INTROSPECTOR

		bool operator==(const dummy_C& r) const {
			return c == r.c && dummy_B::operator==(r);
		}
	};

	enum class dummy_enum {
		// GEN INTROSPECTOR enum class detail::dummy_enum
		INVALID,
		_1,
		_2,
		_3,
		_4,
		COUNT
		// END GEN INTROSPECTOR
	};
}

template <class T>
void report(const T& v, const T& reloaded) {
	if constexpr(!std::is_base_of_v<detail::dummy_A, T> && !augs::is_pool_v<T>) {
		LOG("(%x)\nOriginal: %x\nReloaded: %x", get_type_name<T>(), v, reloaded);
	}
}

template <class T>
auto& ref_or_get(T& v) {
	if constexpr(is_unique_ptr_v<std::remove_const_t<T>>) {
		return *v.get();
	}
	else {
		return v;
	}
}

static_assert(std::is_same_v<decltype(ref_or_get(std::declval<std::unique_ptr<std::vector<int>>&>())), std::vector<int>&>);

template <class A, class B>
bool report_compare(const A& tmp, const B& v) {
	const auto& vv = ref_or_get(v);
	const auto success = vv == tmp;

	if (!success) {
		report(tmp, vv);
	}

	return success;
}

template <class T>
bool try_to_reload_with_file(T& v) {
	if constexpr(!augs::is_pool_v<T>) {
		if (!(v == v)) {
			return false;
		}
	}

	const auto& path = test_file_path;
	const auto tmp = ref_or_get(v);

	augs::save_as_bytes(v, path);
	augs::load_from_bytes(v, path);

	augs::remove_file(path);

	if constexpr(augs::is_pool_v<T>) {
		(void)tmp;
		return true;
	}
	else {
		return report_compare(tmp, v);
	}
}

template <class T>
bool try_to_reload_with_bytes(T& v) {
	if constexpr(!augs::is_pool_v<T>) {
		if (!(v == v)) {
			return false;
		}
	}

	const auto& path = test_file_path;

	const auto tmp = ref_or_get(v);

	{
		std::vector<std::byte> bytes;
		augs::assign_bytes(bytes, v);

		augs::bytes_to_file(bytes, path);
	}

	augs::load_from_bytes(v, path);
	augs::remove_file(path);

	if constexpr(augs::is_pool_v<T>) {
		(void)tmp;
		return true;
	}
	else {
		return report_compare(tmp, v);
	}
}

template <class T>
bool try_to_reload_with_lua(sol::state& lua, T& v) {
	if constexpr(!augs::is_pool_v<T>) {
		if (!(v == v)) {
			return false;
		}
	}

	const auto& path = test_lua_file_path;

	const auto tmp = ref_or_get(v);

	augs::save_as_lua_table(lua, v, path);
	augs::load_from_lua_table(lua, v, path);
	augs::remove_file(path);

	if constexpr(augs::is_pool_v<T>) {
		(void)tmp;
		return true;
	}
	else {
		return report_compare(tmp, v);
	}
}

template <class T>
bool try_to_reload_with_memory_stream(T& v) {
	if constexpr(!augs::is_pool_v<T>) {
		if (!(v == v)) {
			return false;
		}
	}

	const auto& path = test_file_path;

	const auto tmp = ref_or_get(v);

	{
		std::vector<std::byte> bytes;
		augs::assign_bytes(bytes, v);

		augs::bytes_to_file(bytes, path);
	}

	auto bytes = augs::file_to_bytes(path);
	augs::memory_stream ss = std::move(bytes);
	augs::read_bytes(ss, v);

	augs::remove_file(path);

	if constexpr(augs::is_pool_v<T>) {
		(void)tmp;
		return true;
	}
	else {
		return report_compare(tmp, v);
	}
}

#define readwrite_test_cycle(variable) \
REQUIRE(try_to_reload_with_file(variable)); \
REQUIRE(try_to_reload_with_bytes(variable)); \
REQUIRE(try_to_reload_with_memory_stream(variable));

