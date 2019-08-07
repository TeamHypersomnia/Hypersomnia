#pragma once
#include <vector>
#include <optional>

#include "augs/misc/timing/delta.h"
#include "augs/misc/pool/pooled_object_id.h"

#include "game/cosmos/entity_id.h"
#include "game/cosmos/entity_handle_declaration.h"

#include "game/components/transform_component.h"

#include "view/audiovisual_state/systems/audiovisual_cache_common.h"
#include "game/cosmos/get_corresponding.h"

struct interpolation_settings;

class interpolation_system {
	bool enabled = true;
	void set_interpolation_enabled(const bool);

public:
	struct cache {
		transformr recorded_place_of_birth;
		transformr interpolated_transform;
		decltype(entity_id().raw.version) recorded_version = entity_id().raw.version;
		float rotational_slowdown_multiplier = 1.f;
		float positional_slowdown_multiplier = 1.f;

		bool is_constructed() const {
			return recorded_version != entity_id().raw.version;
		}
	};

	entity_id id_to_integerize;

	void integrate_interpolated_transforms(
		const interpolation_settings&,
		const cosmos&,
		const augs::delta delta, 
		const augs::delta fixed_delta_for_slowdowns
	);

	void update_desired_transforms(const cosmos&);

	template <class E>
	transformr get_interpolated(const E& handle) const {
		auto result = get_corresponding<components::interpolation>(handle).interpolated_transform;

		/*
			Here, we integerize the transform of the viewed entity, (and later possibly of the vehicle that it drives)
			so that the rendered player does not shake when exactly followed by the camera,
			which is also integerized for pixel-perfect rendering.

			Ideally we shouldn't do it here because it introduces more state, but that's about the simplest solution
			that doesn't make a mess out of rendering scripts for this special case.	

			Additionally, if someone does not use interpolation (there should be no need to disable it, really)
			they will still suffer from the problem of shaky controlled player. We don't have time for now to handle it better.
		*/

		const auto id = handle.get_id();

		if (entity_id(id) == id_to_integerize) {
			result.pos.discard_fract();
		}

		return result;
	}

	void reserve_caches_for_entities(const size_t);

	template <class E>
	void set_updated_interpolated_transform(
		const E& subject,
		const transformr updated_value
	) {
		auto& info = get_corresponding<components::interpolation>(subject);
		info.interpolated_transform = updated_value;
	}

	void clear();

	bool is_enabled() const {
		return enabled;
	}
};