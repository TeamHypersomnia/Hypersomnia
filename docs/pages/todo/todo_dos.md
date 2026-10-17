---
title: ToDo DoS
hide_sidebar: true
permalink: todo_dos
summary: Just a hidden scratchpad.
---

- After release. Only what can be clogged from a few addresses - what one address can do is skipped for now.

- Global handshake limit can be taken from a few addresses (DoS of joining)
	- 20/s per address, burst 40, table of 4096, fails open when full (server_setup.cpp, take_handshake in netcode.c before seal_open)
	- A real cookie from own address + a garbage sealed box costs a token and an X25519
		- A failed open doesn't put the cookie in the ring, so the cookie works for 10 s
	- Five /64 networks use up the 100/s
	- Players see a silent "connection response timed out", then 5 more seconds on the fallback
	- Fix: bucket per /48, penalize the address whose seal_open failed, maybe a "busy" hint

- Skipped for now (one address is enough):
	- Casual has no per-IP connection limit - one IP fills all slots (intentional for LAN parties)
	- Signalling: no per-IP limits, unlimited relay, fake web servers from one IP, 8192 WS from one IP block web players
		- Fix: per-IP (/64) limit of peers and web servers, a finite rate limit for servers too, cap on pending_packets, ws -> id map
	- Exhausting the 64 WebRTC slots of a native server (a slot per offer, freed after 30 s)
		- Fix: limit slots per external_id and per remote_ip
	- Web peer heartbeat logs with no limit (signalling_server.h) - use official_log_allowed like the native paths
