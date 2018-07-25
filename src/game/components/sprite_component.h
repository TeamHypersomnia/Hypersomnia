#pragma once
#include "augs/pad_bytes.h"
#include "augs/drawing/flip.h"
#include "augs/drawing/sprite.h"

#include "game/components/sprite_component_declaration.h"
#include "game/cosmos/component_synchronizer.h"

namespace components {
	struct sprite {
		// GEN INTROSPECTOR struct components::sprite
		flip_flags flip;
		bool disable_neon_map = false;
		pad_bytes<1> pad;
		float effect_offset_secs = 0.f;
		rgba colorize = white;
		rgba colorize_neon = white;
		// END GEN INTROSPECTOR
	};

	struct overridden_size {
		static constexpr bool is_synchronized = true;

		// GEN INTROSPECTOR struct components::overridden_size
		augs::maybe<vec2i> size;
		// END GEN INTROSPECTOR
	};
};

template <class E>
class component_synchronizer<E, components::overridden_size> : public synchronizer_base<E, components::overridden_size> {
protected:
	using base = synchronizer_base<E, components::overridden_size>;
	using base::operator->;
	using base::handle;

	void infer_caches() const {
		handle.get_cosmos().get_solvable_inferred({}).tree_of_npo.infer_cache_for(handle);
		handle.infer_colliders_from_scratch();
	}

public:
	using base::base;

	const auto& get() const {
		return this->component->size;
	}

	void set(const vec2 new_size) const {
		this->component->size.emplace(new_size);
		infer_caches();
	}

	void reset() const {
		this->component->size.is_enabled = false;
		infer_caches();
	}
};