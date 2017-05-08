#include "physics_system.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/cosmos.h"

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

#include "augs/graphics/renderer.h"
#include "augs/templates/container_templates.h"
#include "augs/build_settings/setting_debug_physics_system_copy.h"

bool physics_system::is_inferred_state_created_for_rigid_body(const const_entity_handle handle) const {
	return handle.alive() && get_rigid_body_cache(handle).body != nullptr;
}

bool physics_system::is_inferred_state_created_for_colliders(const const_entity_handle handle) const {
	return
		handle.alive() // && is_inferred_state_created_for_rigid_body(handle.get_owner_body())
		&& get_colliders_cache(handle).all_fixtures_in_component.size() > 0
	;
}

rigid_body_cache& physics_system::get_rigid_body_cache(const entity_id id) {
	return rigid_body_caches[make_cache_id(id)];
}

colliders_cache& physics_system::get_colliders_cache(const entity_id id) {
	return colliders_caches[make_cache_id(id)];
}

const rigid_body_cache& physics_system::get_rigid_body_cache(const entity_id id) const {
	return rigid_body_caches[make_cache_id(id)];
}

const colliders_cache& physics_system::get_colliders_cache(const entity_id id) const {
	return colliders_caches[make_cache_id(id)];
}

void physics_system::destroy_inferred_state_of(const const_entity_handle handle) {
	if (is_inferred_state_created_for_rigid_body(handle)) {
		auto& cache = get_rigid_body_cache(handle);
		
		for (const auto& colliders_cache_id : cache.correspondent_colliders_caches) {
			colliders_caches[colliders_cache_id] = colliders_cache();
		}

		b2world->DestroyBody(cache.body);

		cache = rigid_body_cache();
	}
	
	if (is_inferred_state_created_for_colliders(handle)) {
		const auto this_cache_id = make_cache_id(handle);
		auto& cache = colliders_caches[this_cache_id];

		ensure(cache.correspondent_rigid_body_cache != -1);

		auto& owner_body_cache = rigid_body_caches[cache.correspondent_rigid_body_cache];

		for (const auto& f : cache.all_fixtures_in_component) {
			owner_body_cache.body->DestroyFixture(f);
		}

		remove_element(owner_body_cache.correspondent_colliders_caches, this_cache_id);
		cache = colliders_cache();
	}
}

void physics_system::create_inferred_state_for_fixtures(const const_entity_handle handle) {
	//ensure(!is_inferred_state_created_for_colliders(handle));
	const bool is_already_constructed = is_inferred_state_created_for_colliders(handle);

	if (is_already_constructed) {
		return;
	}

	const auto colliders = handle.find<components::fixtures>();

	const bool is_anything_to_construct =
		colliders != nullptr
		&& colliders.is_activated()
		&& is_inferred_state_created_for_rigid_body(handle.get_owner_body())
	;

	if (is_anything_to_construct) {
		const auto si = handle.get_cosmos().get_si();
		const auto& group = colliders.get_data();
		auto& cache = get_colliders_cache(handle);

		const auto owner_body_entity = handle.get_owner_body();
		ensure(owner_body_entity.alive());
		auto& owner_cache = get_rigid_body_cache(owner_body_entity);

		const auto this_cache_id = make_cache_id(handle);
		const auto owner_cache_id = make_cache_id(owner_body_entity);

		owner_cache.correspondent_colliders_caches.push_back(this_cache_id);
		cache.correspondent_rigid_body_cache = owner_cache_id;

		b2FixtureDef fixdef;
		fixdef.density = group.density;
		fixdef.friction = group.friction;
		fixdef.isSensor = group.sensor;
		fixdef.filter = group.filter;
		fixdef.restitution = group.restitution;
		fixdef.userData = handle.get_id();

		auto& all_fixtures_in_component = cache.all_fixtures_in_component;
		all_fixtures_in_component.clear();
		
		const auto shape_polygon = handle.find<components::shape_polygon>();

		if (shape_polygon != nullptr && shape_polygon.is_activated()) {
			auto transformed_shape = shape_polygon.get_raw_component().shape;
			transformed_shape.offset_vertices(colliders.get_total_offset());

			for (std::size_t ci = 0; ci < transformed_shape.convex_polys.size(); ++ci) {
				const auto& convex = transformed_shape.convex_polys[ci];
				std::vector<b2Vec2> b2verts(convex.vertices.begin(), convex.vertices.end());

				for (auto& v : b2verts) {
					v = si.get_meters(v);
				}

				b2PolygonShape shape;
				shape.Set(b2verts.data(), b2verts.size());

				fixdef.shape = &shape;
				b2Fixture* const new_fix = owner_cache.body->CreateFixture(&fixdef);

				ensure(static_cast<short>(ci) < std::numeric_limits<short>::max());
				new_fix->index_in_component = static_cast<short>(ci);

				all_fixtures_in_component.push_back(new_fix);
			}

			return;
		}
		
		const auto shape_circle = handle.find<components::shape_circle>();
		
		if (shape_circle != nullptr && shape_circle.is_activated()) {
			b2CircleShape shape;
			shape.m_radius = si.get_meters(shape_circle.get_radius());
		
			fixdef.shape = &shape;
			b2Fixture* const new_fix = owner_cache.body->CreateFixture(&fixdef);
			
			new_fix->index_in_component = 0u;
			all_fixtures_in_component.push_back(new_fix);
			
			return;
		}
		
		ensure(false && "fixtures requested with no shape attached!");
	}
}

