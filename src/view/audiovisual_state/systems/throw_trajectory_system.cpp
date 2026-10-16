#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <optional>
#include <unordered_map>
#include "3rdparty/Box2D/Box2D.h"
#include "augs/algorithm/sample_along_path.h"
#include "augs/algorithm/simplify_polyline.h"
#include "augs/math/damped_motion.h"
#include "augs/math/arithmetical.h"
#include "augs/math/math.h"
#include "augs/math/rects.h"
#include "augs/misc/convex_partitioned_shape.hpp"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/components/fixtures_component.h"
#include "game/components/item_component.h"
#include "game/components/melee_component.h"
#include "game/components/missile_component.h"
#include "game/components/portal_component.h"
#include "game/components/sentience_component.h"
#include "game/components/sprite_component.h"
#include "game/enums/filters.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/detail/entity_scripts.h"
#include "game/detail/explosive/explosive_throw.h"
#include "game/detail/inventory/item_falling.h"
#include "game/detail/melee/thrown_melee_bounce.h"
#include "game/detail/physics/physics_queries.h"
#include "view/audiovisual_state/systems/throw_trajectory_system.h"
#include "view/rendering_scripts/aimed_with.h"

/*
	Thrown melee weapons are simulated until they stop hurting, but at most this long.
*/

static constexpr real32 MAX_MELEE_FLIGHT_SECS = 5.f;

/*
	Below this speed in pixels per second, the thrown item is considered at rest.
*/

static constexpr real32 REST_SPEED = 2.f;

/*
	How much farther than the farthest the thrown item could fly to look for the colliders it could hit.
*/

static constexpr real32 OBSTACLES_QUERY_MARGIN = 200.f;

/*
	A turn of the flight in a single step by at least this many degrees is a bounce,
	whose corner is put where the flight before and after it meet - see simulate_flight.
	Gentler turns - only shallow bounces, or curving by axis-aligned damping - are left to the simplification of the laser.
*/

static constexpr real32 BOUNCE_MIN_DEGREES = 10.f;

/*
	How far in pixels the simplified laser may stray from the simulated flight.
*/

static constexpr real32 LASER_TOLERANCE = 0.5f;

namespace {
	/*
		Lets the floor hit logic of item_falling.h push and spin a body of the simulated world.
	*/

	struct simulated_body {
		b2Body& body;
		const si_scaling si;

		real32 get_degree_velocity() const {
			return body.GetAngularVelocity() * RAD_TO_DEG<real32>;
		}

		void set_angular_velocity(const real32 degrees) const {
			body.SetAngularVelocity(degrees * DEG_TO_RAD<real32>);
		}

		vec2 get_velocity() const {
			const auto v = body.GetLinearVelocity();
			return si.get_pixels(vec2(v.x, v.y));
		}

		void set_velocity(const vec2 pixels) const {
			const auto v = si.get_meters(pixels);
			body.SetLinearVelocity(b2Vec2(v.x, v.y));
		}
	};

	/*
		The modified Box2D takes the whole transform and sweep of a new body.
	*/

	void set_body_def_transform(b2BodyDef& def, const b2Vec2 pos, const real32 radians) {
		def.transform.Set(pos, radians);

		def.sweep.localCenter.SetZero();
		def.sweep.c0 = pos;
		def.sweep.c = pos;
		def.sweep.a0 = radians;
		def.sweep.a = radians;
		def.sweep.alpha0 = 0.f;
	}

	/*
		Explosives fly as a circle. Melee weapons hit with polygons and bounce with a circle
		centered at their center of mass - see thrown_melee_bounce.h.
	*/

	struct thrown_shapes {
		std::vector<b2PolygonShape> polygons;
		std::optional<b2CircleShape> circle;
		real32 bounce_radius = 0.f;
	};

	/*
		Finds how far along a ray the nearest stopper - a sensor, see classify_collider -
		and the nearest obstacle are, ignoring the thrown body.
	*/

	struct stopper_ray_callback : b2RayCastCallback {
		const b2Body* thrown_body = nullptr;
		float32 stopper_fraction = 2.f;
		float32 obstacle_fraction = 2.f;
		unversioned_entity_id stopper;

		float32 ReportFixture(b2Fixture* const fixture, const b2Vec2&, const b2Vec2&, const float32 fraction) override {
			if (fixture->GetBody() == thrown_body) {
				return -1.f;
			}

			if (fixture->IsSensor()) {
				if (fraction < stopper_fraction) {
					stopper_fraction = fraction;
					stopper = fixture->GetUserData();
				}
			}
			else {
				obstacle_fraction = std::min(obstacle_fraction, fraction);
			}

			return 1.f;
		}
	};

	/*
		The laser path follows the positions of the path, with the corners of the bounces put between them.
	*/

	struct simulated_flight {
		std::vector<transformr> path;
		std::optional<std::size_t> first_contact_index;
		unversioned_entity_id stopped_by;

		std::vector<vec2> laser_path;
		std::optional<std::size_t> laser_first_contact_index;
	};

