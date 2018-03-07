---
title: Bug database
hide_sidebar: true
permalink: bug_database
summary: Notable bugs.
---

- When resetting the work unique_pointer inside the editor folder, the editor current access cache must be refreshed.
We were getting crashes because we were just assigning to the unique ptr, effectively making a new allocation. 
	- This was by the way completely sub-optimal.
