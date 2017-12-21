#include <cstring>
#include "physics_world_cache.h"

#include "game/components/item_component.h"
#include "game/components/driver_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/special_physics_component.h"
#include "game/components/shape_polygon_component.h"
#include "game/components/shape_circle_component.h"
#include "game/components/all_inferred_state_component.h"

#include "game/messages/collision_message.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/will_soon_be_deleted.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/entity_handle.h"

#include "augs/templates/dynamic_cast_dispatch.h"
#include "augs/build_settings/setting_debug_physics_world_cache_copy.h"
#include "game/inferred_caches/relational_cache.h"

bool physics_world_cache::cache_exists_for_rigid_body(const const_entity_handle handle) const {
	return 
		handle.alive() 
		&& get_rigid_body_cache(handle).body != nullptr
	;
}

bool physics_world_cache::cache_exists_for_colliders(const const_entity_handle handle) const {
	return
		handle.alive()
		&& get_colliders_cache(handle).all_fixtures_in_component.size() > 0u
	;
}

bool physics_world_cache::cache_exists_for_joint(const const_entity_handle handle) const {
	return 
		handle.alive() 
		&& get_joint_cache(handle).joint != nullptr
	;
}

rigid_body_cache& physics_world_cache::get_rigid_body_cache(const entity_id id) {
	return rigid_body_caches[linear_cache_key(id)];
}

colliders_cache& physics_world_cache::get_colliders_cache(const entity_id id) {
	return colliders_caches[linear_cache_key(id)];
}

joint_cache& physics_world_cache::get_joint_cache(const entity_id id) {
	return joint_caches[linear_cache_key(id)];
}

const rigid_body_cache& physics_world_cache::get_rigid_body_cache(const entity_id id) const {
	return rigid_body_caches[linear_cache_key(id)];
}

const colliders_cache& physics_world_cache::get_colliders_cache(const entity_id id) const {
	return colliders_caches[linear_cache_key(id)];
}

const joint_cache& physics_world_cache::get_joint_cache(const entity_id id) const {
	return joint_caches[linear_cache_key(id)];
}

void physics_world_cache::destroy_cache_of(const const_entity_handle handle) {
	const auto& cosmos = handle.get_cosmos();

	if (cache_exists_for_rigid_body(handle)) {
		auto& cache = get_rigid_body_cache(handle);
		
		for (const b2Fixture* f = cache.body->m_fixtureList; f != nullptr; f = f->m_next) {
			get_colliders_cache(cosmos[f->GetUserData()]) = colliders_cache();
		}
		
		for (const b2JointEdge* j = cache.body->m_jointList; j != nullptr; j = j->next) {
			get_joint_cache(cosmos[j->joint->GetUserData()]) = joint_cache();
		}
		
		// no need to manually destroy each fixture and joint of the body,
		// Box2D will take care of that after just deleting the body.

		b2world->DestroyBody(cache.body);

		cache = rigid_body_cache();
	}
	
	if (cache_exists_for_colliders(handle)) {
		auto& cache = get_colliders_cache(handle);
		auto& owner_body_cache = get_rigid_body_cache(cosmos[cache.all_fixtures_in_component[0]->GetBody()->GetUserData()]);

		for (b2Fixture* f : cache.all_fixtures_in_component) {
			owner_body_cache.body->DestroyFixture(f);
		}

		cache = colliders_cache();
	}

	if (cache_exists_for_joint(handle)) {
		auto& cache = get_joint_cache(handle);

		b2world->DestroyJoint(cache.joint);

		cache = joint_cache();
	}
}