	real32 calc_ground_size_mult(const const_entity_handle item) {
		if (const auto item_def = item.find<invariants::item>()) {
			return item_def->ground_scale;
		}

		return 1.f;
	}

	/*
		The colliders of the item once thrown - see physics_world_cache::specific_infer_colliders_from_scratch.
	*/

	thrown_shapes make_thrown_shapes(const const_entity_handle item, const bool explosive) {
		const auto& cosm = item.get_cosmos();
		const auto si = cosm.get_si();

		auto result = thrown_shapes();

		if (explosive) {
			const auto radius = item.get<invariants::hand_fuse>().circle_shape_radius_when_released;

			b2CircleShape circle;
			circle.m_radius = si.get_meters(radius);

			result.circle = circle;

			return result;
		}

		result.bounce_radius = ::calc_thrown_melee_bounce_radius(item);

		const auto scale = ::calc_thrown_item_body_scale(item);

		auto add_polygon = [&](const auto& vertices) {
			if (vertices.size() < 3 || vertices.size() > b2_maxPolygonVertices) {
				return;
			}

			augs::constant_size_vector<b2Vec2, b2_maxPolygonVertices> b2verts;

			for (const auto& v : vertices) {
				const auto scaled = si.get_meters(vec2(v) * scale);
				b2verts.push_back(b2Vec2(scaled.x, scaled.y));
			}

			b2PolygonShape polygon;
			polygon.Set(b2verts.data(), static_cast<int32>(b2verts.size()));
			result.polygons.push_back(polygon);
		};

		if (const auto image_id = item.get_image_id(); image_id.is_set()) {
			const auto& shape = cosm.get_logical_assets().get_offsets(image_id).non_standard_shape;

			if (!shape.empty()) {
				shape.for_each_convex(add_polygon);
			}
		}

		if (result.polygons.empty()) {
			const auto size = item.get_logical_size();
			const auto h = vec2(std::max(1.f, size.x), std::max(1.f, size.y)) / 2;

			const std::array<vec2, 4> verts = {
				vec2(-h.x, -h.y),
				vec2(h.x, -h.y),
				vec2(h.x, h.y),
				vec2(-h.x, h.y)
			};

			add_polygon(verts);
		}

		return result;
	}

	/*
		The characters a thrown melee weapon would stop at, as they stand now -
		all the conscious ones but the thrower and those protected after spawning.
	*/

	bool stops_thrown_melee(const const_entity_handle thrower, const const_entity_handle target) {
		if (target == thrower) {
			return false;
		}

		const auto sentience = target.find<components::sentience>();

		if (sentience == nullptr || !sentience->is_conscious()) {
			return false;
		}

		return !sentience->spawn_protection_cooldown.lasts(target.get_cosmos().get_clock());
	}

	enum class copied_collider_type {
		NONE,
		STATIC_OBSTACLE,
		DYNAMIC_OBSTACLE,
		STOPPER
	};

	/*
		What a collider the thrown item collides with is copied as:
		- static obstacles like walls,
		- dynamic obstacles like crates - but not characters, nor items held, lying or flying, nor missiles,
		- stoppers, as sensors, where the throw is not followed further, only shown as blocked from there on:
			- portals,
			- if melee_thrower is alive, the characters it would stop at - see stops_thrown_melee.
	*/

	copied_collider_type classify_collider(
		const cosmos& cosm,
		const b2Fixture& fixture,
		const const_entity_handle melee_thrower
	) {
		const auto source_body = fixture.GetBody();
		const auto owner = cosm[fixture.GetUserData()];

		/*
			Portals might not be sensors - their contacts are disabled by the contact listener instead.
		*/

		const bool is_portal = owner.find<components::portal>() != nullptr;
		const bool is_stopping_character = melee_thrower.alive() && !fixture.IsSensor() && ::stops_thrown_melee(melee_thrower, owner);

		if (is_portal || is_stopping_character) {
			return copied_collider_type::STOPPER;
		}

		if (fixture.IsSensor()) {
			return copied_collider_type::NONE;
		}

		if (source_body->GetType() == b2_staticBody) {
			return copied_collider_type::STATIC_OBSTACLE;
		}

		const bool is_dynamic_obstacle = 
			source_body->GetType() == b2_dynamicBody
			&& owner.find<components::sentience>() == nullptr
			&& owner.find<components::item>() == nullptr
			&& owner.find<components::missile>() == nullptr
		;

		return is_dynamic_obstacle ? copied_collider_type::DYNAMIC_OBSTACLE : copied_collider_type::NONE;
	}

	/*
		A static collider copied into an obstacle world, compared between the steps to tell if the copies are up to date.
		The fixture could be destroyed and another one made at its address, so its owner and bounds are compared too.
	*/

	struct copied_static_collider {
		const b2Fixture* fixture = nullptr;
		unversioned_entity_id owner;
		b2AABB bounds;

		bool operator==(const copied_static_collider& b) const {
			return
				fixture == b.fixture
				&& owner == b.owner
				&& bounds.lowerBound == b.bounds.lowerBound
				&& bounds.upperBound == b.bounds.upperBound
			;
		}
	};

