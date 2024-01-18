#pragma once
#include "augs/network/network_types.h"
#include "augs/templates/hash_templates.h"

struct mode_player_id {
	using id_value_type = uint8_t;
	// GEN INTROSPECTOR struct mode_player_id
	id_value_type value = static_cast<id_value_type>(-1);
	// END GEN INTROSPECTOR

	static auto dead() {
		return mode_player_id(static_cast<id_value_type>(-1));
	}

	static auto first() {
		return mode_player_id(0);
	}

	static auto machine_admin() {
		return mode_player_id(static_cast<id_value_type>(max_mode_players_v - 1));
	}

	mode_player_id() = default;
	explicit mode_player_id(const id_value_type b) : value(b) {}
	
	bool operator==(const mode_player_id& b) const {
		return value == b.value;
	}

	bool operator!=(const mode_player_id& b) const {
		return value != b.value;
	}

	auto& operator++() {
		++value;
		return *this;
	}

	bool operator<(const mode_player_id& b) const {
		return value < b.value;
	}

	explicit operator id_value_type() const {
		return value;
	}

	bool is_set() const {
		return *this != mode_player_id();
	}

	void unset() {
		*this = {};
	}

	auto hash() const {
		return augs::hash_multiple(value);
	}
};

namespace std {
	template <>
	struct hash<mode_player_id> {
		std::size_t operator()(const mode_player_id v) const {
			return v.hash();
		}
	};
}
