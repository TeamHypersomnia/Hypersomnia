#pragma once
#include "augs/log.h"
#include "augs/ensure_rel.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "augs/misc/convex_partitioned_shape.hpp"
#include "game/enums/filters.h"

#include "game/detail/explosive/like_explosive.h"
#include "game/detail/melee/like_melee.h"
#include "game/detail/physics/infer_damping.hpp"
#include "game/detail/entity_handle_mixins/calc_connection.hpp"

inline auto to_b2Body_type(const rigid_body_type t) {
	switch (t) {
		case rigid_body_type::STATIC: 
		case rigid_body_type::ALWAYS_STATIC: 
			return b2BodyType::b2_staticBody;
		case rigid_body_type::KINEMATIC: 
			return b2BodyType::b2_kinematicBody;
		case rigid_body_type::DYNAMIC: 
			return b2BodyType::b2_dynamicBody;

		default: 
			return b2BodyType::b2_staticBody;
	}
};

template <class E>
auto calc_needs_analytic_physics(const E& handle) {
	const auto& rigid_body = handle.template get<invariants::rigid_body>();

	if (is_like_thrown_explosive(handle)) {
		return true;
	}

	if (is_like_thrown_melee(handle)) {
		return true;
	}

	return rigid_body.bullet;
}

template <class E>
auto calc_body_type(const E& handle) {
	const auto& physics_def = handle.template get<invariants::rigid_body>();

	const auto type = 
		is_like_planted_or_defused_bomb(handle) 
		? rigid_body_type::STATIC 
		: physics_def.body_type
	;

	return to_b2Body_type(type);
}

template <class E>
auto calc_filters(const E& handle) {
	const auto& colliders_data = handle.template get<invariants::fixtures>();

	if (const auto missile = handle.template find<components::missile>()) {
		if (missile->during_penetration) {
			return filters[predefined_filter_type::PENETRATING_BULLET];
		}
	}

	if (is_like_planted_or_defused_bomb(handle)) {
		return filters[predefined_filter_type::PLANTED_EXPLOSIVE];
	}

	if (is_like_thrown_melee(handle)) {
		return filters[predefined_filter_type::FLYING_MELEE];
	}

	if (is_like_thrown_explosive(handle)) {
		return filters[predefined_filter_type::FLYING_EXPLOSIVE];
	}

	{
		const auto capability = handle.get_owning_transfer_capability();

		if (capability && capability != handle) {
			/*
				Note that a melee in action will still collide with shoot/throw-through surfaces,
				because melee_system uses predefined_queries::melee_query()
				to filter for potential damage targets.
				This is a good thing because we might still want to hit low lying objects.
			*/

			return filters[predefined_filter_type::CHARACTER_WEAPON];
		}
	}

	if (const auto sentience = handle.template find<components::sentience>()) {
		if (sentience->has_exploded) {
			return filter_type::nothing();
		}

		if (!sentience->is_conscious()) {
			return filters[predefined_filter_type::DEAD_CHARACTER];
		}

		return filters[predefined_filter_type::CHARACTER];
	}

	if (const auto portal = handle.template find<components::portal>()) {
		return portal->custom_filter;
	}

	if (const auto rigid = handle.template find<components::rigid_body>()) {
		if (rigid.get_special().inside_portal.is_set()) {
			return filter_type();
		}
	}

	return colliders_data.filter;
}

template <class E>
void physics_world_cache::specific_infer_rigid_body_existence(const E& typed_handle) {
	if (const auto cache = find_rigid_body_cache(typed_handle)) {
		return;
	}

	specific_infer_rigid_body_from_scratch(typed_handle);
}

template <class E>
void physics_world_cache::specific_infer_cache_for(const E& typed_handle) {
	if constexpr(E::template has<invariants::rigid_body>()) {
		specific_infer_rigid_body(typed_handle);
	}

	if constexpr(E::template has<invariants::fixtures>()) {
		specific_infer_colliders(typed_handle);
	}

#if TODO_JOINTS
	if constexpr() {
		specific_infer_joint(typed_handle);
	}
#endif
}