	/*
		The copies of the static colliders found within copied_area are kept between the steps.
		The dynamic obstacles and the stoppers - transient_bodies - are copied anew for every simulation,
		since they move, and a simulation pushes the dynamic ones.
	*/

	struct obstacle_world {
		std::unique_ptr<b2World> world;
		ltrb copied_area;
		std::vector<copied_static_collider> static_colliders;
		std::vector<b2Body*> transient_bodies;
	};

	/*
		How much bigger than needed the area of the copied static colliders is,
		so that they are not copied anew with every move of the thrower.
	*/

	static constexpr real32 COPIED_AREA_MULT = 1.5f;

	std::unique_ptr<b2World> make_empty_world() {
		auto world = std::make_unique<b2World>(b2Vec2(0.f, 0.f));
		world->SetAllowSleeping(false);
		return world;
	}

	b2AABB to_meters_aabb(const si_scaling si, const ltrb area) {
		const auto lt = si.get_meters(area.lt());
		const auto rb = si.get_meters(area.rb());

		b2AABB result;
		result.lowerBound = b2Vec2(lt.x, lt.y);
		result.upperBound = b2Vec2(rb.x, rb.y);

		return result;
	}

	void copy_fixture(b2Body& to, const b2Fixture& fixture, const bool is_sensor) {
		b2FixtureDef def;
		def.density = fixture.GetDensity();
		def.shape = fixture.GetShape();
		def.friction = fixture.GetFriction();
		def.restitution = fixture.GetRestitution();
		def.filter = fixture.GetFilterData();
		def.isSensor = is_sensor;
		def.userData = fixture.GetUserData();

		to.CreateFixture(&def);
	}

	b2Body* copy_body(b2World& world, const b2Body& source, const bool dynamic) {
		b2BodyDef def;
		set_body_def_transform(def, source.GetPosition(), source.GetAngle());

		if (dynamic) {
			/*
				Pushed by the throw like the real ones, from where they are and how they move now.
			*/

			def.type = b2_dynamicBody;
			def.allowSleep = false;
			def.linearVelocity = source.GetLinearVelocity();
			def.angularVelocity = source.GetAngularVelocity();
			def.linearDamping = source.GetLinearDamping();
			def.angularDamping = source.GetAngularDamping();
			def.fixedRotation = source.IsFixedRotation();
		}
		else {
			def.type = b2_staticBody;
		}

		return world.CreateBody(&def);
	}

	/*
		Brings the obstacle world up to date for a throw that could reach anywhere within needed_area.
		Returns whether there are any dynamic obstacles in it.
	*/

	bool update_obstacle_world(
		obstacle_world& w,
		std::vector<copied_static_collider>& found_static_colliders,
		std::unordered_map<const b2Body*, b2Body*>& copied_bodies,
		const cosmos& cosm,
		const ltrb needed_area,
		const b2Filter thrown_filter,
		const const_entity_handle melee_thrower
	) {
		const auto si = cosm.get_si();
		const auto& physics = cosm.get_solvable_inferred().physics;

		const bool area_covered = 
			w.world != nullptr
			&& w.copied_area.l <= needed_area.l
			&& w.copied_area.t <= needed_area.t
			&& w.copied_area.r >= needed_area.r
			&& w.copied_area.b >= needed_area.b
		;

		if (!area_covered) {
			w.copied_area = needed_area;
			w.copied_area.expand_from_center_mult(COPIED_AREA_MULT);
		}

		found_static_colliders.clear();

		physics.for_each_in_aabb_meters(
			to_meters_aabb(si, w.copied_area),
			thrown_filter,
			[&](const b2Fixture& fixture) {
				if (::classify_collider(cosm, fixture, melee_thrower) == copied_collider_type::STATIC_OBSTACLE) {
					found_static_colliders.push_back({ std::addressof(fixture), fixture.GetUserData(), fixture.GetAABB(0) });
				}

				return callback_result::CONTINUE;
			}
		);

		/*
			Sorted, since the order of a query changes whenever the tree of the real world is rebalanced.
		*/

		std::sort(
			found_static_colliders.begin(),
			found_static_colliders.end(),
			[](const auto& a, const auto& b) {
				return std::less<const b2Fixture*>()(a.fixture, b.fixture);
			}
		);

		if (!area_covered || found_static_colliders != w.static_colliders) {
			w.world = ::make_empty_world();
			w.transient_bodies.clear();
			copied_bodies.clear();

			for (const auto& collider : found_static_colliders) {
				const auto& fixture = *collider.fixture;
				auto& copied_body = copied_bodies[fixture.GetBody()];

				if (copied_body == nullptr) {
					copied_body = ::copy_body(*w.world, *fixture.GetBody(), false);
				}

				::copy_fixture(*copied_body, fixture, false);
			}

			std::swap(w.static_colliders, found_static_colliders);
		}

		for (const auto body : w.transient_bodies) {
			w.world->DestroyBody(body);
		}

		w.transient_bodies.clear();
		copied_bodies.clear();

		bool has_dynamic_obstacles = false;

		physics.for_each_in_aabb_meters(
			to_meters_aabb(si, needed_area),
			thrown_filter,
			[&](const b2Fixture& fixture) {
				const auto type = ::classify_collider(cosm, fixture, melee_thrower);

				const bool is_stopper = type == copied_collider_type::STOPPER;
				const bool is_dynamic_obstacle = type == copied_collider_type::DYNAMIC_OBSTACLE;

				if (!is_stopper && !is_dynamic_obstacle) {
					return callback_result::CONTINUE;
				}

				auto& copied_body = copied_bodies[fixture.GetBody()];

				if (copied_body == nullptr) {
					copied_body = ::copy_body(*w.world, *fixture.GetBody(), is_dynamic_obstacle);
					w.transient_bodies.push_back(copied_body);
				}

				::copy_fixture(*copied_body, fixture, is_stopper);

				has_dynamic_obstacles = has_dynamic_obstacles || is_dynamic_obstacle;

				return callback_result::CONTINUE;
			}
		);

		return has_dynamic_obstacles;
	}

