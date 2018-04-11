#pragma once
#include <string>

#include "augs/templates/type_in_list_id.h"
#include "augs/templates/instance_type.h"
#include "augs/templates/transform_types.h"

#include "augs/misc/value_meter.h"
#include "augs/misc/trivially_copyable_tuple.h"

#include "augs/graphics/rgba.h"

#include "game/assets/ids/image_id.h"
#include "game/common_state/entity_name_str.h"

struct sentience_meter_appearance {
	// GEN INTROSPECTOR struct sentience_meter_appearance
	assets::image_id icon;
	rgba bar_color;
	entity_name_str description;
	// END GEN INTROSPECTOR

	auto get_icon() const {
		return icon;
	}

	auto get_bar_color() const {
		return bar_color;
	}

	const auto& get_description() const {
		return description;
	}
};

struct health_meter_instance : value_meter {
	// GEN INTROSPECTOR struct health_meter_instance
	// INTROSPECT BASE value_meter
	// END GEN INTROSPECTOR
};

struct personal_electricity_meter_instance : value_meter {
	// GEN INTROSPECTOR struct personal_electricity_meter_instance
	// INTROSPECT BASE value_meter
	// END GEN INTROSPECTOR
};

struct consciousness_meter_instance : value_meter {
	// GEN INTROSPECTOR struct consciousness_meter_instance
	// INTROSPECT BASE value_meter
	// END GEN INTROSPECTOR
};

struct health_meter {
	using instance = health_meter_instance;
	// GEN INTROSPECTOR struct health_meter
	sentience_meter_appearance appearance;
	// END GEN INTROSPECTOR
};

struct personal_electricity_meter {
	using instance = personal_electricity_meter_instance;
	// GEN INTROSPECTOR struct personal_electricity_meter
	sentience_meter_appearance appearance;
	// END GEN INTROSPECTOR
};

struct consciousness_meter {
	using instance = consciousness_meter_instance;
	// GEN INTROSPECTOR struct consciousness_meter
	sentience_meter_appearance appearance;
	// END GEN INTROSPECTOR
};

template <template <class...> class List>
using meter_list_t = List<
	health_meter,
	personal_electricity_meter,
	consciousness_meter
>;

template <template <class...> class List>
using meter_instance_list_t = transform_types_in_list_t<
	meter_list_t<List>,
	instance_of
>;

using meter_instance_tuple = meter_instance_list_t<augs::trivially_copyable_tuple>;
using meter_id = type_in_list_id<meter_instance_tuple>;