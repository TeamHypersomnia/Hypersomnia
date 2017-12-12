---
title: Unversioned pool id
tags: [topics, ECS] 
hide_sidebar: true
permalink: unversioned_pool_id
summary: |
  An **unversioned pool id** is a [pool id](pool_id) without the [version counter](pool#version-counter); hence, its one and only field is the [indirection index](pool#indirection-index).  
---

## Overview

Dereferencing an unversioned pool id stays predictable **only as long as the object it refers to is alive**.  
In particular, if the object gets freed and the pool later allocates a new object, it may, by a quirk of fate, be assigned the same indirection index.  
Then, the unversioned pool id may begin to point to a **completely different object**.  

An unversioned pool id is useful if its user wants to save on memory and the id of an object is guaranteed to stop existing the moment that the object referred to is deleted.  

For example, a ``b2Body`` contains ``unversioned_entity_id`` as its userdata, instead of the classic [``entity_id``](entity_id).  
It is reasonable, because a ``b2Body`` can never exist without an [entity](entity) and it is always destroyed along with any entity that it is tied to.