	simulated_flight simulate_flight(
		b2World& world,
		const cosmos& cosm,
		const throw_trajectory_input& in,
		const thrown_shapes& shapes,
		const invariants::fixtures& fixtures_def,
		const b2Filter thrown_filter
	) {
		const auto si = cosm.get_si();
		const auto& clk = cosm.get_clock();
		const auto dt = clk.dt;
		const auto dt_secs = dt.in_seconds();

		const auto start_pos = si.get_meters(in.start.pos);
		const auto start_vel = si.get_meters(in.start.get_direction() * in.speed);

		b2BodyDef body_def;
		body_def.type = b2_dynamicBody;
		body_def.bullet = true;
		body_def.allowSleep = false;
		set_body_def_transform(body_def, b2Vec2(start_pos.x, start_pos.y), in.start.rotation * DEG_TO_RAD<real32>);
		body_def.linearDamping = in.damping.linear;
		body_def.angularDamping = in.damping.angular;
		body_def.fixedRotation = in.fixed_rotation;

		const auto body = world.CreateBody(&body_def);

		body->SetLinearDampingVec(b2Vec2(in.damping.linear_axis_aligned.x, in.damping.linear_axis_aligned.y));
		body->SetAngledDampingEnabled(in.angled_damping);

		b2FixtureDef fixture_def;
		fixture_def.density = fixtures_def.density;
		fixture_def.friction = fixtures_def.friction;
		fixture_def.restitution = fixtures_def.restitution;
		fixture_def.filter = thrown_filter;

		if (shapes.circle.has_value()) {
			fixture_def.shape = std::addressof(*shapes.circle);
			body->CreateFixture(&fixture_def);
		}

		if (!shapes.polygons.empty()) {
			fixture_def.filter = ::make_thrown_melee_hitting_filter(thrown_filter);

			for (const auto& polygon : shapes.polygons) {
				fixture_def.shape = std::addressof(polygon);
				body->CreateFixture(&fixture_def);
			}

			b2CircleShape bounce_shape;
			bounce_shape.m_radius = si.get_meters(shapes.bounce_radius);
			bounce_shape.m_p = body->GetLocalCenter();

			auto bounce_def = fixture_def;
			bounce_def.shape = &bounce_shape;
			bounce_def.density = 0.f;
			bounce_def.friction = THROWN_MELEE_BOUNCE_FRICTION;
			bounce_def.restitution = THROWN_MELEE_BOUNCE_RESTITUTION;
			bounce_def.filter = ::make_thrown_melee_bouncing_filter(thrown_filter);

			body->CreateFixture(&bounce_def);
		}

		/*
			A thrown item is placed with rigid_body::set_transform, which resets its center of mass to its origin -
			after the colliders were created, so the bounce circle stays at the center of the real shape.
			So it really spins about its origin, with the inertia about the center of its shape.
		*/

		body->m_sweep.localCenter.SetZero();
		body->m_sweep.c0 = body->m_xf.p;
		body->m_sweep.c = body->m_xf.p;

		/*
			Launched only once the colliders exist, like in perform_transfer and item_system -
			every collider created moves the center of mass, which Box2D compensates by changing the velocity of a spinning body.
		*/

		body->SetLinearVelocity(b2Vec2(start_vel.x, start_vel.y));
		body->SetAngularVelocity(in.angular_velocity * DEG_TO_RAD<real32>);

		auto read_transform = [&]() {
			const auto p = si.get_pixels(vec2(body->GetPosition().x, body->GetPosition().y));
			return transformr(p, body->GetAngle() * RAD_TO_DEG<real32>);
		};

		auto result = simulated_flight();
		result.path.push_back(in.start);
		result.laser_path.push_back(in.start.pos);

		/*
			Where the laser path was when the flight first touched something, in case no bounce is found.
		*/

		std::optional<std::size_t> laser_contact_fallback;

		const auto bounce_max_cos = std::cos(BOUNCE_MIN_DEGREES * DEG_TO_RAD<real32>);

		/*
			Where the flight before and after a step meet - each extended up to twice as far as it flies in a step.
		*/

		auto find_bounce_corner = [&](const vec2 pos_before, const vec2 velocity_before, const vec2 pos_after, const vec2 velocity_after) {
			if (velocity_before.is_epsilon(1.f) || velocity_after.is_epsilon(1.f)) {
				return std::optional<vec2>();
			}

			if (vec2(velocity_before).normalize().dot(vec2(velocity_after).normalize()) > bounce_max_cos) {
				return std::optional<vec2>();
			}

			const auto reach = 2 * dt_secs;

			const auto corner = ::segment_segment_intersection(
				pos_before,
				pos_before + velocity_before * reach,
				pos_after,
				pos_after - velocity_after * reach
			);

			return corner.hit ? std::optional<vec2>(corner.intersection) : std::optional<vec2>();
		};

		/*
			Spun like in perform_transfer and by its floor hits like in item_system,
			so that it is rotated just like it really would be.
			Thrown melee weapons fall like dropped items, only with every interval a bit longer,
			while thrown explosives hop as thrown up - see release_explosive.
		*/

		body->ApplyAngularImpulse(in.angular_impulse * body->GetMass(), true);

		auto fall = item_fall_state();
		fall.num_falls = in.num_falls;

		if (in.explosive) {
			::start_falling_like_thrown_explosive(
				fall,
				in.speed,
				in.floor_hits_when_thrown,
				in.fuse_delay_ms,
				in.nontemporal_seed,
				clk.now
			);
		}
		else {
			::start_falling_like_dropped(
				fall,
				DROPPED_ITEM_FALL_SECS * THROWN_MELEE_HOP_DURATION_MULT,
				DROPPED_ITEM_FALL_HEIGHT,
				in.nontemporal_seed,
				clk.now
			);

			fall.thrown_melee = true;
		}

		const auto adapter = simulated_body { *body, si };

		for (uint32_t i = 0; i < in.max_steps; ++i) {
			const auto now = augs::stepped_timestamp { clk.now.step + i };

			/*
				Thrown melee weapons stop being thrown at the beginning of the first step they are too slow to hurt.
			*/

			if (adapter.get_velocity().length() < in.min_speed) {
				break;
			}

			/*
				The floor hits are advanced before the throw in a step,
				so the first one may only come in the next step.
			*/

			if (i > 0 && ::is_floor_hit_due(fall, now, dt)) {
				::hit_the_floor(adapter, fall, in.explosive, now, clk.logic_speed);
			}

			/*
				Sensors are not swept like bullets, so a fast throw could skip a thin portal or a character in a single step
				and bounce off a wall behind it. So whatever it would fly through this step is checked beforehand.
			*/

			{
				const auto p1 = body->GetPosition();
				const auto v = body->GetLinearVelocity();
				const auto p2 = b2Vec2(p1.x + v.x * dt_secs, p1.y + v.y * dt_secs);

				if ((p2 - p1).LengthSquared() > 0.f) {
					auto callback = stopper_ray_callback();
					callback.thrown_body = body;

					world.RayCast(&callback, p1, p2);

					if (callback.stopper_fraction <= 1.f && callback.stopper_fraction < callback.obstacle_fraction) {
						const auto entry = p1 + callback.stopper_fraction * (p2 - p1);
						const auto entry_pos = si.get_pixels(vec2(entry.x, entry.y));

						result.path.push_back(transformr(entry_pos, body->GetAngle() * RAD_TO_DEG<real32>));
						result.laser_path.push_back(entry_pos);

						if (!result.first_contact_index.has_value()) {
							result.first_contact_index = result.path.size() - 1;
						}

						if (!result.laser_first_contact_index.has_value()) {
							result.laser_first_contact_index = result.laser_path.size() - 1;
						}

						result.stopped_by = callback.stopper;
						break;
					}
				}
			}

			const auto pos_before = read_transform().pos;
			const auto velocity_before = adapter.get_velocity();

			world.Step(dt_secs, 8, 3);

			if (in.faces_flight_direction) {
				/*
					See melee_system::face_thrown_melee_along_flight.
				*/

				const auto velocity = adapter.get_velocity();

				if (!velocity.is_epsilon(1.f)) {
					const auto rotation = ::calc_rotation_facing_flight(body->GetAngle() * RAD_TO_DEG<real32>, velocity, dt_secs);

					body->SetTransform(body->GetPosition(), rotation * DEG_TO_RAD<real32>);
					body->SetAngularVelocity(0.f);
				}
			}

			result.path.push_back(read_transform());

			{
				const auto pos_after = result.path.back().pos;

				const auto corner = find_bounce_corner(pos_before, velocity_before, pos_after, adapter.get_velocity());

				if (corner.has_value()) {
					result.laser_path.push_back(*corner);

					if (!result.laser_first_contact_index.has_value()) {
						result.laser_first_contact_index = result.laser_path.size() - 1;
					}
				}

				result.laser_path.push_back(pos_after);
			}

			bool entered_stopper = false;

			for (auto edge = body->GetContactList(); edge != nullptr; edge = edge->next) {
				const auto contact = edge->contact;

				if (!contact->IsTouching() || !contact->IsEnabled()) {
					continue;
				}

				if (!result.first_contact_index.has_value()) {
					result.first_contact_index = result.path.size() - 1;
					laser_contact_fallback = result.laser_path.size() - 1;
				}

				for (const auto fixture : { contact->GetFixtureA(), contact->GetFixtureB() }) {
					if (fixture->IsSensor()) {
						entered_stopper = true;
						result.stopped_by = fixture->GetUserData();
					}
				}
			}

			if (entered_stopper) {
				break;
			}

			if (adapter.get_velocity().length() < REST_SPEED) {
				break;
			}
		}

		world.DestroyBody(body);

		if (!result.laser_first_contact_index.has_value()) {
			result.laser_first_contact_index = laser_contact_fallback;
		}

		return result;
	}

