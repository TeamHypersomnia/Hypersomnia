#pragma once

enum class client_state_type {
	NETCODE_NEGOTIATING_CONNECTION,

	PENDING_WELCOME,
	WELCOME_ARRIVED,
	PENDING_RECEIVING_INITIAL_SNAPSHOT,
	RECEIVING_INITIAL_SNAPSHOT,
	IN_GAME
};

