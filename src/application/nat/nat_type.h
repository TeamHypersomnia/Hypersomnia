#pragma once
#include "augs/network/port_type.h"
#include "augs/graphics/rgba.h"

enum class nat_type : uint8_t {
	PUBLIC_INTERNET,
	PORT_PRESERVING_CONE,
	CONE,
	ADDRESS_SENSITIVE,
	PORT_SENSITIVE
};

namespace augs {
	template <class T>
	std::string enum_to_string(const T e);

	template <>
	inline std::string enum_to_string(const nat_type e) {
		constexpr std::array<const char*, 5> vals = {
			"PUBLIC_INTERNET",
			"PORT_PRESERVING_CONE",
			"CONE",
			"ADDRESS_SENSITIVE",
			"PORT_SENSITIVE"
		};

		if (static_cast<uint8_t>(e) < static_cast<uint8_t>(vals.size())) {
			return vals[static_cast<uint8_t>(e)];
		}

		return "UnknownEnumValue";
	}
}

struct nat_detection_result {
	static constexpr bool force_read_field_by_field = true;

	// GEN INTROSPECTOR struct nat_detection_result
	nat_type type = nat_type::PUBLIC_INTERNET;
	int port_delta = 0;
	port_type predicted_next_port = 0;
	// END GEN INTROSPECTOR

	bool operator==(const nat_detection_result&) const = default;

	std::string describe() const;
};

std::string nat_type_to_string(nat_type);
rgba nat_type_to_color(nat_type);