	std::optional<throw_trajectory_input> make_throw_input(
		const const_entity_handle character,
		const const_entity_handle item
	) {
		const auto& cosm = item.get_cosmos();

		auto in = throw_trajectory_input();
		in.item = item.get_id();
		in.start = item.get_logic_transform();
		in.nontemporal_seed = cosm.get_nontemporal_rng_seed_for(item);
		in.num_falls = item.get<components::item>().get_fall().num_falls;

		/*
			Items have no movement, which alone could disable angled damping - see calc_angled_damping_enabled.
		*/

		const auto& body_def = item.get<invariants::rigid_body>();
		in.angled_damping = body_def.angled_damping;
		in.fixed_rotation = body_def.fixed_rotation;

		if (const auto fuse = item.find<components::hand_fuse>()) {
			const auto& fuse_def = item.get<invariants::hand_fuse>();

			if (fuse_def.is_like_plantable_bomb()) {
				return std::nullopt;
			}

			/*
				Previewed as thrown with the primary button, unless already armed with another.
			*/

			const auto arming_source = fuse->armed() ? fuse->arming_source : static_cast<uint8_t>(arming_source_type::SHOOT_INTENT);
			const auto launch = ::calc_explosive_throw_launch(item, character, arming_source);

			/*
				Damped like in calc_damping_mults.
			*/

			auto damping = body_def.damping;
			damping.linear *= launch.damping_mult;
			damping.sanitize();

			in.explosive = true;
			in.arming_source = arming_source;
			in.speed = launch.speed;
			in.angular_impulse = launch.angular_impulse;
			in.floor_hits_when_thrown = fuse_def.floor_hits_when_thrown;
			in.fuse_delay_ms = fuse->fuse_delay_ms;
			in.damping = damping;
			in.max_steps = ::calc_explosive_flight_steps(cosm.get_clock(), fuse->fuse_delay_ms, fuse->when_armed);

			return in;
		}

		if (const auto melee_def = item.find<invariants::melee>()) {
			const auto transfers_def = character.find<invariants::item_slot_transfers>();

			if (transfers_def == nullptr) {
				return std::nullopt;
			}

			const auto mult = item.get_current_slot().get_type() == slot_function::SECONDARY_HAND ? -1.f : 1.f;

			auto damping = body_def.damping;
			damping.sanitize();

			in.speed = transfers_def->standard_throw_impulse.linear;
			in.angular_velocity = mult * melee_def->throw_def.throw_angular_speed;
			in.faces_flight_direction = ::faces_flight_direction(*melee_def);
			in.min_speed = melee_def->throw_def.min_speed_to_hurt;
			in.damping = damping;
			in.max_steps = static_cast<uint32_t>(MAX_MELEE_FLIGHT_SECS / cosm.get_clock().dt.in_seconds());

			return in;
		}

		return std::nullopt;
	}

