#pragma once
#include "augs/templates/type_mod_templates.h"
#include "augs/misc/pool/pool_declaration.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_file.h"
#include "augs/templates/can_stream.h"

const auto test_file_path = GENERATED_FILES_DIR "/test_byte_readwrite.bin";

template <class T>
void report(const T& v, const T& reloaded) {
	if constexpr(!augs::is_pool_v<T>) {
		LOG("Original %x\nReloaded: %x", v, reloaded);
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
		bytes = augs::to_bytes(v);

		augs::save_as_bytes(bytes, path);
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
		bytes = augs::to_bytes(v);

		augs::save_as_bytes(bytes, path);
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

