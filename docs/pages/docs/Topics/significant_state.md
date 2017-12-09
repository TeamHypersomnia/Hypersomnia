---
title: Significant state
tags: [topics, ECS] 
hide_sidebar: true
permalink: significant_state
summary: |
  In any programming context, we call state **significant** if it is at any point read from disk, written to disk and/or synchronized through the network.  
  This wiki sometimes refers to "**the** significant state", by which it **always** means the [``cosmos::significant``](cosmos#overview) field. 
---