	/*
		As a laser, only the END silhouette is made.
	*/

	void make_silhouettes(
		std::vector<throw_silhouette>& output,
		const simulated_flight& flight,
		const simulated_flight& free_flight,
		const real32 spacing,
		const bool as_laser
	) {
		output.clear();

		if (as_laser) {
			if (!flight.path.empty()) {
				output.push_back({ flight.path.back(), flight.path.back(), throw_silhouette_type::END, throw_silhouette::LAST_INDEX });
			}

			return;
		}

		auto lerp = [](const transformr& a, const transformr& b, const real32 t) {
			return a.interp(b, t);
		};

		const auto& path = flight.path;
		const auto length = augs::calc_path_length(path, path.size());

		auto index_of = [spacing](const real32 distance) {
			return static_cast<uint32_t>(std::lround(distance / spacing));
		};

		if (flight.first_contact_index.has_value()) {
			const auto& free_path = free_flight.path;
			const auto free_length = augs::calc_path_length(free_path, free_path.size());

			/*
				Blocked from the last point where the real trajectory still follows the unobstructed one.
			*/

			const auto last_shared_index = [&]() {
				std::size_t i = 0;

				while (i + 1 < path.size() && i + 1 < free_path.size() && (path[i + 1].pos - free_path[i + 1].pos).length_sq() < 0.25f) {
					++i;
				}

				return i;
			}();

			const auto contact_distance = augs::calc_path_length(free_path, last_shared_index);
			const auto first_blocked = (std::floor(contact_distance / spacing) + 1.f) * spacing;

			augs::sample_along_path(free_path, spacing, first_blocked, free_length, lerp, [&](const transformr& t, const real32 distance) {
				output.push_back({ t, t, throw_silhouette_type::BLOCKED, index_of(distance) });
			});

			if (!free_path.empty()) {
				output.push_back({ free_path.back(), free_path.back(), throw_silhouette_type::BLOCKED, throw_silhouette::LAST_INDEX });
			}
		}

		augs::sample_along_path(path, spacing, spacing, length, lerp, [&](const transformr& t, const real32 distance) {
			output.push_back({ t, t, throw_silhouette_type::PATH, index_of(distance) });
		});

		if (!path.empty()) {
			output.push_back({ path.back(), path.back(), throw_silhouette_type::END, throw_silhouette::LAST_INDEX });
		}
	}

