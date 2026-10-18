#pragma once
#include "augs/log.h"
#include "augs/ensure_rel.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "augs/misc/convex_partitioned_shape.hpp"
#include "game/enums/filters.h"

#include "game/detail/explosive/like_explosive.h"
#include "game/detail/melee/like_melee.h"
#include "game/detail/melee/thrown_melee_bounce.h"
#include "game/detail/inventory/item_falling.h"
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

/*
	The function of the slot the item is in on a character (or another non-item container),
	also as an attachment of an item there, e.g. a weapon's magazine.
*/

template <class E>
std::optional<slot_function> find_top_level_slot_function(const E& handle) {
	const auto& cosm = handle.get_cosmos();
	auto slot_id = handle.get_current_slot().get_id();

	while (const auto slot = cosm[slot_id]) {
		const auto container = slot.get_container();

		if (container.template find<components::item>() == nullptr) {
			return slot.get_type();
		}

		slot_id = container.get_current_slot().get_id();
	}

	return std::nullopt;
}

/*
	Whether the item hangs on a character's back, also as an attachment of a weapon there.
*/

template <class E>
bool is_hung_on_back(const E& handle) {
	const auto top = ::find_top_level_slot_function(handle);
	return top.has_value() && ::is_back_weapon_slot(*top);
}

template <class E>
const components::sentience* find_bot_owner_sentience(const E& handle) {
	const auto capability = handle.get_owning_transfer_capability();

	if (capability && capability != handle) {
		if (const auto sentience = capability.template find<components::sentience>(); sentience != nullptr && sentience->is_bot) {
			return sentience;
		}
	}

	return nullptr;
}

inline filter_type calc_standard_character_filter(const components::sentience& sentience) {
	return
		sentience.is_conscious() ?
		filters[predefined_filter_type::CHARACTER] :
		filters[predefined_filter_type::DEAD_CHARACTER]
	;
}

/*
	See cosmos_navmesh::calc_bot_obstacle_collider_scale.
	On the standard navmesh (scale 1) bots collide exactly like humans.
*/

template <class E>
real32 calc_bot_obstacle_collider_scale(const E& handle) {
	return handle.get_cosmos().get_common_significant().navmesh.calc_bot_obstacle_collider_scale();
}

template <class E>
bool bot_needs_obstacle_fixtures(const E& handle) {
	return ::calc_bot_obstacle_collider_scale(handle) < 1.f;
}

template <class E>
bool is_bot_character(const E& handle) {
	if (const auto sentience = handle.template find<components::sentience>()) {
		return sentience->is_bot && !sentience->has_exploded;
	}

	return false;
}

/*
	An item in a bot's hands (also an attachment of one, e.g. a magazine) - only those knock against obstacles.
*/

template <class E>
bool is_wielded_by_bot(const E& handle) {
	if (::find_bot_owner_sentience(handle) != nullptr) {
		const auto top = ::find_top_level_slot_function(handle);
		return top == slot_function::PRIMARY_HAND || top == slot_function::SECONDARY_HAND;
	}

	return false;
}

/*
	On a navmesh finer than the standard, bots go through obstacles with their real fixtures
	and collide with them through additional ones:
	- a character through its silhouette scaled down to the navmesh, with no mass;
	- an item in its hands through a sensor copy, which only knocks (see sound_existence_system).
	The real silhouette still takes all hits and collides with other characters.
*/

struct bot_obstacle_fixtures {
	real32 scale = 1.f;
	bool sensor = false;
	filter_type filter;
};

