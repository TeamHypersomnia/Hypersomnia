/*
	Connection soak client for a Hypersomnia server, at the netcode level:
	  soak <server ip:port> <cycles> <abandon every N (0 = never)> [server public key hex]
	Every cycle is a new client on a new local port with a new session key: connect, wait for CONNECTED, disconnect.
	Every N-th cycle instead sends the connection request and vanishes mid-handshake.
	With the server's key the client accepts only that key, like one joining from the server list.
	Exits 1 if any cycle that was not abandoned failed to connect, and prints the first one.
	Build: see build_soak.sh.
*/
#include "netcode.h"
#include <sodium.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int hex_to_bytes(const char* hex, uint8_t* out, int n) {
	if ((int)strlen(hex) != 2 * n) return 0;
	for (int i = 0; i < n; ++i) {
		unsigned v;
		if (sscanf(hex + 2 * i, "%2x", &v) != 1) return 0;
		out[i] = (uint8_t)v;
	}
	return 1;
}

int main(int argc, char** argv) {
	if (argc < 4) {
		fprintf(stderr, "usage: soak <server ip:port> <cycles> <abandon every N> [server public key hex]\n");
		return 2;
	}

	struct netcode_address_t server;
	if (netcode_parse_address(argv[1], &server) != NETCODE_OK) { fprintf(stderr, "bad address\n"); return 2; }

	const int cycles = atoi(argv[2]);
	const int abandon_every = atoi(argv[3]);

	uint8_t expected_key[32];
	const int has_expected_key = argc > 4;
	if (has_expected_key && !hex_to_bytes(argv[4], expected_key, 32)) { fprintf(stderr, "bad key\n"); return 2; }

	if (sodium_init() < 0 || netcode_init() != NETCODE_OK) return 2;

	/* The game's protocol version (major.minor, see work.cpp) - a development build is HS0.0 */
	netcode_set_version_info(getenv("HS_PROTOCOL") ? getenv("HS_PROTOCOL") : "HS0.0");
	netcode_log_level(NETCODE_LOG_LEVEL_NONE);

	int failed = 0, first_failed = -1, first_failed_state = 0, connected = 0, abandoned = 0;
	double t = 0.0;

	for (int i = 0; i < cycles; ++i) {
		uint8_t session_pk[32], session_sk[64];
		crypto_sign_keypair(session_pk, session_sk);

		struct netcode_client_config_t config;
		netcode_default_client_config(&config);
		/* The game's protocol id is its default port */
		config.protocol_id = 8412;

		struct netcode_client_t* client = netcode_client_create("0.0.0.0", &config, t);
		if (!client) { fprintf(stderr, "client create failed at %d\n", i); return 2; }

		netcode_client_connect(client, 1, &server, has_expected_key ? expected_key : NULL, session_sk, NULL);

		if (abandon_every > 0 && i % abandon_every == 0) {
			for (int k = 0; k < 3; ++k) { netcode_client_update(client, t); t += 0.01; usleep(10000); }
			netcode_client_destroy(client);
			++abandoned;
			continue;
		}

		int ok = 0, state = 0;
		for (int k = 0; k < 500; ++k) {
			netcode_client_update(client, t);
			state = netcode_client_state(client);
			if (state == NETCODE_CLIENT_STATE_CONNECTED) { ok = 1; break; }
			if (state < 0) break;
			t += 0.01;
			usleep(10000);
		}

		if (ok) {
			++connected;
			netcode_client_disconnect(client);
		}
		else {
			++failed;
			if (first_failed < 0) { first_failed = i; first_failed_state = state; }
		}

		netcode_client_destroy(client);

		if ((i + 1) % 500 == 0) {
			printf("  %d cycles: %d connected, %d abandoned, %d failed\n", i + 1, connected, abandoned, failed);
			fflush(stdout);
		}
	}

	printf("done: %d cycles, %d connected, %d abandoned mid-handshake, %d failed", cycles, connected, abandoned, failed);
	if (failed) printf(" (first at cycle %d, state %d)", first_failed, first_failed_state);
	printf("\n");

	netcode_term();
	return failed ? 1 : 0;
}