	/*
		Simplified separately before and after the first contact, so that it stays a point of the laser.
	*/

	void make_laser(throw_laser& output, const simulated_flight& flight) {
		output.current.clear();
		output.first_contact = std::nullopt;

		const auto& points = flight.laser_path;

		if (points.empty()) {
			return;
		}

		const auto last_index = points.size() - 1;

		if (const auto contact = flight.laser_first_contact_index; contact.has_value() && *contact > 0) {
			augs::simplify_polyline(points, 0, *contact, LASER_TOLERANCE, output.current);
			output.first_contact = output.current.size() - 1;
			output.current.pop_back();
			augs::simplify_polyline(points, *contact, last_index, LASER_TOLERANCE, output.current);
		}
		else {
			augs::simplify_polyline(points, 0, last_index, LASER_TOLERANCE, output.current);
		}
	}

	/*
		See throw_laser.
	*/

	void interpolate_laser_from(throw_laser& next, const throw_laser* const last) {
		next.previous = last != nullptr ? last->current : next.current;

		if (next.previous.empty() || next.current.empty()) {
			next.previous = next.current;
			return;
		}

		/*
			Without the padding of the last step, so that it does not pile up while the trajectory is shown.
		*/

		while (next.previous.size() >= 2 && next.previous[next.previous.size() - 2] == next.previous.back()) {
			next.previous.pop_back();
		}

		const auto size = std::max(next.previous.size(), next.current.size());

		auto pad = [size](std::vector<vec2>& points) {
			const auto last_point = points.back();
			points.resize(size, last_point);
		};

		pad(next.previous);
		pad(next.current);
	}

	/*
		Every silhouette starts from where the one of the same type and index was in the previous step,
		or from where it is now if there was none.
	*/

	void interpolate_from(throw_trajectory& next, const throw_trajectory* const last) {
		/*
			make_silhouettes outputs them by type in this order, and by increasing index within a type,
			so the two steps are matched by merging.
		*/

		auto order_of = [](const throw_silhouette& t) {
			const auto type_order = [&]() {
				switch (t.type) {
					case throw_silhouette_type::BLOCKED: return 0;
					case throw_silhouette_type::PATH: return 1;
					default: return 2;
				}
			}();

			return std::make_pair(type_order, t.index);
		};

		::interpolate_laser_from(next.laser, last != nullptr ? std::addressof(last->laser) : nullptr);

		std::size_t j = 0;

		for (auto& silhouette : next.silhouettes) {
			silhouette.previous = silhouette.current;

			if (last == nullptr) {
				continue;
			}

			const auto& last_silhouettes = last->silhouettes;
			const auto order = order_of(silhouette);

			while (j < last_silhouettes.size() && order_of(last_silhouettes[j]) < order) {
				++j;
			}

			if (j < last_silhouettes.size() && order_of(last_silhouettes[j]) == order) {
				silhouette.previous = last_silhouettes[j].current;
			}
		}
	}

	/*
		As big as the item lying on the ground - see calc_ground_item_scale.
	*/

	vec2i calc_silhouette_size(const const_entity_handle item) {
		if (const auto sprite = item.find<invariants::sprite>()) {
			return vec2i(vec2(sprite->size) * calc_ground_size_mult(item));
		}

		return vec2i::zero;
	}
}

struct throw_trajectory_system::obstacle_worlds {
	obstacle_world explosive;
	obstacle_world melee;
	std::unique_ptr<b2World> empty_world = ::make_empty_world();

	std::vector<copied_static_collider> found_static_colliders;
	std::unordered_map<const b2Body*, b2Body*> copied_bodies;
};

throw_trajectory_system::throw_trajectory_system() = default;
throw_trajectory_system::~throw_trajectory_system() = default;

throw_trajectory_system::throw_trajectory_system(throw_trajectory_system&&) noexcept = default;
throw_trajectory_system& throw_trajectory_system::operator=(throw_trajectory_system&&) noexcept = default;

void throw_trajectory_system::clear() {
	trajectories.clear();
	previous.clear();
	worlds.reset();
}

