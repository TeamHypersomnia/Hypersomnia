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
		if(!augs::read_object(ar, storage.step_type)) return false;

		if (storage.step_type == step_packaged_for_network::type::NEW_ENTROPY) {
			return augs::read_object(ar, storage.shall_resubstantiate) &&
			augs::read_object(ar, storage.next_client_commands_accepted) &&
			augs::read_object(ar, storage.entropy);
		}
		else if (storage.step_type == step_packaged_for_network::type::NEW_ENTROPY_WITH_HEARTBEAT) {
			return augs::read_object(ar, storage.next_client_commands_accepted) &&
			augs::read_object(ar, storage.entropy) &&

			augs::read_sized_stream(ar, storage.delta);
		}

		ensure(false);
		return false;
	}

	template<class A>
	void write_object(A& ar, const step_packaged_for_network& written) {
		augs::write_object(ar, written.step_type);

		if (written.step_type == step_packaged_for_network::type::NEW_ENTROPY) {
			augs::write_object(ar, written.shall_resubstantiate);
			augs::write_object(ar, written.next_client_commands_accepted);
			augs::write_object(ar, written.entropy);

		}
		else if (written.step_type == step_packaged_for_network::type::NEW_ENTROPY_WITH_HEARTBEAT) {
			augs::write_object(ar, written.next_client_commands_accepted);
			augs::write_object(ar, written.entropy);

			augs::write_sized_stream(ar, written.delta);
		}
		else
			ensure(false);
	}
}