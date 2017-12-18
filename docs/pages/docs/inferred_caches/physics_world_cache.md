---
title: Physics world cache
tags: [topics, inferred_caches]
hide_sidebar: true
permalink: physics_world_cache
summary: | 
  A **physics world cache** is a [cache](inferred_cache) inferred from the contents of all [rigid body components](rigid_body_component), [fixtures components](fixtures_component)  
  and all types of joint components (e.g. [motor joint](motor_joint_component])).  
  In practice, it means that it stores the physics world object (e.g. Box2D's ``b2World``) and its native body, fixture, shape and joint types.  
---

## Copy-assignment operator

Unfortunately, Box2D does not provide any copy-assignment operator for ``b2World``.  
Such a functionality is be essential during client-side prediction of server state.  

We currently have a rudimentary operator= that appears to work, but PlayRho appears to have its own that at least looks nice.  
We might at some point switch to this library because of the fixed-size calculations.  

We keep the significant state in our own format which should enable us to transition all potentially existing content rather smoothly.