void throw_trajectory_system::advance(
	const const_entity_handle viewed_character,
	const throw_trajectory_flags& flags,
	const real32 silhouette_gap
) {
	if (viewed_character.dead()) {
		trajectories.clear();
		return;
	}

	const auto& cosm = viewed_character.get_cosmos();

	if (worlds == nullptr) {
		worlds = std::make_unique<obstacle_worlds>();
	}

	std::swap(previous, trajectories);
	trajectories.clear();

	for (const auto& item_id : viewed_character.get_wielded_items()) {
		const auto item = cosm[item_id];

		if (item.dead() || !::is_throw_trajectory_shown(viewed_character, item, flags)) {
			continue;
		}

		auto in = ::make_throw_input(viewed_character, item);

		if (!in.has_value()) {
			continue;
		}

		in->silhouette_gap = std::max(0.f, silhouette_gap);
		in->as_laser = ::is_throw_trajectory_laser(viewed_character, item, flags);

		const auto last = std::find_if(previous.begin(), previous.end(), [&](const throw_trajectory& t) {
			return t.input.item == in->item;
		});

		const auto last_ptr = [&]() -> const throw_trajectory* {
			if (last == previous.end() || last->input.arming_source != in->arming_source) {
				return nullptr;
			}

			return std::addressof(*last);
		}();

		/*
			Thrown melee weapons stop at characters, and dynamic obstacles might move,
			so then they are simulated anew every step.
		*/

		const bool reusable = 
			in->explosive 
			&& last_ptr != nullptr 
			&& !last_ptr->has_dynamic_obstacles 
			&& last_ptr->input == *in
		;

		if (reusable) {
			trajectories.emplace_back(*last_ptr);
			::interpolate_from(trajectories.back(), nullptr);
			continue;
		}

		const auto shapes = ::make_thrown_shapes(item, in->explosive);
		const auto& fixtures_def = item.get<invariants::fixtures>();

		const auto thrown_filter = filters[
			in->explosive ?
			predefined_filter_type::FLYING_EXPLOSIVE :
			predefined_filter_type::FLYING_MELEE
		];

		const auto dt_secs = cosm.get_clock().dt.in_seconds();

		/*
			With all the pushes of its floor hits, a thrown melee weapon flies at most a few of them farther -
			and only until it is too slow to hurt.
		*/

		const auto pushes_speed = in->explosive ? 0.f : 2 * DROPPED_ITEM_PUSH_SPEED;
		const auto launch_speed = in->speed + pushes_speed;

		const auto max_distance = std::min(
			augs::calc_damped_distance(launch_speed, in->damping.linear, dt_secs, in->max_steps),
			augs::calc_damped_distance_until_speed(launch_speed, in->min_speed, in->damping.linear)
		);
		const auto size = ::calc_silhouette_size(item);
		const auto radius = static_cast<real32>(std::max(size.x, size.y)) / 2;
		const auto reach = max_distance + radius + OBSTACLES_QUERY_MARGIN;

		const auto aabb = ltrb::center_and_size(in->start.pos, vec2::square(2 * reach));

		/*
			Grenades fly through characters.
		*/

		const auto melee_thrower = in->explosive ? cosm[entity_id()] : viewed_character;

		auto& obstacles = in->explosive ? worlds->explosive : worlds->melee;

		const bool has_dynamic_obstacles = ::update_obstacle_world(
			obstacles,
			worlds->found_static_colliders,
			worlds->copied_bodies,
			cosm,
			aabb,
			thrown_filter,
			melee_thrower
		);

		const auto flight = ::simulate_flight(*obstacles.world, cosm, *in, shapes, fixtures_def, thrown_filter);

		auto free_flight = simulated_flight();

		/*
			The flight as if nothing was in the way is needed only for the BLOCKED silhouettes.
		*/

		if (flight.first_contact_index.has_value() && !in->as_laser) {
			free_flight = ::simulate_flight(*worlds->empty_world, cosm, *in, shapes, fixtures_def, thrown_filter);
		}

		auto new_trajectory = throw_trajectory();
		new_trajectory.input = *in;
		new_trajectory.image = item.get_image_id();
		new_trajectory.size = size;
		new_trajectory.has_dynamic_obstacles = has_dynamic_obstacles;

		/*
			Like the laser does - the friendly fire setting is not known to the view.
		*/

		const auto stopper = flight.stopped_by.is_set() ? cosm[flight.stopped_by] : cosm[entity_id()];

		if (stopper.alive() && stopper.has<components::sentience>()) {
			const auto attitude = ::calc_attitude(viewed_character, stopper);
			const bool enemy = attitude == attitude_type::WANTS_TO_KILL || attitude == attitude_type::WANTS_TO_KNOCK_UNCONSCIOUS;

			new_trajectory.end_type = enemy ? throw_end_type::ENEMY : throw_end_type::FRIEND;
		}

		::make_silhouettes(new_trajectory.silhouettes, flight, free_flight, (2.f + in->silhouette_gap) * std::max(radius, 1.f), in->as_laser);

		if (in->as_laser) {
			::make_laser(new_trajectory.laser, flight);
		}

		::interpolate_from(new_trajectory, last_ptr);

		trajectories.emplace_back(std::move(new_trajectory));
	}

	previous.clear();
}