template <class E>
void physics_world_cache::specific_infer_rigid_body_from_scratch(const E& handle) {
	auto& cache = get_corresponding<rigid_body_cache>(handle);

	const auto& physics_def = handle.template get<invariants::rigid_body>();

	cache.clear(handle.get_cosmos(), *this);

	const auto rigid_body = handle.template get<components::rigid_body>();
	const auto& physics_data = rigid_body.get_raw_component();

	b2BodyDef def;
	def.type = calc_body_type(handle);

	def.userData = unversioned_entity_id(handle);

	def.bullet = calc_needs_analytic_physics(handle);
	def.allowSleep = physics_def.allow_sleep;

	const auto damping = ::calc_damping_mults(handle, physics_def);

	def.angularDamping = damping.angular;
	def.linearDamping = damping.linear;

	def.transform = physics_data.physics_transforms.m_xf;
	def.sweep = physics_data.physics_transforms.m_sweep;

	def.linearVelocity = b2Vec2(physics_data.velocity);
	def.angularVelocity = physics_data.angular_velocity;

	if (handle.template has<components::missile>()) {
		def.fixedRotation = true;
	}

	def.active = true;

	cache.body = b2world->CreateBody(&def);

	cache.body->SetAngledDampingEnabled(::calc_angled_damping_enabled(handle));
	cache.body->SetLinearDampingVec(b2Vec2(damping.linear_axis_aligned));

	/*
		Warning: given a working setup of collider and rigid body caches,
		if rigid body now needs completely reinferring, the collider caches will be destroyed.
		Rigid body should never be re-inferred ON ITS OWN when there is possibility 
		that it will need to be rebuilt from scratch. 

		All colliders caches, before their own inference,
		manually infer the existence of the rigid body,
		and the b2Body cannot henceforth disappear from existence.
			Note: if we know it is going to be rebuilt (e.g. due to a change from dynamic to static) 
			we must manually call the fixtures updater.

		Thus the rigid body, on its own inference, does not have to inform all fixtures
		about that it has just come into existence.
	*/
}

template <class E>
void physics_world_cache::specific_infer_rigid_body(const E& handle) {
	auto& cache = get_corresponding<rigid_body_cache>(handle);

	if (cache.is_constructed()) {
		/* The cache already existed. */
		auto& body = *cache.body;

		bool only_update_properties = true;

		if (calc_body_type(handle) != body.GetType()) {
			only_update_properties = false;
		}

		if (only_update_properties) {
			/* 
				Invariant/component guaranteed to exist because it must have once been created from an existing def,
				and changing type content implies reinference of the entire cosm.
			*/
	
			const auto& def = handle.template get<invariants::rigid_body>();
			const auto rigid_body = handle.template get<components::rigid_body>();
			const auto& data = rigid_body.get_raw_component();
	
			/* 
				Currently, nothing that can change inside the component could possibly trigger the need to rebuild the body.
				This may change once we want to delete bodies without fixtures.
			*/
	
			/* These have no side-effects */
			::infer_damping(handle, body);
			body.SetBullet(calc_needs_analytic_physics(handle));
	
			if (handle.template has<components::missile>()) {
				body.SetFixedRotation(true);
			}

			/* These have side-effects, thus we guard */
			if (body.IsSleepingAllowed() != def.allow_sleep) {
				body.SetSleepingAllowed(def.allow_sleep);
			}
	
			if (body.GetLinearVelocity() != b2Vec2(data.velocity)) {
				body.SetLinearVelocity(b2Vec2(data.velocity));
			}
	
			if (body.GetAngularVelocity() != data.angular_velocity) {
				body.SetAngularVelocity(data.angular_velocity);
			}
	
			if (!(body.m_xf == data.physics_transforms.m_xf)) {
				body.m_xf = data.physics_transforms.m_xf;
				body.m_sweep = data.physics_transforms.m_sweep;
	
				b2BroadPhase* broadPhase = &body.m_world->m_contactManager.m_broadPhase;
	
				for (b2Fixture* f = body.m_fixtureList; f; f = f->m_next)
				{
					f->Synchronize(broadPhase, body.m_xf, body.m_xf);
				}
			}
	
			return;
		}
	}

	specific_infer_rigid_body_from_scratch(handle);
}

