#pragma once

class b2Contact;
struct b2Manifold;
struct b2ContactImpulse;

class physics_world_cache;
class cosmos;

struct contact_listener : public b2ContactListener {
	void BeginContact(b2Contact* contact) override;
	void EndContact(b2Contact* contact) override;
	void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
	void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

	bool during_step = false;

	cosmos& cosm;
	physics_world_cache& get_sys() const;

	contact_listener(const contact_listener&) = delete;
	contact_listener(contact_listener&&) = delete;
	
	contact_listener(cosmos&);
	~contact_listener();
	
	contact_listener& operator=(const contact_listener&) = delete;
	contact_listener& operator=(contact_listener&&) = delete;
};
