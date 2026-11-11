#include <stdio.h>
#include "blake3.h"

int main(int argc, char** argv) {
	for (int a = 1; a < argc; ++a) {
		FILE* f = fopen(argv[a], "rb");

		if (!f) {
			return 1;
		}

		blake3_hasher h;
		blake3_hasher_init(&h);
		unsigned char buf[65536];
		size_t n;

		while ((n = fread(buf, 1, sizeof buf, f)) > 0) {
			blake3_hasher_update(&h, buf, n);
		}

		fclose(f);
		unsigned char out[BLAKE3_OUT_LEN];
		blake3_hasher_finalize(&h, out, BLAKE3_OUT_LEN);

		for (int i = 0; i < BLAKE3_OUT_LEN; ++i) {
			printf("%02x", out[i]);
		}

		printf("  %s\n", argv[a]);
	}

	return 0;
}
