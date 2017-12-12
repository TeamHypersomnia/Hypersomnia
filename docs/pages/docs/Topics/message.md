---
title: Message
tags: [topics, ECS] 
hide_sidebar: true
permalink: message
summary: |
    A **message** is a piece of information generated when a [stateless system](stateless_system) runs its logic.  
    A message, after being written to a [**message queue**](message_queue), can be later read and executed by one or more unrelated [systematic functions](systematic_function).
---

## Overview

A message has two distinct usecases 

  1. One [stateless system](stateless_system) wishes to communicate something to other stateless systems, while those systems have otherwise no reason to know about each other.
      - example: [``physics_system``](physics_system) communicates ``collision_messages`` to a [``missile_system``](missile_system) that uses them to detonate rockets and bullets.
  2. A [stateless system](stateless_system) wishes to communicate something to an [audiovisual system](audiovisual_system). 
    
      - Needless to say, the [logic step](logic_step) - which is all information that a stateless system uses for its run - provides no means to talk to any audiovisual system, due to risk of breaking [determinism](determinism).
         However, there are cases where an audiovisual system could use information about some important occurences during the logic step.
         For example, the [exploding ring system](exploding_ring_system) must receive proper input from when explosions happen to play nice explosion effects.
       
         A message called [``exploding_ring_input``](exploding_ring_input) exists for that purpose. It is posted inside stateless systems when they need to ignite explosions.  
         The message is later received by the exploding ring system inside the [post solve callback](cosmos#post-solve), right before the logic step meets the end of its lifetime, along with all message queues.