void physics_world_cache::infer_cache_for(const const_entity_handle handle) {
	const auto& cosmos = handle.get_cosmos();
	const auto& relational = cosmos.solvable.inferred.relational;

	if (const bool is_already_constructed = cache_exists_for_rigid_body(handle)) {
		return;
	}

	infer_cache_for_fixtures(handle);

	if (
		const auto rigid_body = handle.find<components::rigid_body>();
		rigid_body && rigid_body.is_activated()
	) {
		const auto& physics_data = rigid_body.get_raw_component();
		auto& cache = get_rigid_body_cache(handle);

		b2BodyDef def;

		switch (physics_data.body_type) {
		case rigid_body_type::DYNAMIC: def.type = b2BodyType::b2_dynamicBody; break;
		case rigid_body_type::STATIC: def.type = b2BodyType::b2_staticBody; break;
		case rigid_body_type::KINEMATIC: def.type = b2BodyType::b2_kinematicBody; break;
		default:ensure(false) break;
		}

		def.userData = handle.get_id();
		def.bullet = physics_data.bullet;
		def.transform = physics_data.transform;
		def.sweep = physics_data.sweep;
		def.angularDamping = physics_data.angular_damping;
		def.linearDamping = physics_data.linear_damping;
		def.fixedRotation = physics_data.fixed_rotation;
		def.allowSleep = physics_data.allow_sleep;
		def.gravityScale = physics_data.gravity_scale;
		def.active = true;
		def.linearVelocity = b2Vec2(physics_data.velocity);
		def.angularVelocity = physics_data.angular_velocity;

		cache.body = b2world->CreateBody(&def);
		cache.body->SetAngledDampingEnabled(physics_data.angled_damping);
		
		const auto fixture_entities = rigid_body.get_fixture_entities();

		for (const auto f : fixture_entities) {
			infer_cache_for_fixtures(cosmos[f]);
		}

		const auto joint_entities = rigid_body.get_attached_joints();
		
		for (const auto j : joint_entities) {
			infer_cache_for_joint(cosmos[j]);
		}
	}

	infer_cache_for_joint(handle);
}

void physics_world_cache::infer_cache_for_fixtures(const const_entity_handle handle) {
	if (const bool is_already_constructed = cache_exists_for_colliders(handle)) {
		return;
	}
	
	if (
		const auto colliders = handle.find<components::fixtures>();

		colliders != nullptr
		&& colliders.is_activated()
		&& cache_exists_for_rigid_body(handle.get_owner_body())
	) {
		const auto si = handle.get_cosmos().get_si();
		const auto owner_body_entity = handle.get_owner_body();
		ensure(owner_body_entity.alive());
		auto* const owner_b2Body = get_rigid_body_cache(owner_body_entity).body;
		const auto& colliders_data = colliders.get_raw_component();

		b2FixtureDef fixdef;
		fixdef.density = colliders_data.density;
		fixdef.friction = colliders_data.friction;
		fixdef.isSensor = colliders_data.sensor;
		fixdef.filter = colliders_data.filter;
		fixdef.restitution = colliders_data.restitution;
		fixdef.userData = handle.get_id();

		auto& all_fixtures_in_component = get_colliders_cache(handle).all_fixtures_in_component;
		ensure(all_fixtures_in_component.empty());
		
		if (
			const auto shape_polygon = handle.find<components::shape_polygon>();
			
			shape_polygon != nullptr 
			&& shape_polygon.is_activated()
		) {
			auto transformed_shape = shape_polygon.get_raw_component().shape;
			transformed_shape.offset_vertices(colliders.get_total_offset());

			for (std::size_t ci = 0; ci < transformed_shape.convex_polys.size(); ++ci) {
				const auto& convex = transformed_shape.convex_polys[ci];
				std::vector<b2Vec2> b2verts(convex.begin(), convex.end());

				for (auto& v : b2verts) {
					v = si.get_meters(v);
				}

				b2PolygonShape shape;
				shape.Set(b2verts.data(), static_cast<int32>(b2verts.size()));

				fixdef.shape = &shape;
				b2Fixture* const new_fix = owner_b2Body->CreateFixture(&fixdef);

				ensure(static_cast<short>(ci) < std::numeric_limits<short>::max());
				new_fix->index_in_component = static_cast<short>(ci);

				all_fixtures_in_component.push_back(new_fix);
			}

			return;
		}
		
		
		if (
			const auto shape_circle = handle.find<components::shape_circle>();
			shape_circle != nullptr && shape_circle.is_activated()
		) {
			b2CircleShape shape;
			shape.m_radius = si.get_meters(shape_circle.get_radius());
		
			fixdef.shape = &shape;
			b2Fixture* const new_fix = owner_b2Body->CreateFixture(&fixdef);
			
			new_fix->index_in_component = 0u;
			all_fixtures_in_component.push_back(new_fix);
			
			return;
		}
		
		ensure(false && "fixtures requested with no shape attached!");
	}
}