inline vec2 calc_requested_scale(const vec2 sprite_size, vec2 overri_size) {
	overri_size.x = std::max(1.f, overri_size.x);
	overri_size.y = std::max(1.f, overri_size.y);

	auto scale = vec2();

	if (sprite_size.x == 0) {
		scale.x = 1;
	}
	else {
		scale.x = overri_size.x / sprite_size.x;
	}

	if (sprite_size.y == 0) {
		scale.y = 1;
	}
	else {
		scale.y = overri_size.y / sprite_size.y;
	}

	return scale;
}

template <class E>
void physics_world_cache::specific_infer_colliders_from_scratch(const E& handle, const colliders_connection& connection) {
	auto& cosm = handle.get_cosmos();

	auto& cache = get_corresponding<colliders_cache>(handle);
	cache.clear(*this);

	auto& cached_connection = cache.connection;
	cached_connection.owner = {};

	const auto new_owner = cosm[connection.owner];

	if (new_owner.dead()) {
		cache.clear(*this);
		return;
	}

	const auto body_cache = new_owner.template dispatch_on_having_all_ret<invariants::rigid_body>([&](const auto& typed_new_owner) -> rigid_body_cache* {
		if constexpr(is_nullopt_v<decltype(typed_new_owner)>) {
			return nullptr;
		}
		else {
			specific_infer_rigid_body_existence(typed_new_owner);
			return std::addressof(get_corresponding<rigid_body_cache>(typed_new_owner));
		}
	});

	if (!body_cache || !body_cache->is_constructed()) {
		/* 
			No body to attach to. 
			Might happen if we once implement it that the logic deactivates bodies for some reason. 
			Or, if collider owner calculation returns incorrectly an entity without rigid body component.
		*/

		return;
	}

	const auto si = handle.get_cosmos().get_si();
	auto& owner_b2Body = *body_cache->body.get();

	const auto& colliders_data = handle.template get<invariants::fixtures>(); 

	b2FixtureDef fixdef;

	fixdef.userData = handle;

	fixdef.density = handle.calc_density(connection, colliders_data);

	fixdef.friction = colliders_data.friction;
	fixdef.restitution = colliders_data.restitution;
	fixdef.isSensor = colliders_data.sensor;
	fixdef.filter = calc_filters(handle);

	if (const auto sentience = handle.template find<components::sentience>()) {
		if (sentience->has_exploded) {
			fixdef.isSensor = true;
		}
	}

	cached_connection = connection;

	auto& constructed_fixtures = cache.constructed_fixtures;
	ensure(constructed_fixtures.empty());

	auto flips = flip_flags();

	if (const auto overridden_flip = handle.calc_flip_flags()) {
		flips = *overridden_flip;
	}

	if (connection.flip_geometry) {
		flips.vertically = !flips.vertically;
	}

	const bool flip_order = flips.vertically != flips.horizontally;

	auto from_convex_partition = [&](auto shape) {
		shape.offset_vertices(connection.shape_offset);

		if (flips.horizontally) {
			for (auto& v : shape.source_polygon) {
				v.neg_x();
			}
		}

		if (flips.vertically) {
			for (auto& v : shape.source_polygon) {
				v.neg_y();
			}
		}

		unsigned ci = 0;

		auto scale = std::optional<vec2>();

		if (const auto geo = handle.template find<components::overridden_geo>()) {
			auto& s = geo.get();

			if (s.is_enabled) {
				if (const auto sprite = handle.template find<invariants::sprite>()) {
					scale = ::calc_requested_scale(sprite->get_size(), s.value);
				}
			}
		}

		auto add_convex = [&](const auto& convex) {
			if (convex.size() > b2_maxPolygonVertices) {
				return;
			}

			augs::constant_size_vector<b2Vec2, b2_maxPolygonVertices> b2verts(
				convex.begin(), 
				convex.end()
			);

			if (flip_order) {
				reverse_range(b2verts);
			}

			for (auto& v : b2verts) {
				v = si.get_meters(v);

				if (scale) {
					v.x *= scale->x;
					v.y *= scale->y;
				}
			}

			b2PolygonShape ps;
			ps.Set(b2verts.data(), static_cast<int32>(b2verts.size()));

			fixdef.shape = &ps;
			b2Fixture* const new_fix = owner_b2Body.CreateFixture(&fixdef);

			ensure_less(static_cast<short>(ci), std::numeric_limits<short>::max());
			new_fix->index_in_component = static_cast<short>(ci++);

			constructed_fixtures.emplace_back(new_fix);
		};

		shape.for_each_convex(add_convex);
	};

	auto from_circle_shape = [&](const real32 radius) {
		b2CircleShape shape;
		shape.m_radius = si.get_meters(radius);
		shape.m_p += b2Vec2(connection.shape_offset.pos);

		fixdef.shape = &shape;
		b2Fixture* const new_fix = owner_b2Body.CreateFixture(&fixdef);

		new_fix->index_in_component = 0u;
		constructed_fixtures.emplace_back(new_fix);
	};

	auto from_box_shape = [&](vec2 size, const real32 additional_rotation) {
		size.x = std::max(1.f, size.x);
		size.y = std::max(1.f, size.y);

		size = si.get_meters(size);

		const auto hx = size.x / 2;
		const auto hy = size.y / 2;

		std::array<vec2, 4> verts;

		verts[0].set(-hx, -hy);
		verts[1].set( hx, -hy);
		verts[2].set( hx,  hy);
		verts[3].set(-hx,  hy);

		const auto off_meters = si.get_meters(connection.shape_offset.pos);
		const auto total_rotation = additional_rotation + connection.shape_offset.rotation;

		for (auto& v : verts) {
			v.rotate(total_rotation);
			v += off_meters;
		}

		b2PolygonShape ps;
		ps.Set(verts.data(), verts.size());

		fixdef.shape = &ps;
		b2Fixture* const new_fix = owner_b2Body.CreateFixture(&fixdef);

		new_fix->index_in_component = 0;

		constructed_fixtures.emplace_back(new_fix);
	};

	auto from_edge_shape = [&](real32 segment_length) {
		from_box_shape(vec2(segment_length, 0.01f), 0.0f);
	};

	if (is_like_thrown_explosive(handle)) {
		if (const auto fuse = handle.template find<invariants::hand_fuse>()) {
			from_circle_shape(fuse->circle_shape_radius_when_released);
		}

		return;
	}

	if (const auto missile = handle.template find<invariants::missile>()) {
		if (!missile->use_polygon_shape) {
			from_edge_shape(handle.get_logical_size().x);
			return;
		}
	}

	if (const auto expl_body = handle.template find<invariants::cascade_explosion>()) {
		from_circle_shape(expl_body->circle_collider_radius);
		return;
	}

	const auto& logicals = cosm.get_logical_assets();

	if (const auto image_id = handle.get_image_id(); image_id.is_set()) {
		const auto& offsets = logicals.get_offsets(image_id);

		if (const auto& shape = offsets.non_standard_shape; !shape.empty()) {
			from_convex_partition(shape);
		}
		else {
			const auto additional_rotation = [&]() {
				const auto& typed_self = handle;

				if (const auto torso = typed_self.template find<invariants::torso>()) {
					const bool consider_reloading = true;
					const auto stance = torso->calc_stance(typed_self, typed_self.get_wielded_items(), consider_reloading);

					const auto& logicals = cosm.get_logical_assets();

					if (const auto anim = logicals.find(stance.carry)) {
						if (anim->frames.size() > 0) {
							auto considered_offsets = logicals.get_offsets(anim->frames[0].image_id).torso;

							if (typed_self.only_secondary_holds_item()) {
								considered_offsets.flip_vertically();
							}

							const auto stance_rotation = considered_offsets.back.rotation;
							return stance_rotation;
						}
					}
				}

				return 0.f;
			}();

			from_box_shape(handle.get_logical_size(), additional_rotation);
		}

		return;
	}

	if (handle.template has<invariants::area_marker>()) {
		if (const auto marker = handle.template find<components::marker>()) {
			if (const auto geo = handle.template find<components::overridden_geo>()) {
				auto& s = geo.get();

				if (s.is_enabled) {
					if (marker->shape == marker_shape_type::BOX) {
						from_box_shape(s.value, 0.0f);
					}
					else {
						const auto radius = s.value.smaller_side() / 2;
						from_circle_shape(radius);
					}

					return;
				}
			}
		}
	}

	LOG_NVPS(handle);
	ensure(false && "Fixtures requested with no shape defined! Maybe the entity has no image specified?");
}


