---
title: Bandwidth
tags: [planning]
hide_sidebar: true
permalink: todo_bandwidth
summary: Things to do to improve bandwidth.
---

- Do we send the client back their own entropy?
	- If no and we just add num accepted commands, a desync may occur if the server can modify for some reason the entropy after receiving it from the client
	- if yes, that's a slight optimization opportunity, although it may complicate stuff a little

- Clear cosmic entropy when the player is not conscious
	- So that others are not allowed to waste bandwidth when they are dead

- Let user send its mouse sensitivity and later only send shorts to better encode the mouse movement

- Don't send the same entropy to the client who sent it, if it was unmodified
	- Make a bit for that
