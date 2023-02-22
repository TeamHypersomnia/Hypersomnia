---
title: Tips
tags: [planning]
hide_sidebar: true
permalink: tips
summary: Just a hidden scratchpad.
---

- For automation like assigning fish to the organism areas they overlap..
	- You can simply post-process all editor nodes after they are already built.
	- Assigning while they're being created doesn't work because depending on the order of creation..
		- ..area markers might not yet exist while you're creating the organism entities.
