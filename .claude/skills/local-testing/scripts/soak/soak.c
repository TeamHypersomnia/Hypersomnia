/*
	Connection soak client for a Hypersomnia server, at the netcode level:
	  soak <server ip:port> <netcode key hex or "insecure"> <server id or "-"> <cycles> <abandon every N (0 = never)>
	Every cycle is a new client on a new local port: connect, wait for CONNECTED, disconnect.
	Every N-th cycle instead sends the connection request and vanishes mid-handshake.
	Exits 1 if any cycle that was not abandoned failed to connect, and prints the first one.
	Build: see build_soak.sh.
*/
#include "netcode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static int hex_to_bytes(const char* hex, uint8_t* out, int n) {
	for (int i = 0; i < n; ++i) {
		unsigned v;
		if (sscanf(hex + 2 * i, "%2x", &v) != 1) return 0;
		out[i] = (uint8_t)v;
	}
	return 1;
}

int main(int argc, char** argv) {
	if (argc < 6) {
		fprintf(stderr, "usage: soak <server ip:port> <key hex|insecure> <server id|-> <cycles> <abandon every N>\n");
		return 2;
	}

	const char* server = argv[1];
	const int insecure = strcmp(argv[2], "insecure") == 0;
	const char* server_id = argv[3];
	const int cycles = atoi(argv[4]);
	const int abandon_every = atoi(argv[5]);

	uint8_t key[NETCODE_KEY_BYTES];
	memset(key, 0, sizeof(key));
	if (!insecure && !hex_to_bytes(argv[2], key, NETCODE_KEY_BYTES)) { fprintf(stderr, "bad key\n"); return 2; }

	/* secure_connect.h: "HSEC1" + the official server id */
	uint8_t user_data[NETCODE_USER_DATA_BYTES];
	memset(user_data, 0, sizeof(user_data));
	if (!insecure && strcmp(server_id, "-") != 0) {
		memcpy(user_data, "HSEC1", 5);
		memcpy(user_data + 5, server_id, strlen(server_id));
	}

	if (netcode_init() != NETCODE_OK) return 2;
	netcode_log_level(NETCODE_LOG_LEVEL_NONE);
	srand((unsigned)time(NULL) ^ (unsigned)getpid());

	int failed = 0, first_failed = -1, connected = 0, abandoned = 0;
	double t = 0.0;

	for (int i = 0; i < cycles; ++i) {
		const uint64_t client_id = ((uint64_t)rand() << 32) ^ (uint64_t)rand() ^ (uint64_t)i;
		uint8_t token[NETCODE_CONNECT_TOKEN_BYTES];
		NETCODE_CONST char* addresses[] = { server };

		/* The game's protocol id is its default port, 8412; timeouts disabled like in the game (-1) */
		if (netcode_generate_connect_token(1, addresses, addresses, 60, -1, client_id, 8412, key, user_data, token) != NETCODE_OK) {
			fprintf(stderr, "token generation failed\n");
			return 2;
		}

		struct netcode_client_config_t config;
		netcode_default_client_config(&config);
		struct netcode_client_t* client = netcode_client_create("0.0.0.0", &config, t);
		if (!client) { fprintf(stderr, "client create failed at %d\n", i); return 2; }

		netcode_client_connect(client, token);

		if (abandon_every > 0 && i % abandon_every == 0) {
			for (int k = 0; k < 3; ++k) { netcode_client_update(client, t); t += 0.01; usleep(10000); }
			netcode_client_destroy(client);
			++abandoned;
			continue;
		}

		int ok = 0;
		for (int k = 0; k < 500; ++k) {
			netcode_client_update(client, t);
			const int state = netcode_client_state(client);
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
			if (first_failed < 0) first_failed = i;
		}

		netcode_client_destroy(client);

		if ((i + 1) % 500 == 0) {
			printf("  %d cycles: %d connected, %d abandoned, %d failed\n", i + 1, connected, abandoned, failed);
			fflush(stdout);
		}
	}

	printf("done: %d cycles, %d connected, %d abandoned mid-handshake, %d failed", cycles, connected, abandoned, failed);
	if (failed) printf(" (first at cycle %d)", first_failed);
	printf("\n");

	netcode_term();
	return failed ? 1 : 0;
}
