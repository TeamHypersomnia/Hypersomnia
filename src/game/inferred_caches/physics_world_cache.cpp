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
#include "game/enums/filters.h"

template <class E>
auto calc_filters(const E& handle) {
	const auto& colliders_data = handle.template get<invariants::fixtures>();

	if (handle.is_like_planted_bomb()) {
		return filters::planted_explosive();
	}

	return colliders_data.filter;
}

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

	for (b2Fixture* f : all_fixtures_in_component) {
		owner_body_cache->body->DestroyFixture(f);
	}

	all_fixtures_in_component.clear();
}

void joint_cache::clear(physics_world_cache& owner) {
	owner.b2world->DestroyJoint(joint);
	joint = nullptr;
}

void physics_world_cache::destroy_rigid_body_cache(const const_entity_handle handle) {
	if (auto cache = find_rigid_body_cache(handle)) {
		cache->clear(*this);
		rigid_body_caches.erase(handle);
	}
}

void physics_world_cache::destroy_colliders_cache(const const_entity_handle handle) {
	if (auto cache = find_colliders_cache(handle)) {
		cache->clear(*this);
		colliders_caches.erase(handle);
	}
}

void physics_world_cache::destroy_joint_cache(const const_entity_handle handle) {
	if (auto cache = find_joint_cache(handle)) {
		cache->clear(*this);
		joint_caches.erase(handle);
	}
}

void physics_world_cache::destroy_cache_of(const const_entity_handle handle) {
	destroy_rigid_body_cache(handle);
	destroy_colliders_cache(handle);
	destroy_joint_cache(handle);
}

