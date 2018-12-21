---
title: Solver
tags: [topics, ECS] 
hide_sidebar: true
permalink: solver
summary: |
   A **solver** is a free-standing function that, given a [logic step input](logic_step_input) and *pre/post solve callbacks*,  
   instantiates a [logic step](logic_step) on the stack and moves the [cosmos solvable](cosmos_solvable) forward in time.
---

## Overview

The only currently available solver is called *the standard solver*.  
Once we introduce more solvers, they will be put into a separate ``src/game/solvers`` folder.  
This could also be a good place to start writing a game mod.

It initializes all [message](message) queues on [TLS](https://en.wikipedia.org/wiki/Thread-local_storage) and clears them when the step finishes, to ensure no messages persist beyond duration of the step.  
If some messages were to linger between two consecutive steps, one would need to save them to disk or even synchronize through the network to ensure [determinism](determinism), effectively making messages another case of [significant state](significant_state).  
It makes much more sense to just design the solver so that all posted messages are handled in the same simulation step.

## Pre solve

## The solve

It calls all [stateless systems](stateless_system) to effectively move the game forward in time by a specified [delta time](cosmos_clock#fixed-delta).
<!--The *advance* function accepts [entropy](cosmic_entropy) along with a reference to all [logical assets](logical_asset) referenced via ids from inside [significant](#significant), in order to perform a single simulation step.  -->

## Post solve
