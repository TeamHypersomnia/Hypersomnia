#include <cstring>
#include <unordered_set>

#include "physics_world_cache.h"

#include "augs/build_settings/offsetof.h"

#include "game/components/item_component.h"
#include "game/components/driver_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/shape_polygon_component.h"
#include "game/components/shape_circle_component.h"

#include "game/components/rigid_body_component.h"
#include "game/components/transform_component.h"
#include "game/components/motor_joint_component.h"

#include "game/messages/collision_message.h"
#include "game/messages/queue_deletion.h"
#include "game/messages/will_soon_be_deleted.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/entity_handle.h"

#include "augs/templates/dynamic_cast_dispatch.h"
#include "augs/build_settings/setting_debug_physics_world_cache_copy.h"
#include "game/detail/entity_handle_mixins/get_owning_transfer_capability.hpp"
#include "game/enums/filters.h"
#include "game/inferred_caches/physics_world_cache.hpp"
#include "game/cosmos/for_each_entity.h"

rigid_body_cache* physics_world_cache::find_rigid_body_cache(const entity_id id) {
	return mapped_or_nullptr(rigid_body_caches, id);
}

colliders_cache* physics_world_cache::find_colliders_cache(const entity_id id) {
	return mapped_or_nullptr(colliders_caches, id);
}

joint_cache* physics_world_cache::find_joint_cache(const entity_id id) {
	return mapped_or_nullptr(joint_caches, id);
}

const rigid_body_cache* physics_world_cache::find_rigid_body_cache(const entity_id id) const {
	return mapped_or_nullptr(rigid_body_caches, id);
}

const colliders_cache* physics_world_cache::find_colliders_cache(const entity_id id) const {
	return mapped_or_nullptr(colliders_caches, id);
}

const joint_cache* physics_world_cache::find_joint_cache(const entity_id id) const {
	return mapped_or_nullptr(joint_caches, id);
}

void rigid_body_cache::clear(physics_world_cache& owner) {
	if (body == nullptr) {
		return;
	}

	for (const b2Fixture* f = body->m_fixtureList; f != nullptr; f = f->m_next) {
		owner.colliders_caches.erase(f->GetUserData());
	}

	for (const b2JointEdge* j = body->m_jointList; j != nullptr; j = j->next) {
		owner.joint_caches.erase(j->joint->GetUserData());
	}

	// no need to manually destroy each fixture and joint of the body,
	// Box2D will take care of that after just deleting the body.

	owner.b2world->DestroyBody(body);
	body = nullptr;
}

void colliders_cache::clear(physics_world_cache& owner) {
	auto owner_body_cache = owner.find_rigid_body_cache(connection.owner);

	for (b2Fixture* f : constructed_fixtures) {
		owner_body_cache->body->DestroyFixture(f);
	}

	constructed_fixtures.clear();
}

void joint_cache::clear(physics_world_cache& owner) {
	owner.b2world->DestroyJoint(joint);
	joint = nullptr;
}

void physics_world_cache::destroy_rigid_body_cache(const const_entity_handle& handle) {
	if (auto cache = find_rigid_body_cache(handle)) {
		cache->clear(*this);
		rigid_body_caches.erase(handle);
	}
}

void physics_world_cache::destroy_colliders_cache(const const_entity_handle& handle) {
	if (auto cache = find_colliders_cache(handle)) {
		cache->clear(*this);
		colliders_caches.erase(handle);
	}
}

void physics_world_cache::destroy_joint_cache(const const_entity_handle& handle) {
	if (auto cache = find_joint_cache(handle)) {
		cache->clear(*this);
		joint_caches.erase(handle);
	}
}

void physics_world_cache::destroy_cache_of(const const_entity_handle& handle) {
	destroy_rigid_body_cache(handle);
	destroy_colliders_cache(handle);
	destroy_joint_cache(handle);
}

void physics_world_cache::infer_cache_for(const const_entity_handle& handle) {
	handle.dispatch(
		[this](const auto& typed_handle) {
			specific_infer_cache_for(typed_handle);
		}
	);
}

void physics_world_cache::infer_colliders(const const_entity_handle& handle) {
	handle.dispatch_on_having_all<invariants::fixtures>([this](const auto& typed_handle) {
		specific_infer_colliders(typed_handle);
	});
}

void physics_world_cache::infer_rigid_body(const const_entity_handle& handle) {
	handle.dispatch_on_having_all<invariants::rigid_body>([this](const auto& typed_handle) {
		specific_infer_rigid_body(typed_handle);
	});
}

void physics_world_cache::infer_joint(const const_entity_handle&) {
#if TODO_JOINTS
#endif
}

void physics_world_cache::infer_colliders_from_scratch(const const_entity_handle& handle) {
	handle.dispatch_on_having_all<invariants::fixtures>([this](const auto& typed_handle) {
		if (const auto connection = typed_handle.calc_colliders_connection()) {
			specific_infer_colliders_from_scratch(typed_handle, *connection);
		}
		else {
			specific_infer_colliders_from_scratch(typed_handle, colliders_connection());
		}
	});
}