template <class E>
std::optional<bot_obstacle_fixtures> calc_bot_obstacle_fixtures(const E& handle) {
	if (::is_bot_character(handle)) {
		if (const auto scale = ::calc_bot_obstacle_collider_scale(handle); scale < 1.f) {
			const auto standard_filter = ::calc_standard_character_filter(*handle.template find<components::sentience>());
			return bot_obstacle_fixtures { scale, false, ::make_obstacle_only_filter(standard_filter) };
		}

		return std::nullopt;
	}

	if (::is_wielded_by_bot(handle) && ::bot_needs_obstacle_fixtures(handle)) {
		return bot_obstacle_fixtures { 1.f, true, ::make_obstacle_only_filter(filters[predefined_filter_type::CHARACTER_WEAPON]) };
	}

	return std::nullopt;
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
			if (::find_bot_owner_sentience(handle) != nullptr) {
				if (::is_hung_on_back(handle) || ::bot_needs_obstacle_fixtures(handle)) {
					/*
						Weapons hung on a bot's back never snag on obstacles, which the bot navigation
						does not account for. Everything else stays as for a human's: bullets and knives
						fly through them with their effects, they keep their mass.
						is_bot is set before the bot gets any items, so nothing needs reinferring when it changes.

						On a navmesh finer than the standard, nothing a bot carries snags either
						- the items in its hands knock instead, see calc_bot_obstacle_fixtures.
					*/

					return ::make_obstacle_free_filter(filters[predefined_filter_type::CHARACTER_WEAPON]);
				}
			}

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
		const auto standard_filter = ::calc_standard_character_filter(*sentience);

		if (::is_bot_character(handle) && ::bot_needs_obstacle_fixtures(handle)) {
			/*
				See calc_bot_obstacle_fixtures.
				Also when dead, so a corpse is never pushed out through a thin wall.
			*/

			return ::make_obstacle_free_filter(standard_filter);
		}

		return standard_filter;
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
		def.maxTranslation = b2_maxMissileTranslation;
	}
	else {
		def.fixedRotation = physics_def.fixed_rotation;
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
	b2PolygonShape polygon_shape;
	b2CircleShape circle_shape;

	fixdef.userData = handle;

	fixdef.density = handle.calc_density(connection, colliders_data);

	fixdef.friction = colliders_data.friction;
	fixdef.restitution = colliders_data.restitution;
	fixdef.isSensor = colliders_data.sensor;
	fixdef.filter = calc_filters(handle);

	if (const auto sentience = handle.template find<components::sentience>()) {
		if (sentience->has_exploded) {
			/*
				Zero out category/mask so the ghost is completely invisible
				to all physics queries (including melee). The lying corpse
				entity handles physical interactions from now on.
			*/
			fixdef.filter.categoryBits = 0;
			fixdef.filter.maskBits = 0;
		}
	}

	/*
		See thrown_melee_bounce.h.
	*/

	const bool thrown_melee = is_like_thrown_melee(handle);

	if (thrown_melee) {
		fixdef.filter = ::make_thrown_melee_hitting_filter(fixdef.filter);
	}

	cached_connection = connection;

	auto& constructed_fixtures = cache.constructed_fixtures;
	ensure(constructed_fixtures.empty());

	/*
		Scales the shapes of the bot obstacle fixtures - see calc_bot_obstacle_fixtures.
	*/

	real32 obstacle_fixtures_scale = 1.f;

	auto flips = flip_flags();

	if (const auto overridden_flip = handle.calc_flip_flags()) {
		flips = *overridden_flip;
	}

	if (connection.flip_geometry) {
		flips.vertically = !flips.vertically;
	}

	auto calc_ground_scale = [&]() {
		if constexpr(E::template has<components::item>()) {
			return ::calc_ground_body_scale(handle);
		}
		else {
			return 1.f;
		}
	};

	auto from_convex_partition = [&](auto shape) {
		/*
			Read the flips lazily as they might still be toggled
			by the torso stance logic before this is called.
		*/
		const bool flip_order = flips.vertically != flips.horizontally;

		/*
			Mirror the vertices in the shape's local space,
			BEFORE offsetting: connection.shape_offset is already mirrored
			by the attachment offset composition, so flipping afterwards
			would mirror the offset twice.
		*/
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

		shape.offset_vertices(connection.shape_offset);

		auto scale = std::optional<vec2>();

		if (const auto geo = handle.template find<components::overridden_geo>()) {
			auto& s = geo.get();

			if (s.is_enabled) {
				if (const auto sprite = handle.template find<invariants::sprite>()) {
					scale = ::calc_requested_scale(sprite->get_size(), s.value);
				}
			}
		}

		if (const auto ground_scale = calc_ground_scale(); ground_scale != 1.f) {
			scale = scale.value_or(vec2::square(1.f)) * ground_scale;
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

				v *= obstacle_fixtures_scale;
			}

			polygon_shape.Set(b2verts.data(), static_cast<int32>(b2verts.size()));

			fixdef.shape = &polygon_shape;
			b2Fixture* const new_fix = owner_b2Body.CreateFixture(&fixdef);

			ensure_less(constructed_fixtures.size(), static_cast<std::size_t>(std::numeric_limits<short>::max()));
			new_fix->index_in_component = static_cast<short>(constructed_fixtures.size());

			constructed_fixtures.emplace_back(new_fix);
		};

		shape.for_each_convex(add_convex);
	};

	auto from_circle_shape = [&](const real32 radius) {
		circle_shape.m_radius = si.get_meters(radius);
		circle_shape.m_p += b2Vec2(connection.shape_offset.pos);

		fixdef.shape = &circle_shape;
		b2Fixture* const new_fix = owner_b2Body.CreateFixture(&fixdef);

		new_fix->index_in_component = static_cast<short>(constructed_fixtures.size());
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
			if (total_rotation != 0.0f) {
				v.rotate(total_rotation);
			}

			v += off_meters;
			v *= obstacle_fixtures_scale;
		}

		polygon_shape.Set(verts.data(), verts.size());

		/*
			Set m_is_aabb for navmesh optimization:
			the shape is axis-aligned if there is no rotation.
		*/
		polygon_shape.m_is_aabb = (total_rotation == 0.0f);

		fixdef.shape = &polygon_shape;
		b2Fixture* const new_fix = owner_b2Body.CreateFixture(&fixdef);

		new_fix->index_in_component = static_cast<short>(constructed_fixtures.size());

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

		auto build_from_image = [&]() {
			if (const auto& shape = offsets.non_standard_shape; !shape.empty()) {
				from_convex_partition(shape);
			}
			else {
				const image_shape_type* non_standard_shape = nullptr;

				/*
					The raw, unflipped body_rotation of the stance frame.
					The mirroring in from_convex_partition negates it implicitly,
					as flip(rotate(r, v)) == rotate(-r, flip(v)).
				*/
				real32 stance_shape_rotation = 0.f;

				const auto additional_rotation = [&]() {
					const auto& typed_self = handle;

					if (const auto torso = typed_self.template find<invariants::torso>()) {
						const bool consider_reloading = true;
						const auto stance = torso->calc_stance(typed_self, typed_self.get_wielded_items(), consider_reloading);

						const auto& logicals = cosm.get_logical_assets();

						if (const auto anim = logicals.find(stance.carry)) {
							if (anim->frames.size() > 0) {
								auto& offsets = logicals.get_offsets(anim->frames[0].image_id);

								if (!offsets.non_standard_shape.empty()) {
									non_standard_shape = &offsets.non_standard_shape;
									stance_shape_rotation = offsets.torso.body_rotation;
								}

								auto considered_offsets = offsets.torso;

								if (typed_self.only_secondary_holds_item()) {
									considered_offsets.flip_vertically();

									/*
										Mirror the custom collider shape as well,
										as the torso sprite is drawn flipped.
									*/
									flips.vertically = !flips.vertically;
								}

								considered_offsets.apply_body_rotation();

								const auto stance_rotation = considered_offsets.back.rotation;
								return stance_rotation;
							}
						}
					}

					return 0.f;
				}();

				if (non_standard_shape) {
					if (stance_shape_rotation != 0.f) {
						auto rotated_shape = *non_standard_shape;

						for (auto& v : rotated_shape.source_polygon) {
							v.rotate(stance_shape_rotation);
						}

						from_convex_partition(rotated_shape);
					}
					else {
						from_convex_partition(*non_standard_shape);
					}
				}
				else {
					from_box_shape(handle.get_logical_size() * calc_ground_scale(), additional_rotation);
				}
			}
		};

		/*
			The stance may toggle the flips while building - restore them for the second build.
		*/
		const auto flips_before_build = flips;

		build_from_image();

		if (const auto obstacle_fixtures = ::calc_bot_obstacle_fixtures(handle)) {
			flips = flips_before_build;

			fixdef.filter = obstacle_fixtures->filter;
			fixdef.isSensor = obstacle_fixtures->sensor;
			fixdef.density = 0.f;
			obstacle_fixtures_scale = obstacle_fixtures->scale;

			build_from_image();

			cache.has_bot_obstacle_fixtures = true;
		}

		if (thrown_melee) {
			/*
				Created last, so that it is centered at the center of mass of the real shape,
				and of zero density, so that it does not move it.
			*/

			b2CircleShape bounce_shape;
			bounce_shape.m_radius = si.get_meters(::calc_thrown_melee_bounce_radius(handle));
			bounce_shape.m_p = owner_b2Body.GetLocalCenter();

			auto bounce_def = fixdef;
			bounce_def.shape = &bounce_shape;
			bounce_def.density = 0.f;
			bounce_def.friction = THROWN_MELEE_BOUNCE_FRICTION;
			bounce_def.restitution = THROWN_MELEE_BOUNCE_RESTITUTION;
			bounce_def.filter = ::make_thrown_melee_bouncing_filter(calc_filters(handle));

			b2Fixture* const new_fix = owner_b2Body.CreateFixture(&bounce_def);

			new_fix->index_in_component = static_cast<short>(constructed_fixtures.size());
			constructed_fixtures.emplace_back(new_fix);
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

		/*
			Updating the properties would give the bot obstacle fixtures those of the real ones.
		*/
		if (cache.has_bot_obstacle_fixtures || ::calc_bot_obstacle_fixtures(handle).has_value()) {
			only_update_properties = false;
		}

		if constexpr(E::template has<invariants::melee>()) {
			/*
				Thrown melee weapons have colliders of their own - see thrown_melee_bounce.h -
				so they are rebuilt whenever they are or just were thrown.
			*/

			const bool has_bounce_circle = std::any_of(
				cache.constructed_fixtures.begin(),
				cache.constructed_fixtures.end(),
				[](const auto& f) {
					return f.get()->GetType() == b2Shape::e_circle;
				}
			);

			if (has_bounce_circle || is_like_thrown_melee(handle)) {
				only_update_properties = false;
			}
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

			auto chosen_filters = calc_filters(handle);
			auto chosen_sensor = colliders_data.sensor;

			if (const auto sentience = handle.template find<components::sentience>()) {
				if (sentience->has_exploded) {
					chosen_filters.categoryBits = 0;
					chosen_filters.maskBits = 0;
				}
			}

			const bool rebuild_filters = compared.GetFilterData() != chosen_filters;

			for (auto& f : cache.constructed_fixtures) {
				f.get()->SetRestitution(colliders_data.restitution);
				f.get()->SetFriction(colliders_data.friction);
				f.get()->SetSensor(chosen_sensor);

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
