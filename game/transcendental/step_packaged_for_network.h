#pragma once
#include "augs/misc/streams.h"
#include "cosmic_entropy.h"

struct step_packaged_for_network {
	enum class type : unsigned char {
		INVALID,
		NEW_ENTROPY,
		NEW_ENTROPY_WITH_HEARTBEAT
	} step_type = type::INVALID;

	bool shall_resubstantiate = false;
	bool next_client_commands_accepted = false;
	augs::stream delta;
	guid_mapped_entropy entropy;
};

namespace augs {
	template<class A>
	void read_object(A& ar, step_packaged_for_network& storage) {
		read(ar, storage.step_type);

		if (storage.step_type == step_packaged_for_network::type::NEW_ENTROPY) {
			read(ar, storage.shall_resubstantiate);
			read(ar, storage.next_client_commands_accepted);
			read(ar, storage.entropy);
		}
		else if (storage.step_type == step_packaged_for_network::type::NEW_ENTROPY_WITH_HEARTBEAT) {
			read(ar, storage.next_client_commands_accepted);
			read(ar, storage.entropy);

			read_sized_stream(ar, storage.delta);
		}
		else {
			ensure(false);
		}
	}

	template<class A>
	void write_object(A& ar, const step_packaged_for_network& written) {
		augs::write(ar, written.step_type);

		if (written.step_type == step_packaged_for_network::type::NEW_ENTROPY) {
			augs::write(ar, written.shall_resubstantiate);
			augs::write(ar, written.next_client_commands_accepted);
			augs::write(ar, written.entropy);

		}
		else if (written.step_type == step_packaged_for_network::type::NEW_ENTROPY_WITH_HEARTBEAT) {
			augs::write(ar, written.next_client_commands_accepted);
			augs::write(ar, written.entropy);

			augs::write_sized_stream(ar, written.delta);
		}
		else {
			ensure(false);
		}
	}
}