void physics_world_cache::infer_all(const cosmos& cosm) {
	cosm.for_each_having<invariants::rigid_body>([this](const auto& typed_handle) {
		specific_infer_rigid_body(typed_handle);
	});

	cosm.for_each_having<invariants::fixtures>([this](const auto& typed_handle) {
		if (const auto connection = typed_handle.calc_colliders_connection()) {
			specific_infer_colliders_from_scratch(typed_handle, *connection);
		}
		else {
			specific_infer_colliders_from_scratch(typed_handle, colliders_connection());
		}
	});
}

void physics_world_cache::reserve_caches_for_entities(const size_t n) {
	rigid_body_caches.reserve(n);
	colliders_caches.reserve(n);
	joint_caches.reserve(n);
}

b2Fixture_index_in_component physics_world_cache::get_index_in_component(
	const b2Fixture* const f, 
	const const_entity_handle& handle
) const {
	ensure(f->index_in_component != -1);

	b2Fixture_index_in_component result;
	result.convex_shape_index = static_cast<std::size_t>(f->index_in_component);

	ensure_eq(f, find_colliders_cache(handle.get_id())->constructed_fixtures[result.convex_shape_index].get());

	return result;
}

physics_world_cache::physics_world_cache() :
	b2world(new b2World(b2Vec2(0.f, 0.f))) 
{
	b2world->SetAllowSleeping(true);
	b2world->SetAutoClearForces(false);
}

physics_world_cache::physics_world_cache(const physics_world_cache& b) : physics_world_cache() {
	*this = b;
}

physics_world_cache& physics_world_cache::operator=(const physics_world_cache& b) {
	ray_cast_counter = b.ray_cast_counter;
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

	std::unordered_map<const void*, void*> pointer_migrations;
	std::unordered_map<const void*, bool> contact_edge_a_or_b_in_contacts;
	std::unordered_map<const void*, bool> joint_edge_a_or_b_in_joints;

	b2BlockAllocator& migrated_allocator = migrated_b2World.m_blockAllocator;

	const auto contact_edge_a_offset = augs_offsetof(b2Contact, m_nodeA);
	const auto contact_edge_b_offset = augs_offsetof(b2Contact, m_nodeB);

	const auto joint_edge_a_offset = augs_offsetof(b2Joint, m_edgeA);
	const auto joint_edge_b_offset = augs_offsetof(b2Joint, m_edgeB);

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

		using type = std::remove_pointer_t<std::remove_reference_t<decltype(pointer_to_be_migrated)>>;
		static_assert(!std::is_same_v<type, b2Joint>, "Can't migrate an abstract base class");

		const auto void_ptr = reinterpret_cast<const void*>(pointer_to_be_migrated);

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

				{
					const auto ff = pointer_migrations[f->m_proxies[i].fixture];
					ensure_eq(reinterpret_cast<void*>(f), ff);
				}
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

	colliders_caches.clear();
	rigid_body_caches.clear();

	colliders_caches.reserve(b.colliders_caches.size());
	rigid_body_caches.reserve(b.rigid_body_caches.size());

	for (const auto& it : b.colliders_caches) {
		auto& migrated_cache = colliders_caches[it.first];

		migrated_cache = it.second;
		migrated_cache.constructed_fixtures.clear();

		for (const auto& f : it.second.constructed_fixtures) {
			migrated_cache.constructed_fixtures.emplace_back(
				reinterpret_cast<b2Fixture*>(pointer_migrations.at(reinterpret_cast<const void*>(f.get())))
			);
		}
	}
	
	for (const auto& it : b.rigid_body_caches) {
		const auto b_body = it.second.body.get();

		auto& migrated_cache = rigid_body_caches[it.first];
		static_assert(sizeof(migrated_cache) == sizeof(augs::propagate_const<b2Body*>));

#if WHEN_MORE_FIELDS_ADDED
		migrated_cache = it.second;
#endif

		if (b_body) {
			migrated_cache.body = reinterpret_cast<b2Body*>(pointer_migrations.at(reinterpret_cast<const void*>(b_body)));
		}
	}

#if TODO
	joint_caches.clear();
	joint_caches.reserve(b.joint_caches.size());

	for (auto& it : joint_caches) {
		const auto b_joint = b.joint_caches[it.first].joint.get();

		if (b_joint) {
			joint_caches[i].joint = reinterpret_cast<b2Joint*>(pointer_migrations.at(reinterpret_cast<const void*>(b_joint)));
		}
	}
#endif

#if DEBUG_PHYSICS_SYSTEM_COPY
	// ensure that all allocations have been migrated

	ensure_eq(
		migrated_allocator.m_numAllocatedObjects, 
		source_b2World.m_blockAllocator.m_numAllocatedObjects
	);
#endif

	return *this;
}