void physics_world_cache::infer_rigid_body(const const_entity_handle h) {
	h.dispatch_on_having_all<components::rigid_body>([this](const auto handle) {
		const auto it = rigid_body_caches.try_emplace(unversioned_entity_id(handle));
		auto& cache = (*it.first).second;
	
		const auto& physics_def = handle.template get<invariants::rigid_body>();

		auto to_b2Body_type = [](const rigid_body_type t) {
			switch (t) {
				case rigid_body_type::DYNAMIC: return b2BodyType::b2_dynamicBody;
				case rigid_body_type::STATIC: return b2BodyType::b2_staticBody;
				case rigid_body_type::KINEMATIC: return b2BodyType::b2_kinematicBody;
				default: return b2BodyType::b2_staticBody;
			}
		};

		const auto body_type = [&]() {
			return handle.is_like_planted_bomb() ? rigid_body_type::STATIC : physics_def.body_type;
		}();

		if (!it.second) {
			/* The cache already existed. */
			auto& body = *cache.body;

			bool only_update_properties = true;

			if (to_b2Body_type(body_type) != body.GetType()) {
				only_update_properties = false;
			}

			if (only_update_properties) {
				/* 
					Invariant/component guaranteed to exist because it must have once been created from an existing def,
					and changing type content implies reinference of the entire cosmos.
				*/
		
				const auto& def = handle.template get<invariants::rigid_body>();
				const auto rigid_body = handle.template get<components::rigid_body>();
				const auto damping = rigid_body.calc_damping_mults(def);
				const auto& data = rigid_body.get_raw_component();
		
				/* 
					Currently, nothing that can change inside the component could possibly trigger the need to rebuild the body.
					This may change once we want to delete bodies without fixtures.
				*/
		
				/* These have no side-effects */
				body.SetLinearDamping(damping.linear);
				body.SetAngularDamping(damping.angular);
				body.SetLinearDampingVec(b2Vec2(damping.linear_axis_aligned));
				body.SetAngledDampingEnabled(def.angled_damping);
		
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
	
		/*
			Here the cache is not constructed so we rebuild from scratch.
		*/
		cache.clear(*this);
	
		const auto rigid_body = handle.template get<components::rigid_body>();
		const auto& physics_data = rigid_body.get_raw_component();

		b2BodyDef def;
		def.type = to_b2Body_type(body_type);

		def.userData = unversioned_entity_id(handle);

		def.bullet = physics_def.bullet;
		def.allowSleep = physics_def.allow_sleep;

		const auto damping = rigid_body.calc_damping_mults(physics_def);

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

		cache.body->SetAngledDampingEnabled(physics_def.angled_damping);
		cache.body->SetLinearDampingVec(b2Vec2(damping.linear_axis_aligned));
	
		/*
			All colliders caches, before their own inference,
			manually infer the existence of the rigid body.

			Thus the rigid body, on its own inference, does not have to inform all fixtures
			about that it has just come into existence.
		*/
	});
}

void physics_world_cache::infer_cache_for(const const_entity_handle handle) {
	infer_rigid_body(handle);
	infer_colliders(handle);
	infer_joint(handle);
}

void physics_world_cache::infer_colliders(const const_entity_handle h) {
	h.dispatch_on_having_all<invariants::fixtures>([this](const auto handle) {
		std::optional<colliders_connection> calculated_connection;
	
		auto get_calculated_connection = [&](){
			if (!calculated_connection) {
				if (const auto connection = handle.calc_colliders_connection()) {
					calculated_connection = connection; 
				}
				else {
					/* A default with unset owner body will prevent cache from building */
					calculated_connection = colliders_connection();
				}
			}
	
			return *calculated_connection;
		};
	
		const auto it = colliders_caches.try_emplace(handle.get_id().to_unversioned());
		auto& cache = (*it.first).second;
	
		if (!it.second) {
			/* Cache already existed. */
			bool only_update_properties = true;
			
			if (get_calculated_connection() != cache.connection) {
				only_update_properties = false;
			}

			if (cache.all_fixtures_in_component.empty()) {
				only_update_properties = false;
			}
	
			if (only_update_properties) {
				auto& compared = *cache.all_fixtures_in_component[0].get();
				const auto& colliders_data = handle.template get<invariants::fixtures>();
	
				if (const auto new_density = handle.calc_density(
						get_calculated_connection(), 
						colliders_data
					);
	
					compared.GetDensity() != new_density
				) {
					for (auto& f : cache.all_fixtures_in_component) {
						f.get()->SetDensity(new_density);
					}
	
					compared.GetBody()->ResetMassData();
				}
	
				const auto chosen_filters = calc_filters(handle);
				const bool rebuild_filters = compared.GetFilterData() != chosen_filters;
	
				for (auto& f : cache.all_fixtures_in_component) {
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

		infer_colliders_from_scratch(handle, get_calculated_connection());
	});
}

void physics_world_cache::infer_colliders_from_scratch(const const_entity_handle h, const colliders_connection& connection) {
	h.dispatch_on_having_all<invariants::fixtures>([this, connection](const auto handle) {
		const auto& cosmos = handle.get_cosmos();
	
		const auto it = colliders_caches.try_emplace(handle.get_id().to_unversioned());

		auto& cache = (*it.first).second;
		cache.clear(*this);

		const auto new_owner = cosmos[connection.owner];

		if (new_owner.dead()) {
			colliders_caches.erase(it.first);
			return;
		}

		infer_rigid_body_existence(new_owner);

		const auto body_cache = find_rigid_body_cache(new_owner);

		if (!body_cache) {
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

		cache.connection = connection;

		auto& all_fixtures_in_component = cache.all_fixtures_in_component;
		ensure(all_fixtures_in_component.empty());

		auto from_polygon_shape = [&](auto shape) {
			shape.offset_vertices(connection.shape_offset);

			for (std::size_t ci = 0; ci < shape.convex_polys.size(); ++ci) {
				const auto& convex = shape.convex_polys[ci];
				std::vector<b2Vec2> b2verts(convex.begin(), convex.end());

				for (auto& v : b2verts) {
					v = si.get_meters(v);
				}

				b2PolygonShape shape;
				shape.Set(b2verts.data(), static_cast<int32>(b2verts.size()));

				fixdef.shape = &shape;
				b2Fixture* const new_fix = owner_b2Body.CreateFixture(&fixdef);

				ensure(static_cast<short>(ci) < std::numeric_limits<short>::max());
				new_fix->index_in_component = static_cast<short>(ci);

				all_fixtures_in_component.emplace_back(new_fix);
			}
		};

		if (const auto* const shape_polygon = handle.template find<invariants::shape_polygon>()) {
			from_polygon_shape(shape_polygon->shape);
			return;
		}

		// TODO: let shape circle be chosen over the polygon shape when grenade is thrown
		// can either have a separate definition in the hand fuse invariant or assume that the entity will have it

		if (const auto shape_circle = handle.template find<invariants::shape_circle>()) {
			b2CircleShape shape;
			shape.m_radius = si.get_meters(shape_circle->get_radius());

			fixdef.shape = &shape;
			b2Fixture* const new_fix = owner_b2Body.CreateFixture(&fixdef);

			new_fix->index_in_component = 0u;
			all_fixtures_in_component.emplace_back(new_fix);

			return;
		}

		if (const auto* const sprite = handle.template find<invariants::sprite>()) {
			auto size = handle.get_logical_size();

			size.x = std::max(1.f, size.x);
			size.y = std::max(1.f, size.y);

			from_polygon_shape(convex_partitioned_shape::from_box(size));

			return;
		}

		ensure(false && "fixtures requested with no shape attached!");
	});
}

void physics_world_cache::infer_colliders_from_scratch(const const_entity_handle handle) {
	if (const auto connection = handle.calc_colliders_connection()) {
		infer_colliders_from_scratch(handle, *connection);
	}
	else {
		infer_colliders_from_scratch(handle, colliders_connection());
	}
}

void physics_world_cache::infer_joint(const const_entity_handle /* handle */) {
#if TODO
	const auto& cosmos = handle.get_cosmos();

	if (const auto motor_joint = handle.find<components::motor_joint>();

		motor_joint != nullptr
		&& rigid_body_cache_exists_for(cosmos[motor_joint.get_target_bodies()[0]])
		&& rigid_body_cache_exists_for(cosmos[motor_joint.get_target_bodies()[1]])
	) {
		const components::motor_joint joint_data = motor_joint.get_raw_component();

		const auto si = handle.get_cosmos().get_si();
		auto cache = find_joint_cache(handle);
		
		ensure_eq(nullptr, cache->joint);

		b2MotorJointDef def;
		def.userData = handle.get_id();
		def.bodyA = find_rigid_body_cache(cosmos[joint_data.target_bodies[0]]).body;
		def.bodyB = find_rigid_body_cache(cosmos[joint_data.target_bodies[1]]).body;
		def.collideConnected = joint_data.collide_connected;
		def.linearOffset = b2Vec2(si.get_meters(joint_data.linear_offset));
		def.angularOffset = DEG_TO_RAD<float> * joint_data.angular_offset;
		def.maxForce = si.get_meters(joint_data.max_force);
		def.maxTorque = joint_data.max_torque;
		def.correctionFactor = joint_data.correction_factor;

		cache->joint = b2world->CreateJoint(&def);
	}
#endif
}

void physics_world_cache::reserve_caches_for_entities(const size_t n) {
	rigid_body_caches.reserve(n);
	colliders_caches.reserve(n);
	joint_caches.reserve(n);
}

b2Fixture_index_in_component physics_world_cache::get_index_in_component(
	const b2Fixture* const f, 
	const const_entity_handle handle
) const {
	ensure(f->index_in_component != -1);

	b2Fixture_index_in_component result;
	result.convex_shape_index = static_cast<std::size_t>(f->index_in_component);

	ensure_eq(f, find_colliders_cache(handle.get_id())->all_fixtures_in_component[result.convex_shape_index].get());

	return result;
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
		ensure_eq(already_migrated_pointers.find(reinterpret_cast<void**>(&pointer_to_be_migrated)), already_migrated_pointers.end());
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
		ensure_eq(already_migrated_pointers.find((void**)&edge_ptr), already_migrated_pointers.end());
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
		ensure_eq(already_migrated_pointers.find((void**)&edge_ptr), already_migrated_pointers.end());
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

	ensure(false);

	colliders_caches.clear();
	rigid_body_caches.clear();
	joint_caches.clear();
#if TODO

	colliders_caches.reserve(b.colliders_caches.size());
	rigid_body_caches.reserve(b.rigid_body_caches.size());
	joint_caches.reserve(b.joint_caches.size());

	for (auto& it : colliders_caches) {
		for (auto& f : b.colliders_caches[it.first].all_fixtures_in_component) {
			colliders_caches[i].all_fixtures_in_component.emplace_back(
				reinterpret_cast<b2Fixture*>(pointer_migrations.at(reinterpret_cast<const void*>(f.get())))
			);
		}
	}
	
	for (auto& it : b.rigid_body_caches) {
		const auto b_body = b.rigid_body_caches[it.first].body.get();

		if (b_body) {
			rigid_body_caches[i].body = reinterpret_cast<b2Body*>(pointer_migrations.at(reinterpret_cast<const void*>(b_body)));
		}
	}

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
