#pragma once
#include "application/network/net_serialization_helpers.h"

namespace net_messages {
#if CONTEXTS_SEPARATE
	template <typename Stream>
	bool prestep_client_context::Serialize(Stream& stream) {
		if (!serialize(stream, payload)) {
			return false;
		}

		return true;
	}
#endif

	template <typename Stream>
	bool special_client_request::Serialize(Stream& stream) {
		if (!serialize_enum(stream, payload)) {
			return false;
		}

		return true;
	}

	template <typename Stream>
	bool client_welcome::Serialize(Stream& stream) {
		{
			if (!serialize_fixed_size_str(stream, payload.chosen_nickname)) {
				return false;
			}

			if (!serialize_fixed_size_str(stream, payload.rcon_password)) {
				return false;
			}
		}

		if (!serialize(stream, payload.public_settings)) {
			return false;
		}

		serialize_uint32(stream, payload.net.jitter.buffer_at_least_steps);
		serialize_uint32(stream, payload.net.jitter.buffer_at_least_ms);
		serialize_int(stream, payload.net.jitter.max_commands_to_squash_at_once, 0, 255);

		return true;
	}

	template <typename Stream>
	bool rcon_command::Serialize(Stream& stream) {
		if (!serialize(stream, payload)) {
			return false;
		}

		return true;
	}

	template <typename Stream>
	bool client_requested_chat::Serialize(Stream& stream) {
		if (!serialize(stream, payload)) {
			return false;
		}

		return true;
	}

	template <typename Stream>
	bool server_broadcasted_chat::Serialize(Stream& stream) {
		if (!serialize(stream, payload)) {
			return false;
		}

		return true;
	}

	template <typename Stream>
	bool net_statistics_update::Serialize(Stream& stream) {
		if (!serialize(stream, payload)) {
			return false;
		}

		return true;
	}

	template <typename Stream>
	bool public_settings_update::Serialize(Stream& stream) {
		if (!serialize(stream, payload.subject_id)) {
			return false;
		}

		if (!serialize(stream, payload.new_settings)) {
			return false;
		}

		return true;
	}
}
