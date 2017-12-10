---
title: Significant state
tags: [topics, ECS] 
hide_sidebar: true
permalink: significant_state
summary: |
  We call state **significant** if it is at any point read from disk, written to disk and/or synchronized through the network.  
  The most notable example of significant state in Hypersomnia is the [``cosmos::significant``](cosmos#significant) field. 
---

