#pragma once

/*
	A server's places: the netcode takes at most 64 connections (yojimbo::MaxClients).
	Beyond the game slots, it keeps a few spare ones - a player back after a crash replaces their connection
	that hasn't timed out yet, a full server answers why - and places only admins may take: an admin
	gets in even when the server is full - and one for the spectator account (a caster, a stream bot).
	The server list shows the slots without any of them.
*/

inline constexpr int spare_connections_v = 4;
inline constexpr int reserved_admin_places_v = 2;
inline constexpr int reserved_spectator_places_v = 1;
inline constexpr int max_netcode_connections_v = 64;
inline constexpr int max_server_slots_v = max_netcode_connections_v - spare_connections_v - reserved_admin_places_v - reserved_spectator_places_v;
