#pragma once
#include "game/entity_id.h"
#include "math/vec2.h"
#include "math/rects.h"
#include <vector>
#include "transform_component.h"
#include "game/detail/physics_engine_reflected_state.h"

namespace components {
	struct fixtures : public colliders_white_box {
	private:
		friend struct components::physics;
		friend class ::physics_system;

		colliders_black_box black;
		colliders_black_box_detail black_detail;

		fixtures& operator=(const fixtures&);
		fixtures(const fixtures&);
		fixtures(fixtures&&) = delete;

		bool syncable_black_box_exists() const;
		bool should_fixtures_exist_now() const;

		void destroy_fixtures();
		void build_fixtures();

		void rebuild_density(size_t);

	public:
		typedef colliders_black_box::offset_type offset_type;

		fixtures(const colliders_definition&);
		void initialize_from_definition(const colliders_definition& = colliders_definition());
		colliders_definition get_definition() const;

		void set_offset(offset_type, components::transform);
		components::transform get_offset(offset_type) const;
		components::transform get_total_offset() const;

		void set_activated(bool);
		bool is_activated() const;

		void set_density(float, size_t = 0);
		void set_density_multiplier(float, size_t = 0);
		void set_friction(float, size_t = 0);
		void set_restitution(float, size_t = 0);

		float get_density_multiplier(size_t = 0) const;
		float get_friction(size_t = 0) const;
		float get_restitution(size_t = 0) const;
		float get_density(size_t = 0) const;

		void set_owner_body(entity_id);
		entity_id get_owner_body() const;

		vec2 get_aabb_size() const;
		augs::rects::ltrb<float> get_aabb_rect() const;

		size_t get_num_colliders() const;

		b2Body* get_body() const;
		entity_id get_body_entity() const;
	};
}