void physics_system::create_inferred_state_for(const const_entity_handle handle) {
	const auto& cosmos = handle.get_cosmos();

	//ensure(!is_inferred_state_created_for_rigid_body(handle));
	const bool is_already_constructed = is_inferred_state_created_for_rigid_body(handle);

	if (is_already_constructed) {
		return;
	}

	create_inferred_state_for_fixtures(handle);

	const auto rigid_body = handle.find<components::rigid_body>();
	
	const bool is_anything_to_construct =
		rigid_body != nullptr
		&& rigid_body.is_activated()
		&& rigid_body.get_fixture_entities().size() > 0
	;

	if (is_anything_to_construct) {
		const auto fixture_entities = rigid_body.get_fixture_entities();

		const auto& physics_data = rigid_body.get_data();
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
		def.gravityScale = physics_data.gravity_scale;
		def.active = true;
		def.linearVelocity = physics_data.velocity;
		def.angularVelocity = physics_data.angular_velocity;

		cache.body = b2world->CreateBody(&def);
		cache.body->SetAngledDampingEnabled(physics_data.angled_damping);
		
		/* notice that all fixtures must be unconstructed at this moment since we assert that the rigid body itself is not */
		for (const auto f : fixture_entities) {
			create_inferred_state_for_fixtures(cosmos[f]);
		}
	}
}

void physics_system::reserve_caches_for_entities(const size_t n) {
	rigid_body_caches.resize(n);
	colliders_caches.resize(n);
	joint_caches.resize(n);
}

