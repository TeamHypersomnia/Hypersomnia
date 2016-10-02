#include "physics_system.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/cosmos.h"

#include "game/components/item_component.h"
#include "game/components/driver_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/special_physics_component.h"

#include "game/messages/collision_message.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/new_entity_message.h"
#include "game/messages/will_soon_be_deleted.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/step.h"
#include "game/transcendental/entity_handle.h"

double METERS_TO_PIXELS = 100.0;
double PIXELS_TO_METERS = 1.0 / METERS_TO_PIXELS;
float METERS_TO_PIXELSf = 100.f;
float PIXELS_TO_METERSf = 1.0f / METERS_TO_PIXELSf;

bool physics_system::is_constructed_rigid_body(const_entity_handle handle) const {
	return handle.alive() && get_rigid_body_cache(handle).body != nullptr;
}

bool physics_system::is_constructed_colliders(const_entity_handle handle) const {
	return 
		handle.alive() // && is_constructed_rigid_body(handle.get<components::fixtures>().get_owner_body())
		&& 
		get_colliders_cache(handle).fixtures_per_collider.size()> 0 && 
		get_colliders_cache(handle).fixtures_per_collider[0].size() > 0;
}

rigid_body_cache& physics_system::get_rigid_body_cache(entity_id id) {
	return rigid_body_caches[id.pool.indirection_index];
}

colliders_cache& physics_system::get_colliders_cache(entity_id id) {
	return colliders_caches[id.pool.indirection_index];
}

const rigid_body_cache& physics_system::get_rigid_body_cache(entity_id id) const {
	return rigid_body_caches[id.pool.indirection_index];
}

const colliders_cache& physics_system::get_colliders_cache(entity_id id) const {
	return colliders_caches[id.pool.indirection_index];
}

void physics_system::destruct(const_entity_handle handle) {
	if (is_constructed_rigid_body(handle)) {
		auto& cache = get_rigid_body_cache(handle);
		
		for (auto& colliders_cache_id : cache.correspondent_colliders_caches)
			colliders_caches[colliders_cache_id] = colliders_cache();

		b2world->DestroyBody(cache.body);

		cache = rigid_body_cache();
	}
	
	if (is_constructed_colliders(handle)) {
		auto this_cache_id = handle.get_id().pool.indirection_index;
		auto& cache = colliders_caches[this_cache_id];

		ensure(cache.correspondent_rigid_body_cache != -1);

		auto& owner_body_cache = rigid_body_caches[cache.correspondent_rigid_body_cache];

		for (auto& f_per_c : cache.fixtures_per_collider)
			for (auto f : f_per_c)
				owner_body_cache.body->DestroyFixture(f);

		remove_element(owner_body_cache.correspondent_colliders_caches, this_cache_id);
		cache = colliders_cache();
	}
}

void physics_system::fixtures_construct(const_entity_handle handle) {
	//ensure(!is_constructed_colliders(handle));
	if (is_constructed_colliders(handle))
		return;

	if (handle.has<components::fixtures>()) {
		auto& colliders = handle.get<components::fixtures>();

		if (colliders.is_activated() && is_constructed_rigid_body(colliders.get_owner_body())) {
			auto& colliders_data = colliders.get_data();
			auto& cache = get_colliders_cache(handle);

			auto owner_body_entity = colliders.get_owner_body();
			ensure(owner_body_entity.alive());
			auto& owner_cache = get_rigid_body_cache(owner_body_entity);

			auto this_cache_id = handle.get_id().pool.indirection_index;
			auto owner_cache_id = owner_body_entity.get_id().pool.indirection_index;

			owner_cache.correspondent_colliders_caches.push_back(this_cache_id);
			cache.correspondent_rigid_body_cache = owner_cache_id;

			for (size_t ci = 0; ci < colliders_data.colliders.size(); ++ci) {
				const auto& c = colliders_data.colliders[ci];

				b2PolygonShape shape;

				b2FixtureDef fixdef;
				fixdef.density = c.density;
				fixdef.friction = c.friction;
				fixdef.isSensor = c.sensor;
				fixdef.filter = c.filter;
				fixdef.restitution = c.restitution;
				fixdef.shape = &shape;
				fixdef.userData = handle.get_id();

				auto transformed_shape = c.shape;
				transformed_shape.offset_vertices(colliders.get_total_offset());

				std::vector<b2Fixture*> partitioned_collider;

				for (auto convex : transformed_shape.convex_polys) {
					std::vector<b2Vec2> b2verts(convex.begin(), convex.end());

					for (auto& v : b2verts)
						v *= PIXELS_TO_METERSf;

					shape.Set(b2verts.data(), b2verts.size());

					b2Fixture* new_fix = owner_cache.body->CreateFixture(&fixdef);
					
					ensure(static_cast<short>(ci) < std::numeric_limits<short>::max());
					new_fix->collider_index = static_cast<short>(ci);

					partitioned_collider.push_back(new_fix);
				}

				cache.fixtures_per_collider.push_back(partitioned_collider);
			}
		}
	}
}