template <class E>
void physics_world_cache::specific_infer_colliders(const E& handle) {
	std::optional<colliders_connection> calculated_connection;

	auto get_calculated_connection = [&](){
		if (calculated_connection == std::nullopt) {
			calculated_connection = handle.calc_colliders_connection(); 
		}

		return *calculated_connection;
	};

	auto& cache = get_corresponding<colliders_cache>(handle);

	if (cache.is_constructed()) {
		/* Cache already existed. */
		bool only_update_properties = true;

		const auto& cached_connection = cache.connection;
		
		if (get_calculated_connection() != cached_connection) {
			only_update_properties = false;
		}

		if (cache.constructed_fixtures.empty()) {
			only_update_properties = false;
		}

		if (only_update_properties) {
			auto& compared = *cache.constructed_fixtures[0].get();
			const auto& colliders_data = handle.template get<invariants::fixtures>();

			if (const auto new_density = handle.calc_density(
					get_calculated_connection(), 
					colliders_data
				);

				compared.GetDensity() != new_density
			) {
				for (auto& f : cache.constructed_fixtures) {
					f.get()->SetDensity(new_density);
				}

				compared.GetBody()->ResetMassData();
			}

			const auto chosen_filters = calc_filters(handle);
			const bool rebuild_filters = compared.GetFilterData() != chosen_filters;

			for (auto& f : cache.constructed_fixtures) {
				f.get()->SetRestitution(colliders_data.restitution);
				f.get()->SetFriction(colliders_data.friction);
				f.get()->SetSensor(colliders_data.sensor);

				if (rebuild_filters) {
					f.get()->SetFilterData(chosen_filters);
				}
			}
			
			return;
		}
	}

	specific_infer_colliders_from_scratch(handle, get_calculated_connection());
}