void physics_world_cache::infer_cache_for_joint(const const_entity_handle handle) {
	if (const bool is_already_constructed = cache_exists_for_joint(handle)) {
		return;
	}

	const auto& cosmos = handle.get_cosmos();

	if (
		const auto motor_joint = handle.find<components::motor_joint>();

		motor_joint != nullptr
		&& motor_joint.is_activated()
		&& cache_exists_for_rigid_body(cosmos[motor_joint.get_target_bodies()[0]])
		&& cache_exists_for_rigid_body(cosmos[motor_joint.get_target_bodies()[1]])
	) {
		const components::motor_joint joint_data = motor_joint.get_raw_component();

		const auto si = handle.get_cosmos().get_si();
		auto& cache = get_joint_cache(handle);
		
		ensure(nullptr == cache.joint);

		b2MotorJointDef def;
		def.userData = handle.get_id();
		def.bodyA = get_rigid_body_cache(cosmos[joint_data.target_bodies[0]]).body;
		def.bodyB = get_rigid_body_cache(cosmos[joint_data.target_bodies[1]]).body;
		def.collideConnected = joint_data.collide_connected;
		def.linearOffset = b2Vec2(si.get_meters(joint_data.linear_offset));
		def.angularOffset = DEG_TO_RAD<float> * joint_data.angular_offset;
		def.maxForce = si.get_meters(joint_data.max_force);
		def.maxTorque = joint_data.max_torque;
		def.correctionFactor = joint_data.correction_factor;

		cache.joint = b2world->CreateJoint(&def);
	}
}

void physics_world_cache::reserve_caches_for_entities(const size_t n) {
	rigid_body_caches.resize(n);
	colliders_caches.resize(n);
	joint_caches.resize(n);
}

b2Fixture_index_in_component physics_world_cache::get_index_in_component(
	const b2Fixture* const f, 
	const const_entity_handle handle
) {
	ensure(f->index_in_component != -1);

	b2Fixture_index_in_component result;
	result.convex_shape_index = static_cast<std::size_t>(f->index_in_component);

	ensure_eq(f, colliders_caches[linear_cache_key(handle)].all_fixtures_in_component[result.convex_shape_index]);

	return result;

	//const auto this_cache_id = linear_cache_key(handle);
	//const auto& cache = colliders_caches[this_cache_id];
	//const auto& all = cache.all_fixtures_in_component;
	//
	//const auto it = std::find(all.begin(), all.end(), f);
	//
	//if (it != all.end()) {
	//	b2Fixture_index_in_component result;
	//	result.convex_shape_index = it - all.begin();
	//
	//	return result;
	//}
	//
	//ensure(false);
	//return b2Fixture_index_in_component();
}

physics_world_cache::physics_world_cache() :
	b2world(new b2World(b2Vec2(0.f, 0.f))) 
{
	b2world->SetAllowSleeping(true);
	b2world->SetAutoClearForces(false);
}

physics_world_cache::physics_world_cache(const physics_world_cache& b) {
	*this = b;
}

