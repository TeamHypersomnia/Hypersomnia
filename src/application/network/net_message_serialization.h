#pragma once
#include "application/network/net_serialization_helpers.h"

template <typename Stream>
bool preserialized_message::Serialize(Stream& stream) {
	int length = 0;

	if (Stream::IsWriting) {
		length = static_cast<int>(bytes.size());
	}

	serialize_int(stream, length, 0, max_message_size_v);

	if (Stream::IsReading) {
		bytes.resize(length);
	}

	serialize_bytes(stream, (uint8_t*)bytes.data(), length);

	return true;
}

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

		serialize_float(stream, payload.public_settings.mouse_sensitivity.x);
		serialize_float(stream, payload.public_settings.mouse_sensitivity.y);

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
}
