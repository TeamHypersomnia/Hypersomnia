#pragma once
#include <cstdint>

struct session_id_type {
	using id_value_type = uint32_t;
	// GEN INTROSPECTOR struct session_id_type
	id_value_type value = static_cast<id_value_type>(-1);
	// END GEN INTROSPECTOR

	static auto dead() {
		return session_id_type(static_cast<id_value_type>(0));
	}

	static auto first() {
		return session_id_type(1);
	}

	session_id_type() = default;
	explicit session_id_type(const id_value_type b) : value(b) {}
	
	bool operator==(const session_id_type& b) const {
		return value == b.value;
	}

	bool operator!=(const session_id_type& b) const {
		return value != b.value;
	}

	auto& operator++() {
		++value;
		return *this;
	}

	bool operator<(const session_id_type& b) const {
		return value < b.value;
	}

	explicit operator id_value_type() const {
		return value;
	}

	bool is_set() const {
		return *this != session_id_type();
	}

	void unset() {
		*this = {};
	}
};