physics_world_cache& physics_world_cache::operator=(const physics_world_cache& b) {
	ray_casts_since_last_step = b.ray_casts_since_last_step;
	accumulated_messages = b.accumulated_messages;

	b2World& migrated_b2World = *b2world.get();
	migrated_b2World.~b2World();
	new (&migrated_b2World) b2World(b2Vec2(0.f, 0.f));

	const b2World& source_b2World = *b.b2world.get();

	// do the initial trivial copy of all fields,
	// we will migrate all pointers shortly
	migrated_b2World = source_b2World;

	// b2BlockAllocator has a null operator=, so the source_b2World preserves its default-constructed allocator
	// even after the above copy. No need for further operations in this regard.
	// new (&migrated_b2World.m_blockAllocator) b2BlockAllocator;

	// reset the allocator pointer to the new one
	migrated_b2World.m_contactManager.m_allocator = &migrated_b2World.m_blockAllocator;

	std::unordered_map<void*, void*> pointer_migrations;
	std::unordered_map<void*, bool> contact_edge_a_or_b_in_contacts;
	std::unordered_map<void*, bool> joint_edge_a_or_b_in_joints;

	b2BlockAllocator& migrated_allocator = migrated_b2World.m_blockAllocator;

#define my_offsetof(s,m) ((std::size_t)&reinterpret_cast<char const volatile&>((((s*)0)->m)))
	
	const auto contact_edge_a_offset = my_offsetof(b2Contact, m_nodeA);
	const auto contact_edge_b_offset = my_offsetof(b2Contact, m_nodeB);

	const auto joint_edge_a_offset = my_offsetof(b2Joint, m_edgeA);
	const auto joint_edge_b_offset = my_offsetof(b2Joint, m_edgeB);

#if DEBUG_PHYSICS_SYSTEM_COPY
	std::unordered_set<void**> already_migrated_pointers;
#endif

	auto migrate_pointer = [
#if DEBUG_PHYSICS_SYSTEM_COPY
		&already_migrated_pointers, 
#endif
		&pointer_migrations, 
		&migrated_allocator
	](
		auto*& pointer_to_be_migrated, 
		const unsigned count = 1
	) {
#if DEBUG_PHYSICS_SYSTEM_COPY
		ensure(already_migrated_pointers.find(reinterpret_cast<void**>(&pointer_to_be_migrated)) == already_migrated_pointers.end());
		already_migrated_pointers.insert(reinterpret_cast<void**>(&pointer_to_be_migrated));
#endif

		typedef std::remove_pointer_t<std::decay_t<decltype(pointer_to_be_migrated)>> type;
		static_assert(!std::is_same_v<type, b2Joint>, "Can't migrate an abstract base class");

		void* const void_ptr = reinterpret_cast<void*>(pointer_to_be_migrated);

		if (pointer_to_be_migrated == nullptr) {
			return;
		}

		if (
			auto maybe_already_migrated = pointer_migrations.find(void_ptr);
			maybe_already_migrated == pointer_migrations.end()
		) {
			const auto bytes_count = std::size_t{ sizeof(type) * count };

			void* const migrated_pointer = migrated_allocator.Allocate(static_cast<int32>(bytes_count));
			std::memcpy(migrated_pointer, void_ptr, bytes_count);
			
			/* Bookmark position in memory of each and every element */

			pointer_migrations.insert(std::make_pair(
				void_ptr, 
				migrated_pointer
			));
			
			pointer_to_be_migrated = reinterpret_cast<type*>(migrated_pointer);
		}
		else {
			pointer_to_be_migrated = reinterpret_cast<type*>((*maybe_already_migrated).second);
		}
	};

	// migration of contacts and contact edges
	
	auto migrate_contact_edge = [
#if DEBUG_PHYSICS_SYSTEM_COPY
		&already_migrated_pointers,
#endif
		&pointer_migrations, 
		&contact_edge_a_or_b_in_contacts,
		contact_edge_a_offset,
		contact_edge_b_offset
	](b2ContactEdge*& edge_ptr) {
#if DEBUG_PHYSICS_SYSTEM_COPY
		ensure(already_migrated_pointers.find((void**)&edge_ptr) == already_migrated_pointers.end());
		already_migrated_pointers.insert((void**)&edge_ptr);
#endif
		if (edge_ptr == nullptr) {
			return;
		}

		const bool a_or_b_in_contact { contact_edge_a_or_b_in_contacts.at(edge_ptr) };
		const auto offset_to_edge_in_contact = std::size_t{ !a_or_b_in_contact ? contact_edge_a_offset : contact_edge_b_offset };

		std::byte* const contact_that_owns_unmigrated_edge = reinterpret_cast<std::byte*>(edge_ptr) - offset_to_edge_in_contact;
		// here "at" requires that the contacts be already migrated
		std::byte* const migrated_contact = reinterpret_cast<std::byte*>(pointer_migrations.at(contact_that_owns_unmigrated_edge));
		std::byte* const edge_from_migrated_contact = migrated_contact + offset_to_edge_in_contact;

		edge_ptr = reinterpret_cast<b2ContactEdge*>(edge_from_migrated_contact);
	};

	// make a map of pointers to b2ContactEdges to their respective offsets in
	// the b2Contacts that own them
	for (b2Contact* c = migrated_b2World.m_contactManager.m_contactList; c; c = c->m_next) {
		contact_edge_a_or_b_in_contacts.insert(std::make_pair(&c->m_nodeA, false));
		contact_edge_a_or_b_in_contacts.insert(std::make_pair(&c->m_nodeB, true));
	}

	// migrate contact pointers
	// contacts are polymorphic, but their derived classes do not add any member fields.
	// thus, it is safe to just memcpy sizeof(b2Contact)

	migrate_pointer(migrated_b2World.m_contactManager.m_contactList);

	for (b2Contact* c = migrated_b2World.m_contactManager.m_contactList; c; c = c->m_next) {
		migrate_pointer(c->m_prev);
		migrate_pointer(c->m_next);
		migrate_pointer(c->m_fixtureA);
		migrate_pointer(c->m_fixtureB);
		
		c->m_nodeA.contact = c;
		migrate_pointer(c->m_nodeA.other);

		c->m_nodeB.contact = c;
		migrate_pointer(c->m_nodeB.other);
	}

	// migrate contact edges of contacts
	for (b2Contact* c = migrated_b2World.m_contactManager.m_contactList; c; c = c->m_next) {
		migrate_contact_edge(c->m_nodeA.next);
		migrate_contact_edge(c->m_nodeA.prev);

		migrate_contact_edge(c->m_nodeB.next);
		migrate_contact_edge(c->m_nodeB.prev);
	}
	
	// migration of joints and joint edges

	auto migrate_joint_edge = [
#if DEBUG_PHYSICS_SYSTEM_COPY
		&already_migrated_pointers,
#endif
		&pointer_migrations,
		&joint_edge_a_or_b_in_joints,
		joint_edge_a_offset,
		joint_edge_b_offset
	](b2JointEdge*& edge_ptr) {
#if DEBUG_PHYSICS_SYSTEM_COPY
		ensure(already_migrated_pointers.find((void**)&edge_ptr) == already_migrated_pointers.end());
		already_migrated_pointers.insert((void**)&edge_ptr);
#endif
		if (edge_ptr == nullptr) {
			return;
		}

		const bool a_or_b_in_joint { joint_edge_a_or_b_in_joints.at(edge_ptr) };
		const auto offset_to_edge_in_joint = std::size_t { !a_or_b_in_joint ? joint_edge_a_offset : joint_edge_b_offset };

		std::byte* const joint_that_owns_unmigrated_edge = reinterpret_cast<std::byte*>(edge_ptr) - offset_to_edge_in_joint;
		// here "at" requires that the joints be already migrated
		std::byte* const migrated_joint = reinterpret_cast<std::byte*>(pointer_migrations.at(joint_that_owns_unmigrated_edge));
		std::byte* const edge_from_migrated_joint = migrated_joint + offset_to_edge_in_joint;

		edge_ptr = reinterpret_cast<b2JointEdge*>(edge_from_migrated_joint);
	};

	auto migrate_joint = [&migrate_pointer](b2Joint*& j){
		if (j == nullptr) {
			return;
		}

		dynamic_cast_dispatch<
			b2MotorJoint, // most likely

			b2DistanceJoint,
			b2FrictionJoint,
			b2GearJoint,
			b2MouseJoint,
			b2PrismaticJoint,
			b2PulleyJoint,
			b2RevoluteJoint,
			b2RopeJoint,
			b2WeldJoint,
			b2WheelJoint
		>(j, [&j, &migrate_pointer](auto* derived){
			using derived_type = std::remove_pointer_t<decltype(derived)>;
			// static_assert(std::is_same_v<derived_type, b2MotorJoint>, "test failed");
			migrate_pointer(reinterpret_cast<derived_type*&>(j));
		});
	};

	// make a map of pointers to b2JointEdges to their respective offsets in
	// the b2Joints that own them
	for (b2Joint* j = migrated_b2World.m_jointList; j; j = j->m_next) {
		joint_edge_a_or_b_in_joints.insert(std::make_pair(&j->m_edgeA, false));
		joint_edge_a_or_b_in_joints.insert(std::make_pair(&j->m_edgeB, true));
	}

	// migrate joint pointers
	migrate_joint(migrated_b2World.m_jointList);

	for (b2Joint* c = migrated_b2World.m_jointList; c; c = c->m_next) {
		migrate_joint(c->m_prev);
		migrate_joint(c->m_next);
		migrate_pointer(c->m_bodyA);
		migrate_pointer(c->m_bodyB);

		c->m_edgeA.joint = c;
		migrate_pointer(c->m_edgeA.other);

		c->m_edgeB.joint = c;
		migrate_pointer(c->m_edgeB.other);
	}

	// migrate joint edges of joints
	for (b2Joint* c = migrated_b2World.m_jointList; c; c = c->m_next) {
		migrate_joint_edge(c->m_edgeA.next);
		migrate_joint_edge(c->m_edgeA.prev);

		migrate_joint_edge(c->m_edgeB.next);
		migrate_joint_edge(c->m_edgeB.prev);
	}

	auto& proxy_tree = migrated_b2World.m_contactManager.m_broadPhase.m_tree;

	// migrate bodies and fixtures
	migrate_pointer(migrated_b2World.m_bodyList);

	for (b2Body* b = migrated_b2World.m_bodyList; b; b = b->m_next) {
		//auto rigid_body_cache_id = linear_cache_key(b->m_userData);
		//rigid_body_caches[rigid_body_cache_id].body = b;

		migrate_pointer(b->m_fixtureList);
		migrate_pointer(b->m_prev);
		migrate_pointer(b->m_next);
		migrate_pointer(b->m_ownerFrictionGround);

		migrate_contact_edge(b->m_contactList);
		migrate_joint_edge(b->m_jointList);
		b->m_world = &migrated_b2World;
		
		/*
			b->m_fixtureList is already migrated.
			f->m_next will also be always migrated before the next iteration
			thus f is always already a migrated instance.
		*/

		for (b2Fixture* f = b->m_fixtureList; f; f = f->m_next) {
			f->m_body = b;
			
			migrate_pointer(f->m_proxies, f->m_proxyCount);
			f->m_shape = f->m_shape->Clone(&migrated_allocator);
			migrate_pointer(f->m_next);

			for (std::size_t i = 0; i < f->m_proxyCount; ++i) {
#if DEBUG_PHYSICS_SYSTEM_COPY
				/* 
					"fixture" field of b2FixtureProxy should point to the fixture itself,
					thus its value should already be found in the pointer map. 
				*/

				ensure(pointer_migrations.find(f->m_proxies[i].fixture) != pointer_migrations.end())
				const auto ff = pointer_migrations[f->m_proxies[i].fixture];
				ensure_eq(reinterpret_cast<void*>(f), ff);
#endif
				f->m_proxies[i].fixture = f;
				
				void*& ud = proxy_tree.m_nodes[f->m_proxies[i].proxyId].userData;
				ud = pointer_migrations.at(ud);
			}
		}
	}

	/*
		There is no need to iterate userdatas of the broadphase's dynamic tree,
		as for every existing b2FixtureProxy we have manually migrated the correspondent userdata
		inside the loop that migrated all bodies and fixtures.
	*/

	colliders_caches = b.colliders_caches;
	rigid_body_caches = b.rigid_body_caches;

	for (auto& c : colliders_caches) {
		for (auto& f : c.all_fixtures_in_component) {
			f = reinterpret_cast<b2Fixture*>(pointer_migrations.at(reinterpret_cast<void*>(f)));
		}
	}
	
	for (auto& c : rigid_body_caches) {
		if (c.body) {
			c.body = reinterpret_cast<b2Body*>(pointer_migrations.at(reinterpret_cast<void*>(c.body)));
		}
	}

#if DEBUG_PHYSICS_SYSTEM_COPY
	// ensure that all allocations have been migrated

	ensure_eq(
		migrated_allocator.m_numAllocatedObjects, 
		source_b2World.m_blockAllocator.m_numAllocatedObjects
	);
#endif

	return *this;
}
