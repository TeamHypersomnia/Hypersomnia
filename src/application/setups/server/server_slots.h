#pragma once

/*
	A server's places: the netcode takes at most 64 connections (yojimbo::MaxClients).
	Beyond the game slots, it keeps a few spare ones - a player back after a crash replaces their connection
	that hasn't timed out yet, a full server answers why - and a place for the admins (one: they work together,
	a second one at once takes a spare connection) and one for the spectator account (a caster, a stream bot).
	The admins and the spectator account never take a slot - full or not, the slots are the players' and the watchers'.
	The server list shows the slots without any of them.
*/

inline constexpr int spare_connections_v = 4;
inline constexpr int reserved_admin_places_v = 1;
inline constexpr int reserved_spectator_places_v = 1;
inline constexpr int max_netcode_connections_v = 64;
inline constexpr int max_server_slots_v = max_netcode_connections_v - spare_connections_v - reserved_admin_places_v - reserved_spectator_places_v;
