#pragma once
#include <cstdint>
#include <string>
#include "augs/misc/constant_size_string.h"

/*
	One server.key per host: all its instances (casual, ranked, tournament matches) have the same key and tell themselves
	apart by a label the launcher gives them - c1, c2, r1, t1, a1 - in their signed heartbeat and to the players.
	A join may name one ("c2-<key>", see connect_target.h).
*/

enum class server_instance_type : uint8_t {
	CASUAL,
	RANKED,
	AURA,
	TOURNAMENT
};

using instance_label_type = augs::constant_size_string<8>;

inline instance_label_type make_instance_label(const server_instance_type type, const uint16_t number) {
	const auto letter = [type]() {
		switch (type) {
			case server_instance_type::RANKED: return "r";
			case server_instance_type::AURA: return "a";
			case server_instance_type::TOURNAMENT: return "t";
			default: return "c";
		}
	}();

	return instance_label_type(std::string(letter) + std::to_string(number));
}

/*
	A lowercase letter and a number from 1, at most 4 digits.
*/

inline bool is_instance_label(const std::string& s) {
	if (s.size() < 2 || s.size() > 5 || s[0] < 'a' || s[0] > 'z' || s[1] == '0') {
		return false;
	}

	for (std::size_t i = 1; i < s.size(); ++i) {
		if (s[i] < '0' || s[i] > '9') {
			return false;
		}
	}

	return true;
}