void physics_system::construct(const_entity_handle handle) {
	//ensure(!is_constructed_rigid_body(handle));
	if (is_constructed_rigid_body(handle))
		return;

	fixtures_construct(handle);

	if (handle.has<components::physics>()) {
		const auto& physics = handle.get<components::physics>();
		const auto& fixture_entities = physics.get_fixture_entities();

		if (physics.is_activated() && fixture_entities.size() > 0) {
			auto& physics_data = physics.get_data();
			auto& cache = get_rigid_body_cache(handle);

			b2BodyDef def;

			switch (physics_data.body_type) {
			case components::physics::type::DYNAMIC: def.type = b2BodyType::b2_dynamicBody; break;
			case components::physics::type::STATIC: def.type = b2BodyType::b2_staticBody; break;
			case components::physics::type::KINEMATIC: def.type = b2BodyType::b2_kinematicBody; break;
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
			for (const auto& f : fixture_entities)
				fixtures_construct(f);
		}
	}
}

void physics_system::reserve_caches_for_entities(size_t n) {
	rigid_body_caches.resize(n);
	colliders_caches.resize(n);
}

physics_system::physics_system() : 
b2world(new b2World(b2Vec2(0.f, 0.f))), ray_casts_since_last_step(0) {
	b2world->SetAllowSleeping(true);
	b2world->SetAutoClearForces(false);
}

void physics_system::post_and_clear_accumulated_collision_messages(fixed_step& step) {
	step.messages.post(accumulated_messages);
	accumulated_messages.clear();
}

physics_system& physics_system::contact_listener::get_sys() const {
	return cosm.systems_temporary.get<physics_system>();
}

physics_system::contact_listener::contact_listener(cosmos& cosm) : cosm(cosm) {
	get_sys().b2world->SetContactListener(this);
}

physics_system::contact_listener::~contact_listener() {
	get_sys().b2world->SetContactListener(nullptr);
}

void physics_system::step_and_set_new_transforms(fixed_step& step) {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();

	int32 velocityIterations = 8;
	int32 positionIterations = 3;

	ray_casts_since_last_step = 0;

	b2world->Step(static_cast<float32>(delta.in_seconds()), velocityIterations, positionIterations);

	post_and_clear_accumulated_collision_messages(step);

	for (b2Body* b = b2world->GetBodyList(); b != nullptr; b = b->GetNext()) {
		if (b->GetType() == b2_staticBody) continue;
		entity_handle entity = cosmos[b->GetUserData()];
		auto& physics = entity.get<components::physics>();

		recurential_friction_handler(step, b, b->m_ownerFrictionGround);

		physics.component.transform = b->m_xf;
		physics.component.sweep = b->m_sweep;
		physics.component.velocity = b->GetLinearVelocity();
		physics.component.angular_velocity = b->GetAngularVelocity();
	}
}

#define PROTECT_MIGRATIONS 0

physics_system& physics_system::operator=(const physics_system& b) {
	ray_casts_since_last_step = b.ray_casts_since_last_step;
	accumulated_messages = b.accumulated_messages;

	b2World& b2 = *b2world.get();
	b2.~b2World();
	new (&b2) b2World(b2Vec2(0.f, 0.f));

	const b2World& b2c = *b.b2world.get();

	// do the initial trivial copy
	b2 = b2c;

	// no need to install a new allocator since its operator= is a non-op
	// new (&b2.m_blockAllocator) b2BlockAllocator;

	// reset the allocator pointer to the new one
	b2.m_contactManager.m_allocator = &b2.m_blockAllocator;

	std::unordered_map<void*, void*> pointer_migrations;
	std::unordered_map<void*, size_t> m_nodeAB_offset;

	b2BlockAllocator& migrated_allocator = b2.m_blockAllocator;

#if PROTECT_MIGRATIONS
	std::unordered_set<void**> already_migrated;
	auto old_allocations = migrated_allocator.allocations;

	migrated_allocator.allocations.clear();
#endif

	auto migrate_pointer = [
#if PROTECT_MIGRATIONS
		&already_migrated, 
		&old_allocations,
#endif
		&pointer_migrations, &migrated_allocator](auto*& ptr, const unsigned num = 1) {
#if PROTECT_MIGRATIONS
		ensure(already_migrated.find((void**)&ptr) == already_migrated.end());
		already_migrated.insert((void**)&ptr);
#endif

		typedef std::remove_pointer<std::decay_t<decltype(ptr)>>::type type;

		if (ptr == nullptr) {
			return;
		}
		
		void* const p = reinterpret_cast<void*>(ptr);
		void* mig_p = nullptr;

		const size_t s = sizeof(type) * num;

		auto maybe_migrated = pointer_migrations.find(p);

		if (maybe_migrated == pointer_migrations.end()) {
			mig_p = migrated_allocator.Allocate(s);
			memcpy(mig_p, p, s);

			pointer_migrations.insert(std::make_pair(p, mig_p));

#if PROTECT_MIGRATIONS
			if(old_allocations.find(p) != old_allocations.end())
				old_allocations.erase(p);
#endif
		}
		else {
			mig_p = (*maybe_migrated).second;
		}

		ptr = reinterpret_cast<type*>(mig_p);
	};
	
	auto migrate_edge = [
#if PROTECT_MIGRATIONS
		&already_migrated,
#endif
		&pointer_migrations, &m_nodeAB_offset](b2ContactEdge*& ptr) {
#if PROTECT_MIGRATIONS
		ensure(already_migrated.find((void**)&ptr) == already_migrated.end());
		already_migrated.insert((void**)&ptr);
#endif
		if (ptr == nullptr) {
			return;
		}
		else {
			const size_t off = m_nodeAB_offset.at(ptr);
			ensure(off == offsetof(b2Contact, m_nodeA) || off == offsetof(b2Contact, m_nodeB));

			char* owner_contact = (char*)ptr - off;
			// here "at" requires that the contacts be already migrated
			char* remapped_contact = (char*)pointer_migrations.at(owner_contact);
			ptr = (b2ContactEdge*)(remapped_contact + off);
		}
	};

	// migrate proxy tree userdatas
	auto& proxy_tree = b2.m_contactManager.m_broadPhase.m_tree;

	//for (size_t i = 0; i < proxy_tree.m_nodeCount; ++i) {
	//	migrate_pointer(proxy_tree.m_nodes[i].userData);
	//}

	// acquire contact edge offsets
	for (b2Contact* c = b2.m_contactManager.m_contactList; c; c = c->m_next) {
		m_nodeAB_offset.insert(std::make_pair(&c->m_nodeA, offsetof(b2Contact, m_nodeA)));
		m_nodeAB_offset.insert(std::make_pair(&c->m_nodeB, offsetof(b2Contact, m_nodeB)));
	}

	// migrate contact pointers
	migrate_pointer(b2.m_contactManager.m_contactList);

	for (b2Contact* c = b2.m_contactManager.m_contactList; c; c = c->m_next) {
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
	for (b2Contact* c = b2.m_contactManager.m_contactList; c; c = c->m_next) {
		migrate_edge(c->m_nodeA.next);
		migrate_edge(c->m_nodeA.prev);

		migrate_edge(c->m_nodeB.next);
		migrate_edge(c->m_nodeB.prev);
	}

	// migrate bodies and fixtures
	migrate_pointer(b2.m_bodyList);

	for (b2Body* b = b2.m_bodyList; b; b = b->m_next)
	{
		//auto rigid_body_cache_id = b->m_userData.pool.indirection_index;
		//rigid_body_caches[rigid_body_cache_id].body = b;

		migrate_pointer(b->m_fixtureList);
		migrate_pointer(b->m_prev);
		migrate_pointer(b->m_next);
		migrate_pointer(b->m_jointList);
		migrate_pointer(b->m_ownerFrictionGround);

		migrate_edge(b->m_contactList);
		b->m_world = &b2;
		
		for (b2Fixture* f = b->m_fixtureList; f; f = f->m_next) {
			f->m_body = b;
			
			migrate_pointer(f->m_proxies, f->m_proxyCount);
			f->m_shape = f->m_shape->Clone(&migrated_allocator);
			migrate_pointer(f->m_next);

			for (size_t i = 0; i < f->m_proxyCount; ++i) {
				f->m_proxies[i].fixture = f;
				
				void*& ud = proxy_tree.m_nodes[f->m_proxies[i].proxyId].userData;
				ud = pointer_migrations.at(ud);
			}

			//auto c_idx = f->collider_index;
			//auto& cache = colliders_caches[f->m_userData.pool.indirection_index];
			//cache.correspondent_rigid_body_cache = rigid_body_cache_id;
			//
			//if (c_idx >= cache.fixtures_per_collider.size()) {
			//	ensure_eq(c_idx, cache.fixtures_per_collider.size());
			//	cache.fixtures_per_collider.push_back(std::vector<b2Fixture*>());
			//}
			//
			//cache.fixtures_per_collider[c_idx].push_back(f);
		}
	}

	colliders_caches = b.colliders_caches;
	rigid_body_caches = b.rigid_body_caches;

	for (auto& c : colliders_caches) {
		for (auto& fv : c.fixtures_per_collider) {
			for (auto& f : fv) {
				f = reinterpret_cast<b2Fixture*>(pointer_migrations.at(reinterpret_cast<void*>(f)));
			}
		}
	}
	
	for (auto& c : rigid_body_caches) {
		if (c.body) {
			c.body = reinterpret_cast<b2Body*>(pointer_migrations.at(reinterpret_cast<void*>(c.body)));
		}
	}

	// ensure that all allocations have been migrated
#if PROTECT_MIGRATIONS
	ensure(old_allocations.empty());
	ensure_eq(b2c.m_blockAllocator.allocations.size(), migrated_allocator.allocations.size());
#endif

	return *this;
}
