---
title: Solver
tags: [topics, ECS] 
hide_sidebar: true
permalink: solver
summary: |
   A **solver** is a free-standing function that, given a [logic step input](logic_step#input) and *pre/post solve callbacks*,  
   instantiates a [logic step](logic_step) on the stack and moves the [cosmos](cosmos) forward in time.
---

## Overview

The only currently available solver is called *the standard solver*.  
It calls all [stateless systems](stateless_system) to effectively move the game forward in time by a specified [delta time](cosmos_meta#fixed-delta).