b2Fixture_index_in_component physics_system::get_index_in_component(
	const b2Fixture* const f, 
	const const_entity_handle handle
) {
	ensure(f->index_in_component != -1);

	b2Fixture_index_in_component result;
	result.convex_shape_index = static_cast<std::size_t>(f->index_in_component);

	ensure_eq(f, colliders_caches[make_cache_id(handle)].all_fixtures_in_component[result.convex_shape_index]);

	return result;

	//const auto this_cache_id = make_cache_id(handle);
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

physics_system::physics_system() : 
b2world(new b2World(b2Vec2(0.f, 0.f))), ray_casts_since_last_step(0) {
	b2world->SetAllowSleeping(true);
	b2world->SetAutoClearForces(false);
}

void physics_system::post_and_clear_accumulated_collision_messages(const logic_step step) {
	step.transient.messages.post(accumulated_messages);
	accumulated_messages.clear();
}

physics_system& physics_system::contact_listener::get_sys() const {
	return cosm.systems_inferred.get<physics_system>();
}

physics_system::contact_listener::contact_listener(cosmos& cosm) : cosm(cosm) {
	get_sys().b2world->SetContactListener(this);
}

physics_system::contact_listener::~contact_listener() {
	get_sys().b2world->SetContactListener(nullptr);
}

void physics_system::step_and_set_new_transforms(const logic_step step) {
	auto& cosmos = step.cosm;
	const auto delta = step.get_delta();

	int32 velocityIterations = 8;
	int32 positionIterations = 3;

	ray_casts_since_last_step = 0;

	b2world->Step(static_cast<float32>(delta.in_seconds()), velocityIterations, positionIterations);

	post_and_clear_accumulated_collision_messages(step);

	for (b2Body* b = b2world->GetBodyList(); b != nullptr; b = b->GetNext()) {
		if (b->GetType() == b2_staticBody) continue;
		entity_handle entity = cosmos[b->GetUserData()];
		auto& rigid_body = entity.get<components::rigid_body>();

		recurential_friction_handler(step, b, b->m_ownerFrictionGround);

		rigid_body.get_data().transform = b->m_xf;
		rigid_body.get_data().sweep = b->m_sweep;
		rigid_body.get_data().velocity = b->GetLinearVelocity();
		rigid_body.get_data().angular_velocity = b->GetAngularVelocity();
	}
}

physics_system& physics_system::operator=(const physics_system& b) {
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
	std::unordered_map<void*, size_t> contact_edge_offsets_in_contacts;

	b2BlockAllocator& migrated_allocator = migrated_b2World.m_blockAllocator;

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
		
		void* const void_ptr = reinterpret_cast<void*>(pointer_to_be_migrated);

		if (pointer_to_be_migrated == nullptr) {
			return;
		}

		auto maybe_already_migrated = pointer_migrations.find(void_ptr);

		if (maybe_already_migrated == pointer_migrations.end()) {
			const size_t bytes_count = sizeof(type) * count;

			void* const migrated_pointer = migrated_allocator.Allocate(bytes_count);
			memcpy(migrated_pointer, void_ptr, bytes_count);
			
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
	
	auto migrate_contact_edge = [
#if DEBUG_PHYSICS_SYSTEM_COPY
		&already_migrated_pointers,
#endif
		&pointer_migrations, 
		&contact_edge_offsets_in_contacts
	](b2ContactEdge*& edge_ptr) {
#if DEBUG_PHYSICS_SYSTEM_COPY
		ensure(already_migrated_pointers.find((void**)&edge_ptr) == already_migrated_pointers.end());
		already_migrated_pointers.insert((void**)&edge_ptr);
#endif
		if (edge_ptr == nullptr) {
			return;
		}
		else {
			const size_t offset_to_edge_in_contact = contact_edge_offsets_in_contacts.at(edge_ptr);

			ensure(
				offset_to_edge_in_contact == offsetof(b2Contact, m_nodeA) 
				|| offset_to_edge_in_contact == offsetof(b2Contact, m_nodeB)
			);

			char* const contact_that_owns_unmigrated_edge = reinterpret_cast<char*>(edge_ptr) - offset_to_edge_in_contact;
			// here "at" requires that the contacts be already migrated
			char* const migrated_contact = reinterpret_cast<char*>(pointer_migrations.at(contact_that_owns_unmigrated_edge));
			char* const edge_from_migrated_contact = migrated_contact + offset_to_edge_in_contact;

			edge_ptr = reinterpret_cast<b2ContactEdge*>(edge_from_migrated_contact);
		}
	};

	// make a map of pointers to b2ContactEdges to their respective offsets in
	// the b2Contacts that own them
	for (b2Contact* c = migrated_b2World.m_contactManager.m_contactList; c; c = c->m_next) {
		contact_edge_offsets_in_contacts.insert(std::make_pair(&c->m_nodeA, offsetof(b2Contact, m_nodeA)));
		contact_edge_offsets_in_contacts.insert(std::make_pair(&c->m_nodeB, offsetof(b2Contact, m_nodeB)));
	}

	// migrate contact pointers
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

	auto& proxy_tree = migrated_b2World.m_contactManager.m_broadPhase.m_tree;

	// migrate bodies and fixtures
	migrate_pointer(migrated_b2World.m_bodyList);

	for (b2Body* b = migrated_b2World.m_bodyList; b; b = b->m_next)
	{
		//auto rigid_body_cache_id = make_cache_id(b->m_userData);
		//rigid_body_caches[rigid_body_cache_id].body = b;

		migrate_pointer(b->m_fixtureList);
		migrate_pointer(b->m_prev);
		migrate_pointer(b->m_next);
		migrate_pointer(b->m_jointList);
		migrate_pointer(b->m_ownerFrictionGround);

		migrate_contact_edge(b->m_contactList);
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