#if TODO_JOINTS
template <class E>
void physics_world_cache::specific_infer_joint(const E& /* handle */) {
	auto& cosm = handle.get_cosmos();

	if (const auto motor_joint = handle.find<components::motor_joint>();

		motor_joint != nullptr
		&& rigid_body_cache_exists_for(cosm[motor_joint.get_target_bodies()[0]])
		&& rigid_body_cache_exists_for(cosm[motor_joint.get_target_bodies()[1]])
	) {
		const components::motor_joint joint_data = motor_joint.get_raw_component();

		const auto si = handle.get_cosmos().get_si();
		auto cache = find_joint_cache(handle);
		
		ensure_eq(nullptr, cache->joint);

		b2MotorJointDef def;
		def.userData = handle.get_id();
		def.bodyA = find_rigid_body_cache(cosm[joint_data.target_bodies[0]]).body;
		def.bodyB = find_rigid_body_cache(cosm[joint_data.target_bodies[1]]).body;
		def.collideConnected = joint_data.collide_connected;
		def.linearOffset = b2Vec2(si.get_meters(joint_data.linear_offset));
		def.angularOffset = DEG_TO_RAD<float> * joint_data.angular_offset;
		def.maxForce = si.get_meters(joint_data.max_force);
		def.maxTorque = joint_data.max_torque;
		def.correctionFactor = joint_data.correction_factor;

		cache->joint = b2world->CreateJoint(&def);
	}
}
#endif
