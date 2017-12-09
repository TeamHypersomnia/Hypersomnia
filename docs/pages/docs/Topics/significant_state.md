---
title: Significant state
tags: [topics, ECS] 
hide_sidebar: true
permalink: significant_state
summary: |
  In any programming context, we call state **significant** if it is at any point read from disk, written to disk and/or synchronized through the network.  
  Additionally, in the context of this wiki, "**the** significant state" always refers to the field in the [cosmos](cosmos) named "significant" which, among all other state of the cosmos, is the part of state that is **significant**.
---

